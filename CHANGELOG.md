# Changelog

All notable changes to this project will be documented in this file.

## 1.0 - 2026-07-29

- Prevent accidental Notepad++ menu activation after an Alt+Shift layout switch.
- Preserve normal standalone Alt and Alt+letter menu behavior.
- Add a session-only enable/disable command.
- Add x64 ABI assertions, version metadata, hardened build flags, integration
  tests, and a Windows GitHub Actions workflow.
- Build MSVC releases with the static runtime and verify both MSVC and MinGW in
  CI.
- Package Plugin Admin-compatible ZIP files with the DLL at the archive root.
- Add English and Russian project documentation explaining the Windows menu
  activation behavior and the plugin's narrow message filter.
- Show the version, license, and GitHub repository in the About dialog.
- Keep the downloadable release ZIP minimal: it contains only the plugin DLL.
