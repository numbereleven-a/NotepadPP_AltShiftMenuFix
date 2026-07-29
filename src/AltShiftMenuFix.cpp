// SPDX-License-Identifier: GPL-3.0-or-later
#include <windows.h>
#include <commctrl.h>
#include <strsafe.h>

#include "NppPluginApi.h"
#include "Version.h"

namespace
{
constexpr wchar_t kPluginName[] = L"AltShiftMenuFix";
constexpr wchar_t kEnabledCommandName[] = L"Enabled for this session";
constexpr wchar_t kAboutCommandName[] = L"About...";
constexpr UINT_PTR kSubclassId = 0x41534D46; // "ASMF"
constexpr int kCommandCount = 2;
constexpr int kEnabledCommand = 0;

static_assert(_countof(kEnabledCommandName) <= kNppMenuItemSize);
static_assert(_countof(kAboutCommandName) <= kNppMenuItemSize);

NppData g_nppData{};
FuncItem g_commands[kCommandCount]{};
bool g_enabled = true;
bool g_subclassInstalled = false;

void updateMenuCheck()
{
    if (g_nppData.nppHandle != nullptr && g_commands[kEnabledCommand].commandId != 0)
    {
        SendMessageW(
            g_nppData.nppHandle,
            kNppSetMenuItemCheck,
            static_cast<WPARAM>(g_commands[kEnabledCommand].commandId),
            static_cast<LPARAM>(g_enabled));
    }
}

void toggleEnabled()
{
    g_enabled = !g_enabled;
    updateMenuCheck();
}

void showAbout()
{
    MessageBoxW(
        g_nppData.nppHandle,
        L"AltShiftMenuFix " ASMF_VERSION_W L"\n\n"
        L"Prevents the Notepad++ menu from being activated accidentally when "
        L"Alt is released while Shift is still held during an Alt+Shift keyboard "
        L"layout switch.\n\n"
        L"Normal Alt menu access and Alt+letter shortcuts are not changed.\n\n"
        L"License: GPL-3.0-or-later\n"
        L"GitHub: https://github.com/numbereleven-a/Notepadpp_AltShiftMenuFix",
        kPluginName,
        MB_OK | MB_ICONINFORMATION);
}

void setCommand(int index, const wchar_t* name, PluginCommand callback, bool checked = false)
{
    const HRESULT copyResult = StringCchCopyW(
        g_commands[index].itemName,
        kNppMenuItemSize,
        name);

    if (FAILED(copyResult))
    {
        g_commands[index].itemName[0] = L'\0';
        g_commands[index].function = nullptr;
        return;
    }

    g_commands[index].function = callback;
    g_commands[index].initiallyChecked = checked;
    g_commands[index].shortcut = nullptr;
}

bool shouldSuppressMenuActivation(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (!g_enabled || message != WM_SYSCOMMAND)
        return false;

    const bool isKeyboardMenuCommand = (wParam & 0xFFF0U) == SC_KEYMENU;
    const bool isStandaloneAlt = lParam == 0;
    const bool shiftIsStillHeld = (GetKeyState(VK_SHIFT) & 0x8000) != 0;

    return isKeyboardMenuCommand && isStandaloneAlt && shiftIsStillHeld;
}

LRESULT CALLBACK nppSubclassProc(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    UINT_PTR,
    DWORD_PTR)
{
    if (shouldSuppressMenuActivation(message, wParam, lParam))
        return 0;

    return DefSubclassProc(window, message, wParam, lParam);
}

void installSubclass()
{
    if (!g_subclassInstalled && g_nppData.nppHandle != nullptr)
    {
        g_subclassInstalled = SetWindowSubclass(
            g_nppData.nppHandle,
            nppSubclassProc,
            kSubclassId,
            0) != FALSE;

        if (!g_subclassInstalled)
            OutputDebugStringW(L"AltShiftMenuFix: SetWindowSubclass failed.\n");
    }
}

void removeSubclass()
{
    // Notepad++ sends NPPN_SHUTDOWN before unloading plugins. The subclass must
    // be removed here, while this DLL and nppSubclassProc are still valid.
    // Do not move this operation to DllMain: it would execute under loader lock.
    if (g_subclassInstalled && g_nppData.nppHandle != nullptr)
    {
        RemoveWindowSubclass(g_nppData.nppHandle, nppSubclassProc, kSubclassId);
        g_subclassInstalled = false;
    }
}
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
        DisableThreadLibraryCalls(module);

    return TRUE;
}

extern "C" __declspec(dllexport) void setInfo(NppData data)
{
    g_nppData = data;
    setCommand(kEnabledCommand, kEnabledCommandName, toggleEnabled, true);
    setCommand(1, kAboutCommandName, showAbout);
}

extern "C" __declspec(dllexport) const wchar_t* getName()
{
    return kPluginName;
}

extern "C" __declspec(dllexport) FuncItem* getFuncsArray(int* count)
{
    if (count != nullptr)
        *count = kCommandCount;

    return g_commands;
}

extern "C" __declspec(dllexport) void beNotified(NppNotification* notification)
{
    if (notification == nullptr)
        return;

    switch (notification->header.code)
    {
        case kNppReady:
            installSubclass();
            break;

        case kNppShutdown:
            removeSubclass();
            break;

        default:
            break;
    }
}

extern "C" __declspec(dllexport) LRESULT messageProc(UINT, WPARAM, LPARAM)
{
    // Required by the Notepad++ plugin ABI. This plugin does not consume any
    // messages through messageProc; the official template also returns TRUE.
    return TRUE;
}

extern "C" __declspec(dllexport) BOOL isUnicode()
{
    return TRUE;
}
