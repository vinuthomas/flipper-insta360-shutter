# Insta360 Shutter for Flipper Zero

Use the Flipper Zero as a Bluetooth shutter remote for an Insta360 GO 3S. Once the camera connects, press **OK** to take a photo in Photo mode or to start and stop recording in Video mode. Both modes were tested directly on a GO 3S with the firmware patch in this repo.

## Compatibility

| Setup | Result |
| --- | --- |
| Flipper Zero f7 with patched firmware 1.5.1 and a GO 3S | Discovery, connection, photo capture and video start/stop verified |
| Stock Flipper Zero firmware 1.5.1 and a GO 3S | The camera skipped the app during discovery |
| Other firmware versions or camera models | Untested |

### Potential compatibility with other cameras

Insta360 lists the following cameras as compatible with its [GPS Action Remote](https://store.insta360.com/in/product/gps-action-remote) (checked 8 November 2026):

| Camera family | Models |
| --- | --- |
| X series | X4 Air, X5, X4, X3 and ONE X2 |
| GO series | GO 3S and GO 3 |
| Ace series | Ace Pro 2, Ace Pro and Ace |
| Modular cameras | ONE RS (Twin/4K), ONE RS 1-Inch 360 Edition and ONE R |

These models are potentially compatible with this app's shutter control based on GPS Action Remote support. **Only the GO 3S is hardware verified with this app. All other listed models are untested.** Official remote support doesn't guarantee that a camera accepts this app's pairing behavior and shutter commands. GPS, wake-up and other official remote features aren't implemented or verified by this app. The GPS Smart Remote and GPS Preview Remote are separate products with different compatibility lists.

### GO 3S pairing requirement

**An Action Pod is required for initial pairing.** The GO 3S uses the Action Pod's touchscreen to open **Bluetooth Remote** and select the Flipper. The GO 3S Retro Bundle ships without an Action Pod, so the camera and Retro Viewfinder alone can't complete these pairing steps. Insta360 confirms that the Retro camera can pair with a separately available GO 3S Action Pod. [Bluetooth Remote instructions](https://onlinemanual.insta360.com/gpsactionremote/en-us/troubleshooting/connect-problem) · [Retro Bundle details](https://onlinemanual.insta360.com/go3s/en-us/operating-tutorials/accessories-kit/retro-bundle/retro-bundle-intro)

## Why the firmware patch is needed

The GO 3S didn't list the app on stock firmware 1.5.1. Stock firmware limits the app's advertised name to 16 characters and labels the advertised `CE80` UUID list as incomplete. The app can host the remote's Bluetooth service on stock firmware, but it can't change those two parts of the advertisement.

The [firmware patch](firmware/README.md) advertises the full `Insta360 GPS Remote` name and puts `CE80` in a complete 16-bit service list. With both changes in place, the GO 3S discovered the Flipper, connected and responded to the shutter button. We tested the changes together, so we haven't isolated which one the camera requires.

## Installation

Install the patched firmware first, then install the app. The firmware patch is required for the verified GO 3S setup. Installing the app alone on stock 1.5.1 doesn't let the camera discover the Flipper.

### 1. Build and install the patched firmware

The patch targets **Flipper Zero firmware 1.5.1**. Follow the [firmware build and installation instructions](firmware/README.md) to:

1. Back up the Flipper's internal storage and microSD contents. Keep stock 1.5.1 available for rollback.
2. Apply the patch to the specified 1.5.1 source revision and build the update package.
3. Copy the generated update directory to the microSD card and install it through the normal updater.
4. Let the Flipper reboot before installing the app.

Other firmware releases or forks may need an adapted patch and their own hardware test.

### 2. Build and install the app

With the patched firmware installed:

1. Install [uFBT](https://github.com/flipperdevices/flipperzero-ufbt). From this repo's root, build against the verified f7 SDK for 1.5.1 (API 88.2).

   ```sh
   ufbt update --url https://update.flipperzero.one/builds/firmware/1.5.1/flipper-z-f7-sdk-1.5.1.zip
   ufbt
   ```

2. In qFlipper, copy `dist/insta360_remote.fap` into **SD Card → apps → Bluetooth**. Open **Apps → Bluetooth → Insta360 Shutter** on the Flipper.
3. Follow [Connect and shoot](#connect-and-shoot) to pair the camera and test the shutter.

## Connect and shoot

1. Power on the GO 3S and Action Pod. Open **Bluetooth Remote** on the Action Pod and select **Insta360 GPS Remote**.
2. Wait for **CE82 subscribed** on the Flipper. In **Photo** mode, press **OK** to take a photo. In **Video** mode, press **OK** to start recording and press it again to stop. Check the result on the Action Pod.
3. Press **Back** to exit and restore the Flipper's normal Bluetooth profile.

If the remote is missing, scan from nRF Connect on Android. Look for the full name and a **Complete list of 16-bit Service UUIDs** containing `CE80`. Disconnect any scanner that connected to the Flipper before trying again on the camera.

See the [protocol and hardware notes](docs/protocol-and-testing.md) for the packet format and test scope. The app currently covers shutter control. Mode switching, wake-up, GPS and reconnect after a power cycle are outside the verified path.

## Licensing

The Flipper app source is available under the [MIT License](LICENSE).
The [firmware patch](firmware/insta360-advertising.patch) modifies Flipper firmware and is covered by [GPLv3](firmware/LICENSE).
