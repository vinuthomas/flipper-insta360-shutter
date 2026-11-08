# Protocol and hardware verification

The Flipper advertises as the BLE peripheral and hosts `CE80`. The camera connects as the central. The app waits for a notification subscription before it sends a shutter frame.

| Characteristic | Behavior |
| --- | --- |
| `CE81` | Camera writes to the remote with or without response. The app counts the writes. |
| `CE82` | Remote sends shutter notifications to the subscribed camera. |
| `CE83` | Camera reads the two bytes `01 02`, matching Multicam. |

The first shutter notification is `FC EF FE 86 00 03 01 02 00`. The fifth byte is a connection-local sequence counter. It advances by two per press, so the second frame is `FC EF FE 86 02 03 01 02 00`. Both start and stop use the same shutter toggle. The Action Pod's recording indicator confirms the result.

The app includes the 19-byte manufacturer field seen in the working iPhone advertisement. The firmware patch changes only the local name and the 16-bit service-list type for the matching profile. The manufacturer field and the two firmware changes have not been isolated in the successful test.

## Verified on 8 November 2026

- An Android nRF Connect GATT probe matched Multicam's CE80 service and returned `01 02` on CE83. The GO 3S connected, subscribed and started and stopped video in response to both shutter frames.
- The stock Flipper test served the same characteristics and manufacturer bytes. The GO 3S skipped its advertisement.
- The patched Flipper advertised the full name, complete CE80 list and matching manufacturer bytes. With the phone remotes off, the GO 3S discovered it, connected and subscribed. Two presses of **OK** on the Flipper started and stopped video.

One mode-cycle notification sent from Android produced no visible change on the GO 3S. The Flipper app exposes only the verified shutter action.

## Protocol references

- [Multicam GPS remote service](https://github.com/dstrat28/action-multicam-remote/blob/45fb162/ActionCamRemote/Bluetooth/Insta360RemoteService.swift) and [button frames](https://github.com/dstrat28/action-multicam-remote/blob/45fb162/ActionCamRemote/Bluetooth/Insta360RemoteProtocol.swift)
- [Insta360 ESP32 remote sketch](https://github.com/pchwalek/insta360_ble_esp32/blob/83d4748/Insta_BLE.ino)
- [X4 packet capture and button sequence](https://github.com/TheAngryRaven/insta360-ble-gps-spec#4-button-events-remote--camera-ce82)
