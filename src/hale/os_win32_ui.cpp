#if HALE_INCLUDES
#include "hale.h"
#include "hale_key.h"
#include "hale_ui.h"
#include "hale_stack_memory.h"
#endif

#include <windowsx.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Shell32.lib")

#include <dwmapi.h>

namespace hale {

hale_global PagedMemory __stack__;

// Required functions to be implemented in hale_platform_win32_<...>.cpp.

hale_internal b32  __app_init(HALE_STACK, App *app);
hale_internal void __os_app_release(App *app);

hale_internal b32  __os_window_init(HALE_STACK, Window *window);
hale_internal void __os_window_release(HALE_STACK, Window *window);
hale_internal void __os_window_resize(HALE_STACK, Window *window, RECT *rc_client);
hale_internal void __os_window_paint(HALE_STACK, Window *window);

}

#if defined(HALE_OS_WIN_GDI)
#include "os_win32_gdi.cpp"
#elif defined(HALE_OS_WIN_DX)
#include "os_win32_dx.cpp"
#endif

namespace hale {

hale_internal u8
_get_key_modifiers()
{
    u8 modifiers = 0;

    modifiers |= ((::GetKeyState(VK_CONTROL)	    & 0x80000000) != 0) ? (u8)KeyM_Ctrl : 0;
    modifiers |= ((::GetKeyState(VK_SHIFT)  		& 0x80000000) != 0) ? (u8)KeyM_Shift : 0;
    modifiers |= ((::GetKeyState(VK_LMENU)  		& 0x80000000) != 0) ? (u8)KeyM_Alt : 0;
    modifiers |= ((::GetKeyState(VK_RMENU)  		& 0x80000000) != 0) ? (u8)KeyM_AltGr : 0;
    modifiers |= ((::GetKeyState(VK_LWIN)   		& 0x80000000) != 0) ? (u8)KeyM_WinLeft : 0;
    modifiers |= ((::GetKeyState(VK_RWIN)   		& 0x80000000) != 0) ? (u8)KeyM_WinRight : 0;

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
_app_handle_key(PagedMemory *stack,
                Window *window,
                UINT message,
                WPARAM wparam,
                LPARAM lparam,
                LRESULT *lresult)
{
    KeyEvent e = {};
    e.key.modifiers = _get_key_modifiers();

    // WM_KEY_LPARAM *klp = (WM_KEY_LPARAM *)msg->lParam;

    switch (message)
    {
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        {
            e.type = KeyT_KeyDown;
            hale_assert(wparam <= 0xFF);
            e.key.key = (u8)wparam;

            *lresult = 0;
            if (e.key.key == VK_F4 && (e.key.modifiers & KeyM_Alt)) {
                return 0;
            }
        }
        break;

        case WM_SYSKEYUP:
        case WM_KEYUP:
        {
            e.type = KeyT_KeyUp;
            hale_assert(wparam <= 0xFF);
            e.key.key = (u8)wparam;

            *lresult = 0;
        }
        break;

        case WM_CHAR:
        {
            e.type = KeyT_Text;
            e.key.codepoint = (u32)wparam;
            *lresult = 0;
        }
        break;

        case WM_UNICHAR:
        {
            e.type = KeyT_Text;
            e.key.codepoint = (u32)wparam;

            *lresult = (wparam == UNICODE_NOCHAR);
        }
        break;


        default:
        {
            return 0;
        }
        break;
    }

    app_on_key_event(stack, window->app, window, e);

    return 1;
}

// *********************************************************************************
//
//    Window
//
// *********************************************************************************

hale_internal LRESULT CALLBACK
_window_proc(HWND handle,
             UINT message,
             WPARAM wparam,
             LPARAM lparam)
{
    Window *window = (Window*)GetPropA(handle, "hale_window");
    HALE_STACK = &__stack__;
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
        if (window && _app_handle_key(stack, window, message, wparam, lparam, &lresult))
        {
            return lresult;
        }
    }
    break;

    case WM_MOUSEWHEEL: {
        if (window) {
            r32 x = GET_X_LPARAM(lparam);
            r32 y = GET_Y_LPARAM(lparam);
            // WORD fwKeys = GET_KEYSTATE_WPARAM(wparam);
            window->impl.wheel_delta += GET_WHEEL_DELTA_WPARAM(wparam);
            r32 angle = (r32)window->impl.wheel_delta / 120;
            window->impl.wheel_delta = window->impl.wheel_delta % 120;
            window_scroll_by(stack, window, x, y, 0, -angle);
            return 0;
        }
    } break;

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
            __os_window_resize(stack, window, &rc);
            // Pass through to DefWindowProc
        }
    } break;

    case WM_PAINT: {
        if (window)
        {
            __os_window_paint(stack, window);
            return 0;
        }
    } break;

    case WM_CLOSE: {
        if (window) {
            // TODO: Test whether to close (last window, unsaved changes, etc.)
            window->app->running = 0;
        }
    } break;

    }

    return DefWindowProc(handle, message, wparam, lparam);
}

