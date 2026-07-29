// SPDX-License-Identifier: GPL-3.0-or-later
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <strsafe.h>

#include <cwchar>
#include <iterator>

#include "NppPluginApi.h"
#include "Version.h"

namespace
{
constexpr wchar_t kPluginName[] = L"AltShiftMenuFix";
constexpr wchar_t kEnabledCommandName[] = L"Enabled for this session";
constexpr wchar_t kAboutCommandName[] = L"About...";
constexpr wchar_t kRepositoryUrl[] =
    L"https://github.com/numbereleven-a/Notepadpp_AltShiftMenuFix";
constexpr UINT_PTR kSubclassId = 0x41534D46; // "ASMF"
constexpr int kCommandCount = 2;
constexpr int kEnabledCommand = 0;

static_assert(std::size(kEnabledCommandName) <= kNppMenuItemSize);
static_assert(std::size(kAboutCommandName) <= kNppMenuItemSize);

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

HRESULT CALLBACK aboutDialogCallback(HWND, UINT notification, WPARAM, LPARAM parameter, LONG_PTR)
{
    if (notification == TDN_HYPERLINK_CLICKED && parameter != 0)
    {
        const auto* url = reinterpret_cast<const wchar_t*>(parameter);
        if (std::wcscmp(url, kRepositoryUrl) == 0)
            ShellExecuteW(nullptr, L"open", url, nullptr, nullptr, SW_SHOWNORMAL);
    }

    return S_OK;
}

void showAbout()
{
    constexpr wchar_t kDescription[] =
        L"Prevents the Notepad++ menu from being activated accidentally when "
        L"Alt is released while Shift is still held during an Alt+Shift keyboard "
        L"layout switch.\n\n"
        L"Normal Alt menu access and Alt+letter shortcuts are not changed.\n\n"
        L"GitHub: <a href=\"https://github.com/numbereleven-a/"
        L"Notepadpp_AltShiftMenuFix\">"
        L"https://github.com/numbereleven-a/Notepadpp_AltShiftMenuFix</a>";

    TASKDIALOGCONFIG config{};
    config.cbSize = sizeof(config);
    config.hwndParent = g_nppData.nppHandle;
    config.dwFlags = TDF_ENABLE_HYPERLINKS | TDF_ALLOW_DIALOG_CANCELLATION;
    config.dwCommonButtons = TDCBF_OK_BUTTON;
    config.pszWindowTitle = kPluginName;
    config.pszMainInstruction = L"AltShiftMenuFix " ASMF_VERSION_W;
    config.pszContent = kDescription;
    config.pszFooter = L"License: GPL-3.0-or-later";
    config.pfCallback = aboutDialogCallback;

    if (FAILED(TaskDialogIndirect(&config, nullptr, nullptr, nullptr)))
    {
        MessageBoxW(
            g_nppData.nppHandle,
            L"AltShiftMenuFix " ASMF_VERSION_W L"\n\n"
            L"License: GPL-3.0-or-later\n"
            L"GitHub: https://github.com/numbereleven-a/Notepadpp_AltShiftMenuFix",
            kPluginName,
            MB_OK | MB_ICONINFORMATION);
    }
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
