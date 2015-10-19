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

//
//
//

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
__app_init(AppImpl *app)
{
    HRESULT hr;

    hr = ::CoInitialize(NULL);
    if (FAILED(hr)) {
        hale_panic("CoInitialize");
        return false;
    }

    app->font_family = L"Fira Mono";

    HDC screen_dc = ::GetDC(0);
    app->dpi_scale_x = ::GetDeviceCaps(screen_dc, LOGPIXELSX) / 96.0f;
    app->dpi_scale_y = ::GetDeviceCaps(screen_dc, LOGPIXELSY) / 96.0f;
    ::ReleaseDC(0, screen_dc);

    //
    //
    //

    hr = D2D1CreateFactory(
                D2D1_FACTORY_TYPE_SINGLE_THREADED,
                &app->d_factory
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
                reinterpret_cast<IUnknown**>(&app->w_factory)
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
__app_release(AppImpl *app)
{
    __safe_release(&app->d_factory);
    __safe_release(&app->w_factory);
}

// ---
//
// Window
//
// ---

hale_internal HRESULT
__window_create_resources(WindowImpl *window)
{
    if (window->d_render_target) {
        return S_OK;
    }

    HRESULT hr;

    RECT rc;
    GetClientRect(window->handle, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

    hr = window->app->d_factory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(
                    window->handle,
                    size
                    ),
                &window->d_render_target
                );

    if (FAILED(hr)) {
        hale_error("CreateHwndRenderTarget");
        return hr;
    }

    //
    // Default shared solid brush.
    //

    hr = window->d_render_target->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::White),
        &window->d_brush
        );

    if (FAILED(hr)) {
        hale_error("CreateSolidColorBrush");
        return hr;
    }

    return hr;
}

hale_internal void
__window_release_resources(WindowImpl *window)
{
    for (memi i = 0; i != window->text_formats_count; ++i)
    {
        __safe_release(&window->text_formats[i]._format);
        __safe_release(&window->text_formats[i]._brush);
    }
    __safe_release(&window->d_render_target);
}

hale_internal b32
__window_init(WindowImpl *window)
{
    __window_create_resources(window);
    return 1;
}

hale_internal void
__window_release(WindowImpl *window)
{
    __window_release_resources(window);
}

hale_internal void
__window_resize(WindowImpl *window, V2u size)
{
    if (window->d_render_target)
    {
        D2D1_SIZE_U s;
        s.width = size.x;
        s.height = size.y;
        // TODO(cohen): Can this fail?
        window->d_render_target->Resize(s);
    }
}

hale_internal void
__window_paint(WindowImpl *window)
{
    HRESULT hr;

    // Recreates the device-dependent resources
    // if the d_render_target is NULL.
    hr = __window_create_resources(window);

    if (FAILED(hr)) {
        hale_error(__window_create_resources);
    }


    ID2D1HwndRenderTarget* rt = window->d_render_target;
    rt->BeginDraw();
    rt->SetTransform(D2D1::IdentityMatrix());
    rt->Clear(D2D1::ColorF(D2D1::ColorF::White));
    window_render(window);
    hr = rt->EndDraw();

    // NOTE(cohen): Has to be called if anything above fails!
    if (FAILED(hr)) {
        print("releasing render target");
        __window_release_resources(window);
    }

    ValidateRect(window->handle, NULL);
}

//
//
//