hale_internal b32
_window_init(PagedMemory *stack, App *app, Window *window, WNDCLASSA *window_class)
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

    __os_window_init(stack, window);

    // TODO: DWM

    SetPropA(window->impl.handle, "hale_window", (HANDLE)window);

    // Call to Hale.

    b32 r = window_init(stack, window);

    return r;
}

void
window_invalidate(Window *window)
{
    // TODO: To get an extreme redraw speed, we can first redraw the primary
    //       section of the screen (the edited line) and then go to other parts.
    // window->invalidated = 1;

#if 1
    InvalidateRect(window->impl.handle, NULL, FALSE);
#else
    __os_window_paint(window);
#endif
}

//
//
//

Animation *
window_get_animation(Window *window, void *key)
{
    Animation *f;
    for (memi i = 0; i != window->animations.count; i++)
    {
        f = &window->animations.ptr[i];
        if (f->_key == key) {
            return f;
        }
    }
    return 0;
}

Animation *
window_add_animation(Window *window, void *key, Animation *animation)
{
    hale_assert_debug(window_get_animation(window, key) == 0);
    if (window->animations.count != hale_array_count(window->animations.ptr))
    {
        auto a = memory_insert(&window->animations, window->animations.count, 1);
        *a = *animation;
        a->_key = key;
        a->elapsed = 0;

        if (window->app->impl.animations_count == 0)
        {
            window->app->impl.animations_time = platform.read_time_counter();
            window->app->impl.animations_count++;
        }

        return a;
    }

    return 0;
}

hale_internal void
_run_animations(HALE_STACK, App *app, r32 dt)
{
    Window *window;
    for (memi iw = 0; iw != app->windows_count; iw++)
    {
        window = &app->windows[iw];
        if (window->animations.count)
        {
            Animation *a;
            for (memi i = 0; i != window->animations.count; i++)
            {
                a = &window->animations.ptr[i];
                a->elapsed += dt;
                a->animate(stack, clamp01(a->elapsed / a->duration), a);
                if (a->elapsed > a->duration) {
                    memory_remove_ordered(&window->animations, i, 1);
                    --i;
                    app->impl.animations_count--;
                }
            }
            // TODO: the animation should invalidate the portion of the window, and here we'll just do what WM_PAINT would.
            __os_window_paint(stack, window);
        }
    }
}

//
// Idle processing.
//

//VOID CALLBACK _idle_proc(HWND     handle,
//                         UINT     message,
//                         UINT_PTR timer_id,
//                         DWORD    time)
//{
//    print("idle");
//}

// *********************************************************************************
//
//   Main
//
// *********************************************************************************

void
app_resume_parsing(App *app)
{
    if (app->impl.parsing_timer == 0) {
        // Print() << "Starting parsing timer.";
        app->impl.parsing_timer = SetTimer(0, 0, 50, 0);
        hale_assert_message(app->impl.parsing_timer != 0, "SetTimer");
    }
}

