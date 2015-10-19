#include "hale.h"
#include "hale_platform_win32_ui.h"

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#pragma comment(lib, "Gdi32.lib")

namespace hale {

hale_internal void
window_render(WindowImpl *window);

// Required functions to be implemented in hale_platform_win32_<...>.cpp.

hale_internal b32  __app_init(AppImpl *app);
hale_internal void __app_release(AppImpl *app);

hale_internal b32  __window_init(WindowImpl *window);
hale_internal void __window_release(WindowImpl *window);
hale_internal void __window_resize(WindowImpl *window, RECT *rc, LRESULT *lresult);
hale_internal void __window_paint(WindowImpl *window);

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
    WindowImpl *window = (WindowImpl*)GetPropA(handle, "hale_window");

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
window_init(AppImpl *app, WindowImpl *window, WNDCLASSA *window_class)
{
    *window = {};

    window->app = app;
    window->handle = CreateWindowExA(0,
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
window_render(WindowImpl *window)
{
    static int i = 0;
    qDebug() << i++;

//    TextFormat *format = text_format(window,
//                                     30.0f,
//                                     TextFormat::Weight::Normal,
//                                     TextFormat::Style::Normal,
//                                     {0xFF, 0x80, 0x40, 0xFF});

//    ch16 *t = (ch16*)L"This is it guys <3";
//    draw_text(window, 50, 50, 200, 80, format, t, string_length(t));


    TextFormat *formats[3];

    formats[0] = text_format(window, 30.0f, TextFormat::Weight::Normal, TextFormat::Style::Normal, {0xFF, 0x80, 0x40, 0xFF});
    formats[1] = text_format(window, 20.0f, TextFormat::Weight::Bold, TextFormat::Style::Normal, {0x80, 0xFF, 0x40, 0xFF});
    formats[2] = text_format(window, 12.0f, TextFormat::Weight::Bold, TextFormat::Style::Normal, {0x00, 0x00, 0x00, 0xFF});

    TextFormatRange ranges[2];
    ranges[0].begin = 0;
    ranges[0].end = 4;
    ranges[0].format = formats[1];

    ranges[1].begin = 5;
    ranges[1].end = 7;
    ranges[1].format = formats[2];

    ch16 *t = (ch16*)L"This is it guys <3";

    TextLayout layout = {};
    text_layout(window, &layout, t, string_length(t), 300, 200, formats[0]);
    text_layout_set_formats(window, &layout, ranges, hale_array_count(ranges));

    draw_text(window, 50, 50, &layout);
}

// *********************************************************************************
//
//   Main
//
// *********************************************************************************

AppImpl g_app;

int
main(HINSTANCE instance, int argc, char *argv[])
{
    // TODO: Command line args.
    hale_unused(argc);
    hale_unused(argv);

    running = 1;

    // App app = {};
    g_app.instance = instance;

    __app_init(&g_app);

    //
    // Main window class.
    //

    WNDCLASSA window_class;
    ZeroMemory(&window_class, sizeof(window_class));
    window_class.style = CS_OWNDC ; //CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = g_app.instance;
    window_class.hCursor = LoadCursor(0, IDC_ARROW);
    window_class.lpszClassName = "HaleWindow";

    hale_assert(RegisterClassA(&window_class) != 0);

    hale_assert(window_init(&g_app, &g_app.window[0], &window_class));

    // TODO: Initialize application.

    ShowWindow(g_app.window[0].handle, SW_SHOW);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (!running) {
            break;
        }
    }

    __app_release(&g_app);

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
