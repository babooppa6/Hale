#include "hale_ui.h"
#include "hale_document.h"

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "User32.lib")

#include "D2d1.h"
#pragma comment(lib, "D2d1.lib")

#include "Dwrite.h"
#pragma comment(lib, "Dwrite.lib")

// Example usage:
// https://code.google.com/p/gdipp/
    // https://code.google.com/p/gdipp/source/browse/gdimm/wic_text.cpp?r=b42f70c71d7c05c8653837a842b4100babc8906c&spec=svnb42f70c71d7c05c8653837a842b4100babc8906c

namespace hale {

//
//
//

inline D2D1::ColorF
to_dcolor(Color32 color)
{
    return D2D1::ColorF(RGB(color.b, color.g, color.r),
                        (FLOAT)color.a / 255.0f);
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

// ----------------------------------------------------------------
//
// App
//
// ----------------------------------------------------------------

hale_internal b32
__os_app_init(App *app)
{
    HRESULT hr;

    hr = ::CoInitialize(NULL);
    if (FAILED(hr)) {
        hale_panic("CoInitialize");
        return false;
    }

    HDC screen_dc = ::GetDC(0);
    app->impl.dpi_scale_x = ::GetDeviceCaps(screen_dc, LOGPIXELSX) / 96.0f;
    app->impl.dpi_scale_y = ::GetDeviceCaps(screen_dc, LOGPIXELSY) / 96.0f;
    ::ReleaseDC(0, screen_dc);

    //
    //
    //

    hr = D2D1CreateFactory(
                D2D1_FACTORY_TYPE_SINGLE_THREADED,
                &app->impl.d_factory
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
                reinterpret_cast<IUnknown**>(&app->impl.w_factory)
                );

    if (FAILED(hr)) {
        hale_error("DWriteCreateFactory");
        return 0;
    }

    //
    //
    //

    return app_init(app);
}

hale_internal void
__os_app_release(App *app)
{
    __safe_release(&app->impl.d_factory);
    __safe_release(&app->impl.w_factory);
}

// ----------------------------------------------------------------
//
// Window
//
// ----------------------------------------------------------------

hale_internal HRESULT
__window_create_resources(Window *window)
{
    if (window->impl.d_render_target) {
        return S_OK;
    }

    // TODO: <d7samurai> a tip: on windows 8 and up, you can use DXGI_SCALING_NONE for
    // your rendertarget. what i do then is to make the rendertarget the size of
    // the full screen and just change the viewport when rendering. that way you
    // don't have to resize the rendertarget (an expensive operation) and recreate
    // the buffers every time the window is changed.
    // on win7 you have to use DXGI_SCALING_STRETCH, so there you must resize the
    // whole thing at every change
    // (in the DXGI_SWAP_CHAIN_DESC1)

    HRESULT hr;

    RECT rc;
    GetClientRect(window->impl.handle, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

    hr = window->app->impl.d_factory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(
                    window->impl.handle,
                    size
                    ),
                &window->impl.d_render_target
                );

    if (FAILED(hr)) {
        hale_error("CreateHwndRenderTarget");
        return hr;
    }

    //
    // Rendering params.
    //

    ScopedCOM<IDWriteRenderingParams> rendering_params;
    // TODO: Base this on default parameters.
    //       https://msdn.microsoft.com/en-us/library/windows/desktop/dd368201(v=vs.85).aspx
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dd368190(v=vs.85).aspx
    hr = window->app->impl.w_factory->CreateCustomRenderingParams(
                // Average gamma
                // (gamma.red + gamma.green + gamma.blue) / 3
                1.0f,
                // enhancedContrast
                0.0f,
                // clearTypeLevel
                1.0f,
                // pixelGeometry
                DWRITE_PIXEL_GEOMETRY_RGB,
                // renderingMode (hinting)
                DWRITE_RENDERING_MODE_CLEARTYPE_NATURAL_SYMMETRIC,
                // renderingParams (out)
                &rendering_params.ptr
                );

    window->impl.d_render_target->SetTextRenderingParams(rendering_params.ptr);
    // TODO: render_target->SetTextAntialiasMode(text_aa_mode);
    //

    //
    // Default shared solid brush.
    //

    hr = window->impl.d_render_target->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::White),
        &window->impl.d_brush
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
    for (memi i = 0; i != window->impl.text_formats_count; ++i)
    {
        __safe_release(&window->impl.text_formats[i]._format);
        __safe_release(&window->impl.text_formats[i]._brush);
    }
    __safe_release(&window->impl.d_render_target);
}

hale_internal b32
__os_window_init(Window *window)
{
    window->text_processor.impl.window = window;
    __window_create_resources(window);
    return 1;
}

hale_internal void
__os_window_release(Window *window)
{
    __window_release_resources(window);
}

hale_internal void
__os_window_resize(Window *window, RECT *rc_client)
{
    if (window->impl.d_render_target)
    {
        D2D1_SIZE_U s;
        s.width = rc_client->right;
        s.height = rc_client->bottom;
        // TODO(cohen): Can this fail?
        window->impl.d_render_target->Resize(s);
        window->client_rect.max_x = rc_client->right;
        window->client_rect.max_y = rc_client->bottom;
        window_layout(window);
        __os_window_paint(window);
    }
}

hale_internal void
__os_window_paint(Window *window)
{
    HRESULT hr;

    // Recreates the device-dependent resources
    // if the d_render_target is NULL.
    hr = __window_create_resources(window);

    if (FAILED(hr)) {
        hale_error(__window_create_resources);
    }

//    RECT _r;
//    GetClientRect(window->impl.handle, &_r);
//    Rect<r32> r = {_r.left, _r.top, _r.right, _r.bottom};

    ID2D1HwndRenderTarget* rt = window->impl.d_render_target;
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

    ValidateRect(window->impl.handle, NULL);
}

// ----------------------------------------------------------------
//
// TextFormat
//
// ----------------------------------------------------------------

hale_internal void
__create_text_format(TextFormat *format)
{

}

TextFormat *
text_format(TextProcessor *text_processor,
            r32 size,
            TextFormat::Weight weight,
            TextFormat::Style style,
            Color32 color)
{
    Window *window = text_processor->impl.window;
    __Window *impl_window = &window->impl;

    TextFormat *format = NULL;
    TextFormat *f = NULL;
    memi i;
    for (i = 0; i < impl_window->text_formats_count; ++i)
    {
        f = &impl_window->text_formats[i];
        // TODO: Comparing floats with epsilons.
        if (equal(f->size, size, 0.001f) &&
            f->weight == weight &&
            f->style == style &&
            f->color == color)
        {
            if (f->_format) {
                return f;
            } else {
                format = f;
            }
        }
    }

    // Not found, create one.

    if (format == NULL || format->_format == NULL)
    {
        if (impl_window->text_formats_count == hale_array_count(impl_window->text_formats)) {
            hale_error("Too many formats.");
            return 0;
        }
        format = &impl_window->text_formats[impl_window->text_formats_count];
        *format = {};

        format->weight = weight;
        format->style = style;
        format->size = size;
        format->color = color;
    }

    // DWRITE_FONT_WEIGHT font_weight = (DWRITE_FONT_WEIGHT)format->weight;
    DWRITE_FONT_WEIGHT font_weight = (DWRITE_FONT_WEIGHT)format->weight;
    DWRITE_FONT_STRETCH font_stretch = DWRITE_FONT_STRETCH_NORMAL;
    DWRITE_FONT_STYLE font_style = (DWRITE_FONT_STYLE)format->style;


   HRESULT hr = window->app->impl.w_factory->CreateTextFormat(
        (WCHAR*)window->app->options.text_font_family,
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

    format->_brush = NULL;

    hr = impl_window->d_render_target->CreateSolidColorBrush(
        to_dcolor(format->color),
        &format->_brush
        );

    if (FAILED(hr)) {
        hale_error("CreateSolidColorBrush");
        __safe_release(&format->_format);
        return 0;
    }

    impl_window->text_formats_count++;

    return format;
}

// ----------------------------------------------------------------
//
// TextLayout
//
// ----------------------------------------------------------------

b32
text_layout(TextProcessor *text_processor,
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

    // TODO: Use GDI if DW measuring mode is not DWRITE_MEASURING_MODE_NATURAL
//    hr = _dw_factory->CreateGdiCompatibleTextLayout(lpString,
//            c,
//            dw_text_format,
//            (FLOAT) _dc_bmp_header.biWidth,
//            0,
//            _pixels_per_dip,
//            NULL,
//            _use_gdi_natural,
//            dw_text_layout);

    // GOOD LORD
    hr = text_processor->impl.window->app->impl.w_factory->CreateTextLayout(
                (const WCHAR*)text,
                (UINT32)text_length,
                base_style->_format,
                (FLOAT)width,
                (FLOAT)height,
                &layout->w_layout);

    hale_assert(layout->w_layout);

    if (FAILED(hr)) {
        return 0;
    }

    DWRITE_TEXT_METRICS metrics;
    layout->w_layout->GetMetrics(&metrics);
    layout->height = metrics.height;

    return 1;
}

void
text_layout_release(TextLayout *layout)
{
    __safe_release(&layout->w_layout);
}

void
text_layout_update(TextProcessor *text_processor,
                   TextLayout *layout,
                   ch16 *text,
                   memi text_length,
                   TextFormat *format_base,
                   TextFormatRange *format_ranges,
                   memi format_ranges_count,
                   r32 max_width)
{
    if (text || layout->w_layout == NULL)
    {
        text_layout(text_processor,
                    layout,
                    text, text_length,
                    // TODO: max_height
                    max_width, 9999,
                    format_base
                    );
    }
    else
    {
        layout->w_layout->SetMaxWidth(max_width);

        DWRITE_TEXT_METRICS metrics;
        layout->w_layout->GetMetrics(&metrics);
        layout->height = metrics.height;
    }



    HRESULT hr;
    DWRITE_TEXT_RANGE r;
    IDWriteTextLayout *wl = layout->w_layout;
    TextFormatRange *s;
    for (memi i = 0; i != format_ranges_count; i++)
    {
        s = &format_ranges[i];

        // TODO: Isn't there a better way to do this?
        //     - There is, my friend, just implement your own
        //       TextRenderer or TextLayout.
        r.startPosition = s->begin;
        r.length = s->end - s->begin;
        hr = wl->SetFontWeight((DWRITE_FONT_WEIGHT)s->format->weight, r);
        hr = wl->SetFontStyle((DWRITE_FONT_STYLE)s->format->style, r);
        hr = wl->SetFontSize((FLOAT)s->format->size, r);
        layout->w_layout->SetDrawingEffect(s->format->_brush, r);
    }
}

void
text_layout_clear_formats(Window *window,
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

// ----------------------------------------------------------------
//
// DocumentLayout
//
// ----------------------------------------------------------------

DocumentLayout *
document_layout(TextProcessor *text_processor, DocumentSession *session)
{
    Window *window = text_processor->impl.window;
    DocumentLayout *layout = &window->document_layouts[window->document_layouts_count++];
    *layout = {};

    layout->text_processor = text_processor;
    layout->session = session;
    layout->scroll_block_i = 0;
    layout->scroll_block_y = 0;

    memi block_count = vector_count(session->document->blocks);
    vector_init(&layout->impl.blocks, block_count);
    vector_resize(&layout->impl.blocks, block_count);

    return layout;
}

void
document_layout_scroll_to(DocumentLayout *layout, r32 dx, r32 dy)
{

}

void
document_layout_set_viewport(DocumentLayout *layout, Rect<r32> viewport)
{

}

void
document_layout_invalidate(DocumentLayout *layout, memi from, memi to)
{

}

hale_internal __DocumentLayout::Block*
get_block(DocumentLayout *layout, memi block_index)
{
    __DocumentLayout *impl = &layout->impl;
    if (vector_count(impl->blocks) == 0) {
        return NULL;
    }

    DocumentSession *session = layout->session;
    Document *document = session->document;

    Memory<ch16, MallocAllocator> text = {};
    TextFormatRange *formats = NULL;
    memi formats_count = 0;

    auto block = &impl->blocks[block_index];

    // Text

    if (((block->flags & __DocumentLayout::TextValid) == 0))
    {
        // TODO: Optimize this with TextBuffer.
        // TODO: If we would keep only a shorter cache of the blocks, we can optimize there.

        memi count = document_block_length_without_le(document, block_index);
        memi begin = document_block_begin(document, block_index);
        memory_reallocate(&text, count);
        text.count = count;
        document_text(document, begin, count, text.e, count);

        block->flags |= __DocumentLayout::TextValid;
        block->flags &= ~__DocumentLayout::FormatValid;
    }

    // Formats

    if ((block->flags & __DocumentLayout::FormatValid) == 0)
    {
        block->flags |= __DocumentLayout::FormatValid;
        // document_formats(session->document, block_index);
        // TODO: Update the formats.
    }

    r32 max_width = layout->viewport.max_x - layout->viewport.min_x;
    if (block->layout.w_layout == NULL ||
            text.e ||
            formats ||
            !equal(block->layout.w_layout->GetMaxWidth(), max_width, 0.1f)
            )
    {
        if (block->layout.w_layout == NULL) {
            hale_assert(text.e);
        }
        text_layout_update(layout->text_processor,
                           &block->layout,
                           text.e, text.count,
                           layout->base_format,
                           formats, formats_count,
                           max_width);
    }

    memory_deallocate(&text);

    // TODO: Invalidate session's cursor's vertical anchors if they are on this block.

    return block;
}

void
document_layout_layout(DocumentLayout *layout)
{
    memi i = layout->scroll_block_i;
    r32 y = 0;

    __DocumentLayout::Block *block;
    do {
        block = get_block(layout, i);
        block->y = y;
        y += block->layout.height;
        i++;
    } while ((block->y + block->layout.height) < layout->viewport.max_y);
}

void
document_layout_draw(Window *window, DocumentLayout *layout, r32 x, r32 y)
{
    document_layout_layout(layout);

    memi i = layout->scroll_block_i;
    // r32 y = layout->viewport.min_y + layout->scroll_block_y;
    __DocumentLayout::Block *block;
    block = get_block(layout, i);
    while (block && (block->y + block->layout.height) < layout->viewport.max_y)
    {
        // TODO: Draw selections.
        draw_text(window, x, block->y, &block->layout);
        // TODO: Draw cursors and selections.
        block = get_block(layout, ++i);
    }
}

// ----------------------------------------------------------------
//
// Draw
//
// ----------------------------------------------------------------

void
draw_text(Window *window,
          r32 x, r32 y,
          TextLayout *layout)
{
    // TODO: Use IDWriteTextLayout::Draw?
    __Window *impl = &window->impl;
    D2D1_DRAW_TEXT_OPTIONS options = D2D1_DRAW_TEXT_OPTIONS_CLIP;
    impl->d_render_target->DrawTextLayout({(FLOAT)x, (FLOAT)y},
                                          layout->w_layout,
                                          layout->base->_brush,
                                          options);
}

void
draw_text(Window *window,
          r32 x, r32 y, r32 width, r32 height,
          TextFormat *format,
          const ch16 *text,
          memi text_length)
{
    hale_assert(format);
    hale_assert(format->_format);

    auto brush = window->impl.d_brush;
    brush->SetColor(to_dcolor(format->color));

    window->impl.d_render_target->DrawText(
                (WCHAR*)text,
                text_length,
                format->_format,
                D2D1::RectF(x, y, x+width, y+height),
                brush,
                D2D1_DRAW_TEXT_OPTIONS_NONE,
                DWRITE_MEASURING_MODE_GDI_CLASSIC
                );
}

//
//
//

void
draw_rectangle(Window *window,
               Rect<r32> rect,
               Color32 color)
{
    __Window *impl = &window->impl;
    impl->d_brush->SetColor(to_dcolor(color));
    impl->d_render_target->FillRectangle(to_drect(rect),
                                         impl->d_brush);
}

} // namespace hale
