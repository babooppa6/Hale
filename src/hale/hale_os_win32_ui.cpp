#include "hale.h"
#include "hale_ui.h"

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#pragma comment(lib, "Gdi32.lib")

namespace hale {

// Required functions to be implemented in hale_platform_win32_<...>.cpp.

hale_internal b32  __os_app_init(App *app);
hale_internal void __os_app_release(App *app);

hale_internal b32  __os_window_init(Window *window);
hale_internal void __os_window_release(Window *window);
hale_internal void __os_window_resize(Window *window, RECT *rc_client);
hale_internal void __os_window_paint(Window *window);

}

#if defined(HALE_PLATFORM_WIN32_GDI)
#include "hale_platform_win32_gdi.cpp"
#elif defined(HALE_PLATFORM_WIN32_DX)
#include "hale_os_win32_dx.cpp"
#endif

namespace hale {

hale_internal uint8
get_key_modifiers()
{
    uint8 modifiers = 0;

    modifiers |= ((::GetKeyState(VK_CONTROL)	    & 0x80000000) != 0) ? KeyEvent::Ctrl : 0;
    modifiers |= ((::GetKeyState(VK_SHIFT)  		& 0x80000000) != 0) ? KeyEvent::Shift : 0;
    modifiers |= ((::GetKeyState(VK_LMENU)  		& 0x80000000) != 0) ? KeyEvent::Alt : 0;
    modifiers |= ((::GetKeyState(VK_RMENU)  		& 0x80000000) != 0) ? KeyEvent::AltGr : 0;
    modifiers |= ((::GetKeyState(VK_LWIN)   		& 0x80000000) != 0) ? KeyEvent::WinLeft : 0;
    modifiers |= ((::GetKeyState(VK_RWIN)   		& 0x80000000) != 0) ? KeyEvent::WinRight : 0;

    return modifiers;
}

struct WM_KEY_LPARAM
{
    WORD repeat;
    BYTE scan_code;
    BYTE extended : 1;
    BYTE reserved : 4;
    BYTE context_code : 1;
    BYTE previous : 1;
    BYTE transition : 1;
};

hale_internal b32
os_app_handle_key(Window *window, UINT message, WPARAM wparam, LPARAM lparam, LRESULT *lresult)
{
    KeyEvent e = {};
    e.modifiers = get_key_modifiers();

    // WM_KEY_LPARAM *klp = (WM_KEY_LPARAM *)msg->lParam;

    switch (message)
    {
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        {
            e.type = KeyEvent::KeyDown;
            e.vkey = (u32)wparam;
            e.codepoint = 0;

            *lresult = 0;
            if (e.vkey == VK_F4 && e.modifiers & KeyEvent::Alt) {
                return 0;
            }
        }
        break;

        case WM_SYSKEYUP:
        case WM_KEYUP:
        {
            e.type = KeyEvent::KeyUp;
            e.vkey = (u32)wparam;
            e.codepoint = 0;

            *lresult = 0;
        }
        break;

        case WM_CHAR:
        {
            e.type = KeyEvent::Char;
            e.vkey = 0;
            e.codepoint = (u32)wparam;
            *lresult = 0;
        }
        break;

        case WM_UNICHAR:
        {
            e.type = KeyEvent::Char;
            e.vkey = 0;
            e.codepoint = (u32)wparam;

            *lresult = (wparam == UNICODE_NOCHAR);
        }
        break;

        default:
        {
            return 0;
        }
        break;
    }

    app_on_key_event(window->app, window, e);

    return 1;
}

// *********************************************************************************
//
//    Window
//
// *********************************************************************************

hale_global
b32 running;

hale_internal LRESULT CALLBACK
os_window_proc(HWND handle,
               UINT message,
               WPARAM wparam,
               LPARAM lparam)
{
    Window *window = (Window*)GetPropA(handle, "hale_window");

    LRESULT lresult;

    switch (message)
    {
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_SYSDEADCHAR:
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_DEADCHAR:
    case WM_CHAR:
    case WM_UNICHAR:
    {
        if (window && os_app_handle_key(window, message, wparam, lparam, &lresult))
        {
            return lresult;
        }
    }
    break;

// TODO: WM_SIZING works with the window rectangle, not the client one!!!
//    case WM_SIZING: {
//        if (window)
//        {
//            // https://msdn.microsoft.com/en-us/library/windows/desktop/ms632647(v=vs.85).aspx
//            RECT *rc = (RECT *)lparam;
//            __os_window_resize(window, rc);
//            // Pass through to DefWindowProc
//        }
//    } break;

    case WM_SIZE: {
        if (window && wparam != SIZE_MINIMIZED)
        {
            RECT rc; // = {0, 0, LOWORD(lparam), HIWORD(lparam)};
            GetClientRect(handle, &rc);
            __os_window_resize(window, &rc);
            // Pass through to DefWindowProc
        }
    } break;

    case WM_PAINT: {
        if (window)
        {
            __os_window_paint(window);
            return 0;
        }
    } break;

    case WM_CLOSE: {
        running = false;
    } break;

    }

    return DefWindowProc(handle, message, wparam, lparam);
}

hale_internal b32
os_window_init(App *app, Window *window, WNDCLASSA *window_class)
{
    *window = {};

    window->app = app;
    window->impl.handle = CreateWindowExA(0,
                                     window_class->lpszClassName,
                                     // TODO:
                                     // Shows as 慈敬 in the title.
                                     // That's a bug, but slash1221 said in the chat.
                                     // that:
                                     //
                                     // 慈 means charity, and 敬 means respect
                                     // http://www.chinesetools.eu/chinese-dictionary/index.php
                                     // http://tangorin.com/general/%E6%85%88

                                     "Hale",
                                     WS_OVERLAPPEDWINDOW,
                                     20, 20, 1280, 720,
                                     0,
                                     0,
                                     window_class->hInstance,
                                     0
                                     );

    if (window->impl.handle == 0)
    {
        win32_print_error("CreateWindowExA");
        hale_panic("CreateWindowExA");
        return 0;
    }

    __os_window_init(window);

    // TODO: DWM

    SetPropA(window->impl.handle, "hale_window", (HANDLE)window);

    // Call to Hale.

    b32 r = window_init(window);

    return r;
}

// *********************************************************************************
//
//   Main
//
// *********************************************************************************

int
main(HINSTANCE instance, int argc, char *argv[])
{
    // TODO: Command line args.
    hale_unused(argc);
    hale_unused(argv);

    running = 1;

    App *app = (App*)malloc(sizeof(App));

    __App *app_impl = &app->impl;
    app_impl->instance = instance;

    __os_app_init(app);

    //
    // Main window class.
    //

    WNDCLASSA window_class;
    ZeroMemory(&window_class, sizeof(window_class));
    window_class.style = CS_OWNDC ; //CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = os_window_proc;
    window_class.hInstance = app_impl->instance;
    window_class.hCursor = LoadCursor(0, IDC_ARROW);
    window_class.lpszClassName = "HaleWindow";

    hale_assert(RegisterClassA(&window_class) != 0);

    hale_assert(os_window_init(app, &app->windows[0], &window_class));

    // TODO: Initialize application.

    ShowWindow(app->windows[0].impl.handle, SW_SHOW);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (!running) {
            break;
        }
    }

    __os_app_release(app);

    return 0;
}

} // namespace hale

#if 1

int
main(int argc, char *argv[])
{
    HINSTANCE hinst = GetModuleHandle(0);
    return hale::main(hinst, argc, argv);
}

#else

#include "hale_test_document.cpp"
#include "hale_test_encoding.cpp"

int
main(int argc, char *argv[])
{
    win32_app app = {};

    hale::test_encoding();
    hale::test_document();

    return 0;
}

#endif
