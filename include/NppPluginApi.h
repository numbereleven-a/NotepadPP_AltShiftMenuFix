// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <windows.h>

#include <cstddef>

// Minimal ABI surface required by this plugin. The layouts and message values
// match the documented Notepad++ plugin interface. Keeping this header small
// avoids importing the unrelated demonstration and docking code from the
// official plugin template.

struct NppData
{
    HWND nppHandle = nullptr;
    HWND scintillaMainHandle = nullptr;
    HWND scintillaSecondHandle = nullptr;
};

using PluginCommand = void(__cdecl*)();

struct ShortcutKey
{
    bool isCtrl = false;
    bool isAlt = false;
    bool isShift = false;
    UCHAR key = 0;
};

constexpr int kNppMenuItemSize = 64;

struct FuncItem
{
    wchar_t itemName[kNppMenuItemSize]{};
    PluginCommand function = nullptr;
    int commandId = 0;
    bool initiallyChecked = false;
    ShortcutKey* shortcut = nullptr;
};

// Every Notepad++/Scintilla notification starts with NMHDR. This plugin only
// consumes the notification code, so the remainder of SCNotification is not
// needed and is deliberately not duplicated here.
struct NppNotification
{
    NMHDR header{};
};

static_assert(sizeof(NppData) == 24);
static_assert(sizeof(ShortcutKey) == 4);
static_assert(offsetof(NMHDR, code) == 16);
static_assert(offsetof(FuncItem, function) == 128);
static_assert(offsetof(FuncItem, commandId) == 136);
static_assert(offsetof(FuncItem, initiallyChecked) == 140);
static_assert(offsetof(FuncItem, shortcut) == 144);
static_assert(sizeof(FuncItem) == 152);

constexpr UINT kNppMessageBase = WM_USER + 1000;
constexpr UINT kNppSetMenuItemCheck = kNppMessageBase + 40;

constexpr UINT kNppNotificationFirst = 1000;
constexpr UINT kNppReady = kNppNotificationFirst + 1;
constexpr UINT kNppShutdown = kNppNotificationFirst + 9;
