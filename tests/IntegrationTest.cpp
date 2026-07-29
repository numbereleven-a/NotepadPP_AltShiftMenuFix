// SPDX-License-Identifier: GPL-3.0-or-later
#include <windows.h>

#if defined(_MSC_VER)
#pragma comment(linker, "/manifestdependency:\"type='win32' "
                        "name='Microsoft.Windows.Common-Controls' "
                        "version='6.0.0.0' processorArchitecture='*' "
                        "publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#include <cstring>
#include <cwchar>

#include "NppPluginApi.h"

namespace
{
constexpr LRESULT kDeliveredResult = 123;
constexpr int kTestCommandId = 4242;

int g_systemCommandsDelivered = 0;
int g_menuCheckMessages = 0;
WPARAM g_lastMenuCheckCommand = 0;
LPARAM g_lastMenuCheckState = 0;

LRESULT CALLBACK testWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_SYSCOMMAND)
    {
        ++g_systemCommandsDelivered;
        return kDeliveredResult;
    }

    if (message == kNppSetMenuItemCheck)
    {
        ++g_menuCheckMessages;
        g_lastMenuCheckCommand = wParam;
        g_lastMenuCheckState = lParam;
        return TRUE;
    }

    return DefWindowProcW(window, message, wParam, lParam);
}

bool setShiftState(bool pressed)
{
    BYTE state[256]{};
    if (!GetKeyboardState(state))
        return false;

    state[VK_SHIFT] = pressed ? 0x80 : 0x00;
    state[VK_LSHIFT] = pressed ? 0x80 : 0x00;
    return SetKeyboardState(state) != FALSE;
}

class ShiftStateGuard
{
public:
    ~ShiftStateGuard()
    {
        setShiftState(false);
    }
};

class TestWindow
{
public:
    TestWindow()
    {
        WNDCLASSW windowClass{};
        windowClass.lpfnWndProc = testWindowProc;
        windowClass.hInstance = GetModuleHandleW(nullptr);
        windowClass.lpszClassName = className_;

        instance_ = windowClass.hInstance;
        registered_ = RegisterClassW(&windowClass) != 0;
        if (!registered_)
            return;

        window_ = CreateWindowExW(
            0,
            className_,
            L"AltShiftMenuFix integration test",
            WS_OVERLAPPED,
            0,
            0,
            200,
            100,
            nullptr,
            nullptr,
            instance_,
            nullptr);
    }

    ~TestWindow()
    {
        if (window_ != nullptr)
            DestroyWindow(window_);
        if (registered_)
            UnregisterClassW(className_, instance_);
    }

    TestWindow(const TestWindow&) = delete;
    TestWindow& operator=(const TestWindow&) = delete;

    HWND get() const
    {
        return window_;
    }

private:
    static constexpr wchar_t className_[] = L"AltShiftMenuFixTestWindow";
    HINSTANCE instance_ = nullptr;
    HWND window_ = nullptr;
    bool registered_ = false;
};

class LoadedModule
{
public:
    explicit LoadedModule(const char* path) : module_(LoadLibraryA(path))
    {
    }

    ~LoadedModule()
    {
        if (module_ != nullptr)
            FreeLibrary(module_);
    }

    LoadedModule(const LoadedModule&) = delete;
    LoadedModule& operator=(const LoadedModule&) = delete;

    HMODULE get() const
    {
        return module_;
    }

private:
    HMODULE module_ = nullptr;
};

using BeNotified = void(__cdecl*)(NppNotification*);

class NotificationGuard
{
public:
    NotificationGuard(BeNotified callback, HWND window) : callback_(callback), window_(window)
    {
    }

    ~NotificationGuard()
    {
        shutdown();
    }

    NotificationGuard(const NotificationGuard&) = delete;
    NotificationGuard& operator=(const NotificationGuard&) = delete;

    void ready()
    {
        NppNotification notification{};
        notification.header.hwndFrom = window_;
        notification.header.code = kNppReady;
        callback_(&notification);
        active_ = true;
    }

    void shutdown()
    {
        if (!active_)
            return;

        NppNotification notification{};
        notification.header.hwndFrom = window_;
        notification.header.code = kNppShutdown;
        callback_(&notification);
        active_ = false;
    }

private:
    BeNotified callback_ = nullptr;
    HWND window_ = nullptr;
    bool active_ = false;
};

template <typename FunctionPointer>
FunctionPointer loadFunction(HMODULE module, const char* name)
{
    const FARPROC rawAddress = GetProcAddress(module, name);
    static_assert(sizeof(FunctionPointer) == sizeof(rawAddress));

    FunctionPointer function = nullptr;
    std::memcpy(&function, &rawAddress, sizeof(function));
    return function;
}

