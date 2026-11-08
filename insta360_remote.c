#include <furi.h>
#include <furi_hal.h>
#include <bt/bt_service/bt.h>
#include <ble/core/ble_defs.h>
#include <furi_ble/event_dispatcher.h>
#include <furi_ble/gatt.h>
#include <furi_ble/profile_interface.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <storage/storage.h>
#include <stdatomic.h>

#define TAG "InstaRemote"
#define REMOTE_NAME "Insta360 GPS Rem"

enum {
    RemoteWrite,
    RemoteNotify,
    RemoteVersion,
};

enum {
    LinkConnected = 1,
    LinkSubscribed = 2,
};

enum { ButtonFrameSize = 9 };
static const uint8_t version[] = {0x01, 0x02};

static const BleGattCharacteristicParams primary_chars[] = {
    [RemoteWrite] =
        {.name = "CE81",
         .uuid.Char_UUID_16 = 0xce81,
         .uuid_type = UUID_TYPE_16,
         .char_properties = CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RESP,
         .data.fixed.length = 244,
         .gatt_evt_mask = GATT_NOTIFY_ATTRIBUTE_WRITE,
         .is_variable = CHAR_VALUE_LEN_VARIABLE},
    [RemoteNotify] =
        {.name = "CE82",
         .uuid.Char_UUID_16 = 0xce82,
         .uuid_type = UUID_TYPE_16,
         .char_properties = CHAR_PROP_NOTIFY,
         .data.fixed.length = ButtonFrameSize,
         .gatt_evt_mask = GATT_NOTIFY_ATTRIBUTE_WRITE},
    [RemoteVersion] =
        {.name = "CE83",
         .uuid.Char_UUID_16 = 0xce83,
         .uuid_type = UUID_TYPE_16,
         .char_properties = CHAR_PROP_READ,
         .data.fixed = {.ptr = version, .length = sizeof(version)}},
};

typedef struct {
    FuriHalBleProfileBase base;
    GapSvcEventHandler* handler;
    uint16_t primary_service;
    BleGattCharacteristicInstance primary[COUNT_OF(primary_chars)];
    atomic_uint link;
    atomic_uint writes;
    atomic_uint sequence;
} RemoteProfile;

typedef enum {
    ResultNone,
    ResultShutterQueued,
    ResultError,
} CommandResult;

typedef struct {
    RemoteProfile* profile;
    FuriMessageQueue* input;
    atomic_uint result;
    atomic_bool exit_requested;
} RemoteApp;

static const FuriHalBleProfileTemplate remote_template;

static BleEventAckStatus remote_event(void* event, void* context) {
    RemoteProfile* remote = context;
    // The dispatcher passes an HCI UART packet: packet type, event code, payload length.
    const uint8_t* packet = event;
    const uint8_t* data = packet + 3;
    const uint8_t length = packet[2];

    if(packet[1] == HCI_LE_META_EVT_CODE && length >= 2 &&
       data[0] == HCI_LE_CONNECTION_COMPLETE_SUBEVT_CODE && data[1] == 0) {
        atomic_store(&remote->writes, 0);
        atomic_store(&remote->sequence, 0);
        atomic_store(&remote->link, LinkConnected);
        FURI_LOG_I(TAG, "BLE link connected");
    } else if(packet[1] == HCI_DISCONNECTION_COMPLETE_EVT_CODE && length >= 4 &&
              data[0] == 0) {
        atomic_store(&remote->link, 0);
        FURI_LOG_I(TAG, "BLE link disconnected, reason %02X", data[3]);
    } else if(packet[1] == HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE && length >= 10 &&
              data[0] == 0x01 && data[1] == 0x0c) {
        // STM32WB ACI_GATT_ATTRIBUTE_MODIFIED_VSEVT_CODE (0x0c01).
        const aci_gatt_attribute_modified_event_rp0* changed = (const void*)(data + 2);
        if(changed->Attr_Handle == remote->primary[RemoteWrite].handle + 1) {
            atomic_fetch_add(&remote->writes, 1);
            FURI_LOG_I(TAG, "CE81 write: %u bytes", (unsigned)changed->Attr_Data_Length);
            return BleEventAckFlowEnable;
        }
        // ST allocates the notification subscription descriptor after the value.
        if(changed->Attr_Handle == remote->primary[RemoteNotify].handle + 2 &&
           changed->Attr_Data_Length == 2 && length >= 12) {
            bool enabled = (changed->Attr_Data[0] & 1) != 0;
            atomic_store(&remote->link, LinkConnected | (enabled ? LinkSubscribed : 0));
            FURI_LOG_I(TAG, "CE82 notifications %s", enabled ? "enabled" : "disabled");
            return BleEventAckFlowEnable;
        }
    }

    // GAP still needs to process the connection and disconnection events.
    return BleEventNotAck;
}