TextFormat *
text_format(WindowImpl *window,
            r32 size,
            TextFormat::Weight weight,
            TextFormat::Style style,
            Pixel32 color)
{
    TextFormat *format = NULL;
    TextFormat *f = NULL;
    memi i;
    for (i = 0; i < window->text_formats_count; ++i)
    {
        f = &window->text_formats[i];
        // TODO: Comparing floats with epsilons.
        if (fabs(f->size - size) <= 0.001f &&
            f->weight == weight &&
            f->style == style &&
            f->color == color)
        {
            if (f->_format) {
                return format;
            } else {
                format = f;
            }
        }
    }

    // Not found, create one.

    if (format == NULL || format->_format == NULL)
    {
        if (window->text_formats_count == HALE_PLATFORM_WIN32_DX_MAX_FORMATS) {
            hale_error("Too many formats.");
            return 0;
        }
        format = &window->text_formats[window->text_formats_count];
        *format = {};

        format->weight = weight;
        format->style = style;
        format->size = size;
        format->color = color;
    }

    DWRITE_FONT_WEIGHT font_weight = (DWRITE_FONT_WEIGHT)format->weight;
    DWRITE_FONT_STRETCH font_stretch = DWRITE_FONT_STRETCH_NORMAL;
    DWRITE_FONT_STYLE font_style = (DWRITE_FONT_STYLE)format->style;

    HRESULT hr = window->app->w_factory->CreateTextFormat(
        window->app->font_family,
        // System font collection.
        NULL,
        font_weight,
        font_style,
        font_stretch,
        format->size,
        L"en-us",
        &format->_format
        );

    if (FAILED(hr)) {
        hale_error("CreateTextFormat");
        return 0;
    }

    hr = window->d_render_target->CreateSolidColorBrush(
        to_dcolor(format->color),
        &format->_brush
        );

    if (FAILED(hr)) {
        hale_error("CreateSolidColorBrush");
        __safe_release(&format->_format);
        return 0;
    }

    window->text_formats_count++;

    return format;
}

b32
text_layout(WindowImpl *window,
            TextLayout *layout,
            const ch16 *text, memi text_length,
            r32 width,
            r32 height,
            TextFormat *base_style)
{
    hale_assert(text_length <= 0xFFFFffff);
    HRESULT hr;

    *layout = {};
    layout->base = base_style;
    hr = window->app->w_factory->CreateTextLayout(
                (const WCHAR*)text,
                (UINT32)text_length,
                base_style->_format,
                (FLOAT)width,
                (FLOAT)height,
                &layout->w_layout);

    if (FAILED(hr)) {
        return 0;
    }

    return 1;
}

void
text_layout_set_formats(WindowImpl *window,
                        TextLayout *layout,
                        TextFormatRange *styles,
                        memi styles_count)
{
    HRESULT hr;
    DWRITE_TEXT_RANGE r;
    IDWriteTextLayout *wl = layout->w_layout;
    TextFormatRange *s;
    for (memi i = 0; i != styles_count; i++)
    {
        s = &styles[i];

        // TODO: Isn't there a better way to do this?
        r.startPosition = s->begin;
        r.length = s->end - s->begin;
        hr = wl->SetFontWeight((DWRITE_FONT_WEIGHT)s->format->weight, r);
        hale_assert(SUCCEEDED(hr));
        hr = wl->SetFontStyle((DWRITE_FONT_STYLE)s->format->style, r);
        hale_assert(SUCCEEDED(hr));
        hr = wl->SetFontSize((FLOAT)s->format->size, r);
        hale_assert(SUCCEEDED(hr));
        layout->w_layout->SetDrawingEffect(s->format->_brush, r);
    }
}

void
text_layout_clear_formats(WindowImpl *window,
                          TextLayout *layout)
{
    DWRITE_TEXT_RANGE r;
    r.startPosition = 0;
    r.length = -1;
    layout->w_layout->SetFontWeight((DWRITE_FONT_WEIGHT)layout->base->weight, r);
    layout->w_layout->SetFontStyle((DWRITE_FONT_STYLE)layout->base->style, r);
    layout->w_layout->SetFontSize(layout->base->size, r);
    layout->w_layout->SetDrawingEffect(layout->base->_brush, r);
}

void
draw_text(WindowImpl *window,
          r32 x, r32 y,
          TextLayout *layout)
{
    // TODO: Use IDWriteTextLayout::Draw?

    D2D1_DRAW_TEXT_OPTIONS options = D2D1_DRAW_TEXT_OPTIONS_CLIP;

    window->d_render_target->DrawTextLayout({x, y},
                                            layout->w_layout,
                                            layout->base->_brush,
                                            options);
}

//
//
//

void
draw_text(WindowImpl *window,
          r32 x, r32 y, r32 width, r32 height,
          TextFormat *format,
          const ch16 *text,
          memi text_length)
{
    hale_assert(format);
    hale_assert(format->_format);

    auto brush = window->d_brush;
    brush->SetColor(to_dcolor(format->color));

    window->d_render_target->DrawText(
                (WCHAR*)text,
                text_length,
                format->_format,
                D2D1::RectF(x, y, x+width, y+height),
                brush,
                D2D1_DRAW_TEXT_OPTIONS_NONE,
                DWRITE_MEASURING_MODE_GDI_CLASSIC
                );
}

} // namespace hale
