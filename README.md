# Insta360 Shutter for Flipper Zero

Use the Flipper Zero as a Bluetooth shutter remote for an Insta360 GO 3S. Once the camera connects, press **OK** to start recording and press it again to stop. The GO 3S handled both presses in a direct hardware test with the firmware patch in this repo.

## Compatibility

| Setup | Result |
| --- | --- |
| Flipper Zero f7 with patched firmware 1.5.1 and a GO 3S | Discovery, connection and video start/stop verified |
| Stock Flipper Zero firmware 1.5.1 and a GO 3S | The camera skipped the app during discovery |
| Other firmware versions or camera models | Untested |

## Why the firmware patch is needed

The GO 3S didn't list the app on stock firmware 1.5.1. Stock firmware limits the app's advertised name to 16 characters and labels the advertised `CE80` UUID list as incomplete. The app can host the remote's Bluetooth service on stock firmware, but it can't change those two parts of the advertisement.

The [firmware patch](firmware/README.md) advertises the full `Insta360 GPS Remote` name and puts `CE80` in a complete 16-bit service list. With both changes in place, the GO 3S discovered the Flipper, connected and responded to the shutter button. We tested the changes together, so we haven't isolated which one the camera requires.

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

The Flipper app source is available under the [MIT License](LICENSE).
The [firmware patch](firmware/insta360-advertising.patch) modifies Flipper firmware and is covered by [GPLv3](firmware/LICENSE).
