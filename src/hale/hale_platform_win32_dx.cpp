#include "hale.h"
#include "hale_platform_win32_ui.h"
#include "hale_platform_win32_dx.h"

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "User32.lib")

#include "D2d1.h"
#pragma comment(lib, "D2d1.lib")

#include "Dwrite.h"
#pragma comment(lib, "Dwrite.lib")

namespace hale {

inline D2D1::ColorF
to_dcolor(Pixel32 color)
{
    UINT32 rgb = *((UINT32*)color.rgb.E);
    FLOAT a = (FLOAT)color.a / 255.0f;
    return D2D1::ColorF(rgb, a);
}

inline D2D1_RECT_F
to_drect(const Rect<r32> &rect)
{
    return D2D1::RectF(rect.min_x,
                       rect.min_y,
                       rect.max_x,
                       rect.max_y);
}




template <class T> inline void __safe_release(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = nullptr;
    }
}

template<class T>
struct ScopedCOM
{
    ScopedCOM() : ptr(nullptr) {}
    ScopedCOM(T *ptr) : ptr(ptr) {}
    ~ScopedCOM() {
        __safe_release(&ptr);
    }

    T *operator->() {
        return ptr;
    }

    T *take() {
        T *ret = ptr;
        ptr = nullptr;
        return ret;
    }

    T *ptr;
};

//
//
//

hale_internal b32
__app_init(App *app)
{
    auto context = &app->context;

    HRESULT hr;

    hr = ::CoInitialize(NULL);
    if (FAILED(hr)) {
        hale_panic("CoInitialize");
        return false;
    }

    app->context.font_family = L"Fira Mono";

    HDC screen_dc = ::GetDC(0);
    context->dpi_scale_x = ::GetDeviceCaps(screen_dc, LOGPIXELSX) / 96.0f;
    context->dpi_scale_y = ::GetDeviceCaps(screen_dc, LOGPIXELSY) / 96.0f;
    ::ReleaseDC(0, screen_dc);

    //
    //
    //

    hr = D2D1CreateFactory(
                D2D1_FACTORY_TYPE_SINGLE_THREADED,
                &context->d_factory
                );

    if (FAILED(hr)) {
        hale_error("D2D1CreateFactory");
        return 0;
    }

    //
    //
    //

    hr = DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(&context->w_factory)
                );

    if (FAILED(hr)) {
        hale_error("DWriteCreateFactory");
        return 0;
    }

    //
    //
    //

    return 1;
}

hale_internal void
__app_release(App *app)
{
    auto context = &app->context;
    for (memi i = 0; i != context->cache.count; ++i)
    {
        __safe_release(&context->cache.fonts[i].d_format);
    }
    __safe_release(&context->d_factory);
    __safe_release(&context->w_factory);
}

// ---
//
// Window
//
// ---

hale_internal HRESULT
__window_create_resources(Window *window)
{
    if (window->context.d_render_target) {
        return S_OK;
    }

    HRESULT hr;

    RECT rc;
    GetClientRect(window->handle, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

    hr = window->app->context.d_factory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(
                    window->handle,
                    size
                    ),
                &window->context.d_render_target
                );

    if (FAILED(hr)) {
        hale_error("CreateHwndRenderTarget");
        return hr;
    }

    //
    // Default shared solid brush.
    //

    hr = window->context.d_render_target->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::White),
        &window->context.d_brush
        );

    if (FAILED(hr)) {
        hale_error("CreateSolidColorBrush");
        return hr;
    }

    return hr;
}

hale_internal void
__window_release_resources(Window *window)
{
    __safe_release(&window->context.d_render_target);
}

hale_internal b32
__window_init(Window *window)
{
    __window_create_resources(window);
    return 1;
}

hale_internal void
__window_release(Window *window)
{
    __window_release_resources(window);
}

hale_internal void
__window_resize(Window *window, V2u size)
{
    if (window->context.d_render_target)
    {
        D2D1_SIZE_U s;
        s.width = size.x;
        s.height = size.y;
        // TODO(cohen): Can this fail?
        window->context.d_render_target->Resize(s);
    }
}

hale_internal void
__window_paint(Window *window)
{
    HRESULT hr;

    // Recreates the device-dependent resources
    // if the d_render_target is NULL.
    hr = __window_create_resources(window);

    if (FAILED(hr)) {
        hale_error(__window_create_resources);
    }


    ID2D1HwndRenderTarget* rt = window->context.d_render_target;
    rt->BeginDraw();

    rt->SetTransform(D2D1::IdentityMatrix());

    // TODO: Global background color configuration.
    rt->Clear(D2D1::ColorF(D2D1::ColorF::White));

    window_render(window);

    hr = rt->EndDraw();

    // NOTE(cohen): Has to be called if anything above fails!
    if (FAILED(hr)) {
        print("releasing render target");
        __window_release_resources(window);
    }
}

//
//
//

Font *
make_font(AppContext *context, r32 size, Font::Weight weight, Font::Style style)
{
    memi i;
    for (i = 0; i < context->cache.count; ++i)
    {
        auto &font = context->cache.fonts[i];
        // TODO: Comparing floats with epsilons.
        if (fabs(font.size - size) <= 0.001f &&
            font.weight == weight &&
            font.style == style)
        {
            return &font;
        }
    }

    // Not found, create one.

    if (context->cache.count == HALE_PLATFORM_WIN32_DX_MAX_FORMATS) {
        hale_error("Too many formats.");
        return 0;
    }

    i = context->cache.count;

    auto &font = context->cache.fonts[i];
    font = {};
    font.weight = weight;
    font.style = style;
    font.size = size;

    DWRITE_FONT_WEIGHT font_weight = (DWRITE_FONT_WEIGHT)font.weight;
    DWRITE_FONT_STRETCH font_stretch = DWRITE_FONT_STRETCH_NORMAL;
    DWRITE_FONT_STYLE font_style = (DWRITE_FONT_STYLE)font.style;

    HRESULT hr = context->w_factory->CreateTextFormat(
        context->font_family,
        // System font collection.
        NULL,
        font_weight,
        font_style,
        font_stretch,
        font.size,
        L"en-us",
        &font.d_format
        );

    if (FAILED(hr)) {
        win32_print_error("CreateTextFormat");
        return 0;
    }

    context->cache.count++;

    return &font;
}

void
draw_text(Window *window, Rect<r32> rect, Pixel32 color, Font *font, const ch16 *text, memi text_length)
{
    hale_assert(font);
    hale_assert(font->d_format);

    auto brush = window->context.d_brush;

    brush->SetColor(to_dcolor(color));
    window->context.d_render_target->DrawText(
                (WCHAR*)text,
                text_length,
                font->d_format,
                to_drect(rect),
                brush,
                D2D1_DRAW_TEXT_OPTIONS_NONE,
                DWRITE_MEASURING_MODE_GDI_CLASSIC
                );
}

} // namespace hale