static void remote_delete_chars(
    uint16_t service,
    BleGattCharacteristicInstance* chars,
    size_t count) {
    if(!service) return;
    for(size_t i = 0; i < count && chars[i].characteristic; ++i) {
        if(chars[i].handle) {
            ble_gatt_characteristic_delete(service, &chars[i]);
        } else {
            // The SDK copies the params even when the controller rejects the characteristic.
            free((void*)chars[i].characteristic);
        }
    }
    ble_gatt_service_delete(service);
}

static void remote_stop(FuriHalBleProfileBase* profile) {
    RemoteProfile* remote = (RemoteProfile*)profile;
    if(remote->handler) ble_event_dispatcher_unregister_svc_handler(remote->handler);
    remote_delete_chars(remote->primary_service, remote->primary, COUNT_OF(remote->primary));
    free(remote);
}

static bool remote_init_chars(
    uint16_t service,
    const BleGattCharacteristicParams* params,
    BleGattCharacteristicInstance* chars,
    size_t count) {
    for(size_t i = 0; i < count; ++i) {
        ble_gatt_characteristic_init(service, &params[i], &chars[i]);
        if(!chars[i].handle) return false;
        if(params[i].char_properties == CHAR_PROP_READ &&
           ble_gatt_characteristic_update(service, &chars[i], NULL)) {
            return false;
        }
    }
    return true;
}

static FuriHalBleProfileBase* remote_start(FuriHalBleProfileParams params) {
    UNUSED(params);
    RemoteProfile* remote = calloc(1, sizeof(RemoteProfile));
    if(!remote) return NULL;
    remote->base.config = &remote_template;
    atomic_init(&remote->link, 0);
    atomic_init(&remote->writes, 0);
    atomic_init(&remote->sequence, 0);

    const Service_UUID_t primary_uuid = {.Service_UUID_16 = 0xce80};
    // One service record, two per characteristic and one extra for CE82's CCCD.
    if(!ble_gatt_service_add(UUID_TYPE_16, &primary_uuid, PRIMARY_SERVICE, 8, &remote->primary_service) ||
       !remote_init_chars(
           remote->primary_service, primary_chars, remote->primary, COUNT_OF(primary_chars))) {
        remote_stop(&remote->base);
        return NULL;
    }

    remote->handler = ble_event_dispatcher_register_svc_handler(remote_event, remote);
    return &remote->base;
}

