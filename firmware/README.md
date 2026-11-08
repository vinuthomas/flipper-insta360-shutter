# Firmware patch for Flipper Zero 1.5.1

The patch applies to official Flipper Zero firmware 1.5.1 at commit `9e810dc399e027e48ce07751df72168abbb19065`. It changes advertising for the profile with service `CE80` and the complete-name marker `Insta360 GPS Rem`. That profile gets the full `Insta360 GPS Remote` name and a complete 16-bit service list. The marker fits the stock app interface, so the patch preserves the existing `GapConfig` layout and SDK API 88.2.

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
