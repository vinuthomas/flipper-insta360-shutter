# Working on Insta360 Shutter

## Project and compatibility

- The app source is `insta360_remote.c`. Its Flipper app manifest is `application.fam`.
- Read `README.md`, `docs/protocol-and-testing.md` and `firmware/README.md` before changing Bluetooth behavior.
- Preserve the GO 3S discovery, connection and shutter path documented in README.md.
- Treat the app's advertised name, CE80 UUID and advertising name type as a contract with the firmware patch. Change both sides together when needed.

## Build and verification

- Build against the pinned Flipper 1.5.1 f7 SDK. A successful build verifies compilation and API imports. Hardware behavior needs a separate camera test.
- Use `ufbt` from the repository root. If the SDK needs setup, use the exact SDK URL in `README.md`. Do not silently switch to the latest SDK.
- For firmware patch changes, run `git apply --check` against the upstream revision specified in `firmware/README.md` when that checkout is available.
- Report the checks actually run and any hardware validation still needed. Do not claim camera behavior from a successful build alone.
- Preserve restoration of the Flipper's normal Bluetooth profile when the app exits.

## Scope and repository hygiene

- Keep firmware flashing, GitHub publishing and release uploads separate from source changes. Keep firmware binaries, captures, device identifiers and backups out of Git.
- The app currently implements the shutter toggle. Record experiments separately from hardware-verified behavior.
- Update the relevant documentation when behavior, compatibility or build instructions change.
- Preserve intentional local changes. Do not add generated SDK files or firmware checkouts to the repository.
- An app-source license is pending maintainer selection. Do not select or replace it without the maintainer's instruction.

## Writing

- Use conversational, direct and technically informed plain English. Use contractions and active voice.
- Keep paragraphs compact. Use lists for steps or comparisons.
- Preserve supplied experience and judgment. Do not invent hardware tests, personal experiences or compatibility claims.
- Avoid em dashes, Oxford commas, a comma before "and", hype and filler.