static void remote_gap_config(GapConfig* config, FuriHalBleProfileParams params) {
    UNUSED(params);
    *config = (GapConfig){
        .adv_service = {.UUID_Type = UUID_TYPE_16, .Service_UUID_16 = 0xce80},
        // Manufacturer field from the working iPhone scan record, including company ID.
        .mfg_data = {0x4c, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        .mfg_data_len = 19,
        .appearance_char = 0x0180, // Generic remote control
        .pairing_method = GapPairingNone,
        .bonding_mode = false,
        .conn_param = {.conn_int_min = 0x06, .conn_int_max = 0x24},
    };
    // Keep a distinct address from the Flipper RPC and stock HID profiles.
    memcpy(config->mac_address, furi_hal_version_get_ble_mac(), sizeof(config->mac_address));
    config->mac_address[2] += 2;
    // This is the full configured name within stock firmware's buffer.
    _Static_assert(sizeof(REMOTE_NAME) + 1 <= sizeof(config->adv_name), "BLE name too long");
    config->adv_name[0] = AD_TYPE_COMPLETE_LOCAL_NAME;
    memcpy(config->adv_name + 1, REMOTE_NAME, sizeof(REMOTE_NAME));
}

static const FuriHalBleProfileTemplate remote_template = {
    .start = remote_start,
    .stop = remote_stop,
    .get_gap_config = remote_gap_config,
};

static void remote_draw(Canvas* canvas, void* context) {
    RemoteApp* app = context;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 11, "Insta360 Shutter");
    canvas_set_font(canvas, FontSecondary);
    if(!app->profile) {
        canvas_draw_str(canvas, 2, 28, "BLE profile failed");
        canvas_draw_str(canvas, 2, 58, "Back: exit");
        return;
    }

    unsigned link = atomic_load(&app->profile->link);
    const char* status = "Waiting for BLE link";
    if(link & LinkSubscribed) {
        status = "CE82 subscribed";
    } else if(link & LinkConnected) {
        status = "Linked; waiting for CE82";
    }
    canvas_draw_str(canvas, 2, 23, status);

    char writes[30];
    snprintf(writes, sizeof(writes), "CE81 writes: %u", atomic_load(&app->profile->writes));
    canvas_draw_str(canvas, 2, 35, writes);

    if(link & LinkSubscribed) {
        const char* message = "OK: shutter";
        switch(atomic_load(&app->result)) {
        case ResultShutterQueued:
            message = "Shutter queued";
            break;
        case ResultError:
            message = "BLE update failed";
            break;
        default:
            break;
        }
        canvas_draw_str(canvas, 2, 47, message);
    }
    canvas_draw_str(canvas, 2, 60, "Back: exit");
}

static void remote_input(InputEvent* event, void* context) {
    RemoteApp* app = context;
    if(event->type != InputTypeShort) return;
    if(event->key == InputKeyBack) {
        atomic_store(&app->exit_requested, true);
    } else if(event->key == InputKeyOk) {
        furi_message_queue_put(app->input, &event->key, 0);
    }
}

int32_t insta360_remote_app(void* context) {
    UNUSED(context);
    Bt* bt = furi_record_open(RECORD_BT);
    bool supported = furi_hal_bt_is_gatt_gap_supported();
    RemoteApp app = {.input = furi_message_queue_alloc(4, sizeof(InputKey))};
    atomic_init(&app.result, ResultNone);
    atomic_init(&app.exit_requested, false);

    if(supported) {
        bt_disconnect(bt);
        bt_keys_storage_set_storage_path(bt, APP_DATA_PATH("remote.keys"));
        app.profile = (RemoteProfile*)bt_profile_start(bt, &remote_template, NULL);
        if(app.profile) furi_hal_bt_start_advertising();
    }

    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* view = view_port_alloc();
    view_port_draw_callback_set(view, remote_draw, &app);
    view_port_input_callback_set(view, remote_input, &app);
    gui_add_view_port(gui, view, GuiLayerFullscreen);

    while(!atomic_load(&app.exit_requested)) {
        InputKey key;
        if(furi_message_queue_get(app.input, &key, 200) == FuriStatusOk &&
           app.profile && (atomic_load(&app.profile->link) & LinkSubscribed) &&
           !atomic_load(&app.exit_requested)) {
            uint8_t sequence = (uint8_t)atomic_fetch_add(&app.profile->sequence, 2);
            uint8_t frame[ButtonFrameSize] = {
                0xfc, 0xef, 0xfe, 0x86, sequence, 0x03, 0x01, 0x02, 0x00};
            // SDK 1.5.1 returns true on error for this function.
            bool error = ble_gatt_characteristic_update(
                app.profile->primary_service, &app.profile->primary[RemoteNotify], frame);
            atomic_store(&app.result, error ? ResultError : ResultShutterQueued);
        }
        if(app.profile && !(atomic_load(&app.profile->link) & LinkSubscribed)) {
            atomic_store(&app.result, ResultNone);
        }
        view_port_update(view);
    }

    view_port_enabled_set(view, false);
    gui_remove_view_port(gui, view);
    view_port_free(view);
    furi_record_close(RECORD_GUI);
    furi_message_queue_free(app.input);

    if(supported) {
        bt_disconnect(bt);
        bt_keys_storage_set_default_path(bt);
        if(!bt_profile_restore_default(bt)) {
            FURI_LOG_E(TAG, "Default BLE profile restore failed");
        }
    }
    furi_record_close(RECORD_BT);
    return 0;
}
