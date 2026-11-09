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

One mode-cycle notification sent from Android produced no visible change on the GO 3S. On 9 November 2026, I tested mode cycling again from the Flipper.

## Additional controls and test status

The development build adds three candidate commands from the [M5StickC remote's command definitions](https://github.com/theserialhobbyist/insta360_m5StickC_remote/blob/main/config.h). They use the existing CE82 subscription and share the connection-local sequence counter with shutter:

| Control | Final two frame bytes | Status |
| --- | --- | --- |
| Short Left: screen toggle | `00 00` | Worked on my GO 3S on 9 November 2026 |
| Hold Right: sleep/power off | `00 03` | Worked on my GO 3S on 9 November 2026 |
| Short Up: mode select then confirm | `01 00`, then `02 00` | I verified both sequences on 9 November 2026 |

Shutter also worked on my GO 3S in the test build. I haven’t recorded its firmware version. My GO 3S disconnects from the Flipper when it goes to sleep. Sleep is one way in this app: remote wake-up is not implemented, so the camera must be turned back on manually. Screen on/off works as a toggle without putting the camera to sleep. Reconnection after manual wake-up or a power cycle remains unverified. The test did not separately establish the power state of the camera and Action Pod.

On 9 November 2026, I confirmed on my camera that Up followed by OK changes to the next mode. Each completed mode-change sequence advances to the next available camera mode. Repeated Up presses cycle through the available modes; the exact mode list and order are not recorded. Subsequent OK presses work as shutter in the selected mode. The development build now automates that sequence: Up sends mode select, waits 300 ms and sends the shutter command as confirmation. Both frames consume their own sequence value, advancing by two each time. I also confirmed that the automatic sequence works on my camera with this delay. A failed first update, exit request, lost subscription or connection change cancels the confirmation.

With recording stopped and the screen on, press Up once and check that the mode changes without taking a photo or starting recording. Repeat slowly to check the mode order, then verify that OK still acts as shutter.

OK still sends shutter with `02 00`. Holding Right avoids sending sleep with a short accidental press. Queued feedback means the Bluetooth update succeeded, not that the camera accepted the command. The advertising configuration, CE83 value and normal Bluetooth restoration on exit are unchanged. Wake-up isn't implemented.

### Camera test sequence

1. Stop the Android advertiser and close Multicam. Check the full advertised name and complete CE80 list, then disconnect the scanner.
2. Connect from the Action Pod and wait for Ready (CE82 subscribed). Record the camera model and firmware version in your test notes.
3. In Video mode, press OK twice. Confirm recording starts and stops on the Action Pod. In Photo mode, confirm OK captures a photo.
4. Test one-press Up mode change as described above. With recording stopped, press Left once. Note whether the Action Pod screen changes. Press Left again and note whether it returns. Retest shutter.
5. Hold Right once. Note whether the camera, Action Pod or both turn off and whether the Flipper disconnects. Turn the camera back on manually.
6. Check whether the camera reconnects without pairing again. If it doesn't, record that separately and reconnect manually. Retest shutter.
7. Exit with Back and check that the Flipper's normal Bluetooth profile returns.

Keep results as observations until confirmed. Release scope should include only controls that pass hardware testing.

## Protocol references

- [Multicam GPS remote service](https://github.com/dstrat28/action-multicam-remote/blob/45fb162/ActionCamRemote/Bluetooth/Insta360RemoteService.swift) and [button frames](https://github.com/dstrat28/action-multicam-remote/blob/45fb162/ActionCamRemote/Bluetooth/Insta360RemoteProtocol.swift)
- [Insta360 ESP32 remote sketch](https://github.com/pchwalek/insta360_ble_esp32/blob/83d4748/Insta_BLE.ino)
- [X4 packet capture and button sequence](https://github.com/TheAngryRaven/insta360-ble-gps-spec#4-button-events-remote--camera-ce82)

## Screen layout

The refreshed screen keeps all four controls visible with a larger OK shutter area. Ready means CE82 notifications are subscribed; Pairing means no camera is connected and Linking means the subscription is pending. The footer reports commands sent, not confirmed camera state. The README SVG is an emulated ready state with approximate fonts, generated from `remote_screen.h` by `scripts/render_screen.py`. The layout change passed compilation and API checks; visual inspection on the device remains separate from camera protocol testing.
