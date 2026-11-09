# Working on Insta360 Shutter

## Project and compatibility

- The app source is `insta360_remote.c`. Its Flipper app manifest is `application.fam`.
- Read `README.md`, `docs/protocol-and-testing.md` and `firmware/README.md` before changing Bluetooth behavior.
- Preserve the hardware-verified path from 8 November 2026. The GO 3S discovered the Flipper on patched 1.5.1, connected, subscribed to CE82 and started and stopped video with two OK presses. Both phone remotes were off. The repository build has a renamed menu entry and passed the build check. The hardware test used the earlier label.
- Stock 1.5.1 hosted the same CE80 service but the GO 3S skipped discovery. The patch targets the app's CE80 profile and complete-name marker `Insta360 GPS Rem`. It advertises `Insta360 GPS Remote` and a complete 16-bit CE80 list without changing the app API. Treat the marker, name type and UUID as a contract with the patch.
- The successful test used the full name, complete service list and captured manufacturer data together. Their individual effects are unverified. Keep this configuration as the working baseline when investigating discovery.
- Mode switching is paused. One Android mode-cycle notification produced no visible change on the GO 3S. The supported action is shutter start/stop. Wake-up, GPS, power-cycle reconnect and other camera models need hardware verification.

## Build and verification

- Build against the pinned Flipper 1.5.1 f7 SDK. A successful build verifies compilation and API imports. Hardware behavior needs a separate camera test.
- Use `ufbt` from the repository root. If the SDK needs setup, use the exact SDK URL in `README.md`. Do not silently switch to the latest SDK.
- For firmware patch changes, run `git apply --check` against the upstream revision specified in `firmware/README.md` when that checkout is available.
- Report the checks actually run and any hardware validation still needed. Do not claim camera behavior from a successful build alone.
- On hardware, check the complete advertised name and CE80 list first. Then verify camera discovery, CE82 subscription and the Action Pod recording indicator after each OK press. Stop the Android advertiser and close Multicam before this test.
- Keep CE83's `01 02` value and the connection-local shutter sequence, which starts at zero and advances by two per press. The two verified frames are in `docs/protocol-and-testing.md`. On SDK 88.2, `ble_gatt_characteristic_update()` returns true on error.
- Preserve restoration of the Flipper's normal Bluetooth profile when the app exits. For an authorized firmware install, use the normal updater and keep the pre-update backups and stock 1.5.1 available for rollback.

## Scope and repository hygiene

- Keep firmware flashing, GitHub publishing and release uploads separate from source changes. Keep firmware binaries, captures, device identifiers and backups out of Git.
- The app currently implements the shutter toggle. Record experiments separately from hardware-verified behavior.
- Update the relevant documentation when behavior, compatibility or build instructions change.
- Preserve intentional local changes. Do not add generated SDK files or firmware checkouts to the repository.
- Keep the license split explicit. The app source is MIT and the firmware patch is GPLv3. Get the maintainer's instruction before changing either license.

## Writing

- Use conversational, direct and technically informed plain English. Use contractions and active voice.
- Keep paragraphs compact. Use lists for steps or comparisons.
- Preserve supplied experience and judgment. Do not invent hardware tests, personal experiences or compatibility claims.
- Avoid em dashes, Oxford commas, a comma before "and", hype and filler.

## Screen preview

Regenerate the SVG with `python3 scripts/render_screen.py` after changing `remote_screen.h`. The preview uses approximate fonts rather than the firmware fonts. The screen reports connection readiness and commands sent, not camera recording state.