bool expectDelivered(HWND window, WPARAM command, LPARAM argument, int expectedCount)
{
    return SendMessageW(window, WM_SYSCOMMAND, command, argument) == kDeliveredResult &&
        g_systemCommandsDelivered == expectedCount;
}
}

int main(int argc, char** argv)
{
    if (argc != 2)
        return 10;

    ShiftStateGuard shiftStateGuard;
    TestWindow testWindow;
    if (testWindow.get() == nullptr)
        return 11;

    LoadedModule plugin(argv[1]);
    if (plugin.get() == nullptr)
        return 12;

    using SetInfo = void(__cdecl*)(NppData);
    using GetName = const wchar_t*(__cdecl*)();
    using GetFuncsArray = FuncItem*(__cdecl*)(int*);
    using IsUnicode = BOOL(__cdecl*)();
    using MessageProc = LRESULT(__cdecl*)(UINT, WPARAM, LPARAM);

    const auto setInfo = loadFunction<SetInfo>(plugin.get(), "setInfo");
    const auto getName = loadFunction<GetName>(plugin.get(), "getName");
    const auto getFuncsArray = loadFunction<GetFuncsArray>(plugin.get(), "getFuncsArray");
    const auto beNotified = loadFunction<BeNotified>(plugin.get(), "beNotified");
    const auto isUnicode = loadFunction<IsUnicode>(plugin.get(), "isUnicode");
    const auto messageProc = loadFunction<MessageProc>(plugin.get(), "messageProc");
    if (setInfo == nullptr || getName == nullptr || getFuncsArray == nullptr ||
        beNotified == nullptr || isUnicode == nullptr || messageProc == nullptr)
        return 13;

    if (std::wcscmp(getName(), L"AltShiftMenuFix") != 0 || !isUnicode() ||
        messageProc(0, 0, 0) != TRUE)
        return 14;

    NppData data{};
    data.nppHandle = testWindow.get();
    setInfo(data);

    int commandCount = 0;
    FuncItem* commands = getFuncsArray(&commandCount);
    if (commands == nullptr || commandCount != 2 || getFuncsArray(nullptr) != commands)
        return 15;
    if (std::wcscmp(commands[0].itemName, L"Enabled for this session") != 0 ||
        !commands[0].initiallyChecked || commands[0].function == nullptr)
        return 16;
    if (std::wcscmp(commands[1].itemName, L"About...") != 0 ||
        commands[1].function == nullptr)
        return 17;

    commands[0].commandId = kTestCommandId;
    NotificationGuard notifications(beNotified, testWindow.get());
    notifications.ready();

    if (!setShiftState(true))
        return 18;

    if (!expectDelivered(testWindow.get(), SC_CLOSE, 0, 1) ||
        !expectDelivered(testWindow.get(), SC_MINIMIZE, 0, 2) ||
        !expectDelivered(testWindow.get(), SC_MOVE, 0, 3))
        return 20;

    if (!expectDelivered(testWindow.get(), SC_KEYMENU, L' ', 4) ||
        !expectDelivered(testWindow.get(), SC_KEYMENU, L'F', 5))
        return 21;

    const LRESULT lowBitsResult = SendMessageW(
        testWindow.get(), WM_SYSCOMMAND, SC_KEYMENU | 0x0FU, 0);
    if (lowBitsResult != 0 || g_systemCommandsDelivered != 5)
        return 22;

    const LRESULT blockedResult = SendMessageW(testWindow.get(), WM_SYSCOMMAND, SC_KEYMENU, 0);
    if (blockedResult != 0 || g_systemCommandsDelivered != 5)
        return 23;

    if (!setShiftState(false) ||
        !expectDelivered(testWindow.get(), SC_KEYMENU, 0, 6))
        return 24;

    if (!setShiftState(true))
        return 25;

    commands[0].function();
    if (g_menuCheckMessages != 1 ||
        g_lastMenuCheckCommand != static_cast<WPARAM>(kTestCommandId) ||
        g_lastMenuCheckState != FALSE)
        return 26;
    if (!expectDelivered(testWindow.get(), SC_KEYMENU, 0, 7))
        return 27;

    notifications.shutdown();
    commands[0].function();
    if (g_menuCheckMessages != 2 || g_lastMenuCheckState != TRUE)
        return 28;
    if (!expectDelivered(testWindow.get(), SC_KEYMENU, 0, 8))
        return 29;

    return 0;
}
