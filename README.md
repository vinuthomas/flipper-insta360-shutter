# Insta360 Shutter for Flipper Zero

Use the Flipper Zero as a Bluetooth shutter remote for an Insta360 GO 3S. Once the camera connects, press **OK** to start recording and press it again to stop. The GO 3S handled both presses in a direct hardware test with the firmware patch in this repo.

## Compatibility

| Setup | Result |
| --- | --- |
| Flipper Zero f7 with patched firmware 1.5.1 and a GO 3S | Discovery, connection and video start/stop verified |
| Stock Flipper Zero firmware 1.5.1 and a GO 3S | The camera skipped the app during discovery |
| Other firmware versions or camera models | Untested |

The firmware patch lets this app advertise the full `Insta360 GPS Remote` name and mark `CE80` as a complete 16-bit service list. Both changes were present in the successful test. Their individual effects are unverified.

## Build and install

1. Build and install the [patched firmware](firmware/README.md) using Flipper's normal updater.
2. Install [uFBT](https://github.com/flipperdevices/flipperzero-ufbt). From this repo's root, build against the f7 SDK for 1.5.1 (API 88.2).

   ```sh
   ufbt update --url https://update.flipperzero.one/builds/firmware/1.5.1/flipper-z-f7-sdk-1.5.1.zip
   ufbt
   ```

3. In qFlipper, copy `dist/insta360_remote.fap` into **SD Card → apps → Bluetooth**. Open **Apps → Bluetooth → Insta360 Shutter** on the Flipper.

## Connect and record

1. Power on the GO 3S and Action Pod. Open **Bluetooth Remote** on the Action Pod and select **Insta360 GPS Remote**.
2. Wait for **CE82 subscribed** on the Flipper. With the camera idle in Video mode, press **OK** and check the Action Pod for the recording indicator. Press **OK** again to stop.
3. Press **Back** to exit and restore the Flipper's normal Bluetooth profile.

If the remote is missing, scan from nRF Connect on Android. Look for the full name and a **Complete list of 16-bit Service UUIDs** containing `CE80`. Disconnect any scanner that connected to the Flipper before trying again on the camera.

See the [protocol and hardware notes](docs/protocol-and-testing.md) for the packet format and test scope. The app currently covers shutter control. Mode switching, wake-up, GPS and reconnect after a power cycle are outside the verified path.

## Licensing

The firmware patch modifies Flipper's GPLv3 firmware. Its upstream license is in [firmware/LICENSE](firmware/LICENSE). An app-source license is pending maintainer selection before publication.
