# Firmware patch for Flipper Zero 1.5.1

The patch applies to official Flipper Zero firmware 1.5.1 at commit `9e810dc399e027e48ce07751df72168abbb19065`.

## Why the patch is needed

On stock 1.5.1, the GO 3S didn't list the Flipper even though the app exposed the same `CE80` service and characteristics as the working Android remote. Discovery happens before the camera can use those characteristics. The stock `GapConfig` interface leaves 16 characters for the advertised name after the advertising type byte and the string terminator. `Insta360 GPS Remote` is 19 characters. The firmware also labels 16-bit service lists as incomplete (`0x02`), with no app setting to select a complete list (`0x03`). [Flipper advertising code](https://github.com/flipperdevices/flipperzero-firmware/blob/1.5.1/targets/f7/ble_glue/gap.c)

The patch matches the app's `CE80` profile and its complete-name marker `Insta360 GPS Rem`. It substitutes the full name and marks the `CE80` list complete. The marker fits the stock app interface, so the patch preserves the `GapConfig` layout and SDK API 88.2. The GO 3S discovered the Flipper and accepted both shutter presses with this combination. The test doesn't establish whether either change would work on its own.

## Build the update

From the Flipper firmware checkout, replace the patch path with the path to this repo.

```sh
git checkout 1.5.1
git submodule update --init --recursive
git apply --check /path/to/flipper-insta360-shutter/firmware/insta360-advertising.patch
git apply /path/to/flipper-insta360-shutter/firmware/insta360-advertising.patch
DIST_SUFFIX=1.5.1-insta360-test ./fbt COMPACT=1 DEBUG=0 updater_minpackage
```

The normal updater package is in `dist/f7-C/f7-update-1.5.1-insta360-test/`. It contains the manifest, updater and firmware image. The minimal package leaves the installed radio firmware and SD resources in place. [Flipper's update package documentation](https://github.com/flipperdevices/flipperzero-firmware/blob/1.5.1/documentation/OTA.md)

## Install through the normal updater

1. Back up internal storage in qFlipper and copy the microSD contents to your computer. Keep the official 1.5.1 update available for rollback.
2. Copy the entire generated update directory to the microSD card under `update/`. Open its `update.fuf` in the Flipper file browser and start the update.
3. Let the Flipper reboot. Install the app and verify the advertised name and complete service list before testing the camera.

The normal updater backs up and restores internal storage as part of the install. Recovery repair erases internal storage. [Backup instructions](https://docs.flipper.net/zero/basics/firmware-update/storage-repair) · [Recovery behavior](https://docs.flipper.net/zero/basics/firmware-update/firmware-recovery)
