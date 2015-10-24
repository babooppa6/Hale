#ifndef HALE_PLATFORM_WIN32_UI_H
#define HALE_PLATFORM_WIN32_UI_H

#include "hale.h"

#if defined(HALE_PLATFORM_WIN32_GDI)
#include "hale_platform_win32_gdi.h"
#elif defined(HALE_PLATFORM_WIN32_DX)
#include "hale_os_win32_dx.h"
#endif

namespace hale {

struct __App;

struct __Window
{
    HWND handle;
    s32 wheel_delta;

    __WINDOW_IMPL
};

struct __App
{
    HINSTANCE instance;
    r64 animations_time;
    memi animations_count;

    __APP_IMPL;
};

} // namespace hale

#endif // HALE_PLATFORM_WIN32_UI_H