void
app_suspend_parsing(App *app)
{
    if (app->impl.parsing_timer) {
        // Print() << "Killing parsing timer.";
        KillTimer(0, app->impl.parsing_timer);
        app->impl.parsing_timer = 0;
    }
}

int
main(HINSTANCE instance, s32 argc, ch16 *argv[])
{
    __stack__ = {};

#if HALE_DEBUG
    memory_init(&__stack__, hale_align_up_to_page_size(hale_gigabytes(1)), (void*)hale_terabytes(2));
#else
    memory_init(&__stack__, hale_align_up_to_page_size(hale_gigabytes(1)), 0);
#endif

	if (__stack__.base == 0) {
		win32_print_error("VirtualAlloc");
        hale_error("VirtualAlloc");
	}

    StackMemory<App> app_(&__stack__, 1);

    // App *app = (App*)malloc(sizeof(App));

    App *app = app_.ptr();
    *app = {};
    app->argc = argc;
    app->argv = argv;

    __App *app_impl = &app->impl;
    app_impl->instance = instance;

    __app_init(&__stack__, app);

    //
    // Main window class.
    //

    WNDCLASSA window_class;
    ZeroMemory(&window_class, sizeof(window_class));
    window_class.style = CS_OWNDC; //CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = _window_proc;
    window_class.hInstance = app_impl->instance;
    window_class.hCursor = LoadCursor(0, IDC_ARROW);
    window_class.lpszClassName = "HaleWindow";

    hale_assert(RegisterClassA(&window_class) != 0);

    hale_assert(_window_init(&__stack__, app, &app->windows[0], &window_class));
    app->windows_count++;

    // TODO: Initialize application.

    ShowWindow(app->windows[0].impl.handle, SW_SHOW);

    int exit_code = EXIT_FAILURE;
    MSG msg = {};

    // UINT_PTR idle_timer = SetTimer(0, 0, USER_TIMER_MINIMUM, &_idle_proc);

    app->running = 1;

    while(app->running)
    {
        while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                exit_code = (int)msg.wParam;
                app->running = 0;
                break;
            }
            else if (app->impl.animations_count == 0 && msg.message == WM_TIMER && msg.wParam == app->impl.parsing_timer)
            {
                // TODO: Try to implement without the timer, with our
                //       idle monitoring / interleaving.
                if (app_on_parse_event(app) == 0) {
                    app_suspend_parsing(app);
                }

                continue;
            }

            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        if (app->impl.animations_count)
        {
            r64 _time = platform.read_time_counter();
            r32 dt = _time - app->impl.animations_time;
            app->impl.animations_time = _time;

            _run_animations(&__stack__, app, dt);

            // Pom.. pom..
            if (dt < 0.016f) {
                Sleep((0.016f - dt) * 1000.0f);
            }
        } else if (app->running) {
            WaitMessage();
        }
    }

//    while (GetMessage(&msg, NULL, 0, 0) > 0)
//    {
//        TranslateMessage(&msg);
//        DispatchMessage(&msg);

//        if (!running) {
//            break;
//        }
//    }

    __os_app_release(app);

    // Print() << "StackAllocator" << ((r64)__stack__.capacity / (1024*1024)) << "MB";

    return exit_code;
}

} // namespace hale

//
//
//

//
//
//

//
// Build with bat file.
//

#if HALE_TEST
#include "test.h"
#endif

int CALLBACK
wWinMain(HINSTANCE hinst,
        HINSTANCE hint_previous,
        LPWSTR command_line,
        int show_code)
{
    int argc = 0;
	LPWSTR *argv = 0;
	if (*command_line != 0) {
		argv = CommandLineToArgvW(command_line, &argc);
	}
    // TODO: Parse the argv/argc to an internal vector or even the command line ast.
#if HALE_TEST
    int exit_code = hale::test_main((hale::s32)argc, (hale::ch**)argv);
#else
    int exit_code = hale::main(hinst, (hale::s32)argc, (hale::ch**)argv);
#endif
    LocalFree(argv);

    return exit_code;
}
