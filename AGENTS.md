# Working on Insta360 Shutter

- Preserve the GO 3S discovery, connection and shutter path documented in README.md.
- Treat the app's advertised name, CE80 UUID and advertising name type as a contract with the firmware patch. Change both sides together when needed.
- Build against the pinned Flipper 1.5.1 f7 SDK. A successful build verifies compilation and API imports. Hardware behavior needs a separate camera test.
- Keep firmware flashing, GitHub publishing and release uploads separate from source changes. Keep firmware binaries, captures, device identifiers and backups out of Git.
- The app currently implements the shutter toggle. Record experiments separately from hardware-verified behavior.
