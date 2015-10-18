#include "hale.h"
#include "hale_platform_win32_ui.h"

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#pragma comment(lib, "Gdi32.lib")

namespace hale {

hale_internal void
window_render(Window *window);

// Required functions to be implemented in hale_platform_win32_<...>.cpp.

hale_internal b32  __app_init(App *app);
hale_internal void __app_release(App *app);

hale_internal b32  __window_init(Window *window);
hale_internal void __window_release(Window *window);
hale_internal void __window_resize(Window *window, RECT *rc, LRESULT *lresult);
hale_internal void __window_paint(Window *window);

}

#if defined(HALE_PLATFORM_WIN32_GDI)
#include "hale_platform_win32_gdi.cpp"
#elif defined(HALE_PLATFORM_WIN32_DX)
#include "hale_platform_win32_dx.cpp"
#endif

namespace hale {

// *********************************************************************************
//
//    Window
//
// *********************************************************************************

hale_global
b32 running;

hale_internal LRESULT CALLBACK
window_proc(HWND handle,
                  UINT message,
                  WPARAM wparam,
                  LPARAM lparam)
{
    Window *window = (Window*)GetPropA(handle, "hale_window");

    switch (message)
    {
    case WM_CLOSE: {
        running = false;
    } break;

    case WM_SIZING: {
        if (window) {
//            LRESULT result;
//            RECT *rc = (RECT *)lparam;
//            __window_resize(window, rc, &result);
        }
    } break;

    case WM_PAINT: {
        if (window) {
            __window_paint(window);
            return 0;
        }
    } break;

    }

    return DefWindowProc(handle, message, wparam, lparam);
}

hale_internal b32
window_init(App *app, Window *window, WNDCLASSA *window_class)
{
    *window = {};

    window->app = app;
    window->handle = CreateWindowExA(0,
                                     window_class->lpszClassName,
                                     "Hale",
                                     WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                     20, 20, 1280, 720,
                                     0,
                                     0,
                                     window_class->hInstance,
                                     0
                                     );

    if (window->handle == 0)
    {
        win32_print_error("CreateWindowExA");
        hale_panic("CreateWindowExA");
        return 0;
    }

    __window_init(window);

    // TODO: DWM

    SetPropA(window->handle, "hale_window", (HANDLE)window);

    return 1;
}

hale_internal void
window_render(Window *window)
{
    Font *font = make_font(&window->app->context, 30.0f, Font::Weight::Normal, Font::Style::Normal);
    Rect<r32> rect;
    rect.min = {50, 50};
    rect.max = {200, 80};
    Pixel32 color;
    color.r = 0xFF;
    color.g = 0x80;
    color.b = 0x40;
    color.a = 0xFF;

    draw_text(window, rect, color, font, (ch16*)L"This is it guys <3");
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

    App app = {};
    app.instance = instance;

    __app_init(&app);

    //
    // Main window class.
    //

    WNDCLASSA window_class;
    ZeroMemory(&window_class, sizeof(window_class));
    window_class.style = CS_OWNDC ; //CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = app.instance;
    window_class.hCursor = LoadCursor(0, IDC_ARROW);
    window_class.lpszClassName = "HaleWindow";

    hale_assert(RegisterClassA(&window_class) != 0);

    hale_assert(window_init(&app, &app.window[0], &window_class));

    // TODO: Initialize application.
    // draw_rectangle(app.window[0], 0, 0, 50, 50, 0xFF0030);

    ShowWindow(app.window[0].handle, SW_SHOW);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (!running) {
            break;
        }
    }

    __app_release(&app);

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
