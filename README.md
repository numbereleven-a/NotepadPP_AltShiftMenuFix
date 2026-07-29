# AltShiftMenuFix plugin for Notepad++

[Русская версия](README_RU.md)

AltShiftMenuFix is a small plugin that prevents the Notepad++ menu
from being activated accidentally after switching the keyboard layout with
<kbd>Alt</kbd>+<kbd>Shift</kbd>.

## Why this happens

Windows uses the release of the Alt key as a signal that an application's menu
bar may need keyboard focus. When Left Alt+Shift is also configured as the
keyboard-layout shortcut, the same key sequence can both change the layout and
generate a keyboard-menu command for Notepad++.

If Alt is released while Shift is still held, Notepad++ may focus its menu bar.
The next typed letter is then interpreted as a menu mnemonic and opens a menu
instead of being inserted into the document.

## What the plugin does

The plugin installs a small message filter on the main Notepad++ window. It
discards only the keyboard-menu command produced by releasing Alt with no menu
letter while Shift is still held.

It does not change:

- the Windows Alt+Shift layout switch;
- normal standalone Alt menu access;
- Alt+letter menu shortcuts;
- other Notepad++ keyboard shortcuts;
- windows created by other plugins.

No settings files, network connections, telemetry, or background processes are
used.

## Reproducing the issue without the plugin

This requires Left Alt+Shift to be configured as the Windows input-language
shortcut:

1. Focus the Notepad++ editor.
2. Press and hold Left Shift.
3. Press and release Left Alt while Shift remains held.
4. Release Shift and type a letter that is used by the Notepad++ menu.

Without the plugin, the menu may receive focus and open. With the plugin
enabled, the letter is inserted into the document normally.

## Installation

1. Download `NppAltShiftMenuFix-1.0-x64.zip` from the Downloads section below.
2. Close every Notepad++ window.
3. Create `%ProgramFiles%\Notepad++\plugins\AltShiftMenuFix` if it does not
   already exist.
4. Extract the single `AltShiftMenuFix.dll` file into that directory.
5. Start Notepad++.

To uninstall the plugin, close Notepad++ and remove the `AltShiftMenuFix`
plugin directory.

## Compatibility

- Windows x64
- Notepad++ x64
- Tested with Notepad++ 8.9.6

## Building

Requirements:

- Windows x64;
- CMake 3.21 or newer;
- Visual Studio 2022 Build Tools, or MinGW-w64 with a resource compiler.

Visual Studio:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
cpack --config build/CPackConfig.cmake -C Release -B dist
```

MinGW-w64 with Ninja:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

The repository contains only the small Notepad++ ABI surface used by this
plugin. It does not compile or ship the unrelated `plugintemplate` demo or its
docking framework.

## License

AltShiftMenuFix is distributed under the
[GNU General Public License v3.0 or later](LICENSE).

## Download

[![GitHub Latest](https://img.shields.io/github/v/release/numbereleven-a/NotepadPP_AltShiftMenuFix)](https://github.com/numbereleven-a/NotepadPP_AltShiftMenuFix/releases/latest)
[![GitHub Downloads](https://img.shields.io/github/downloads/numbereleven-a/NotepadPP_AltShiftMenuFix/total)](https://github.com/numbereleven-a/NotepadPP_AltShiftMenuFix/releases)
