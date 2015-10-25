#include "hale_ui.h"
#include "hale_document.h"

#include "D2d1.h"
#pragma comment(lib, "D2d1.lib")

#include "Dwrite.h"
#pragma comment(lib, "Dwrite.lib")

// Example usage:
// https://code.google.com/p/gdipp/
    // https://code.google.com/p/gdipp/source/browse/gdimm/wic_text.cpp?r=b42f70c71d7c05c8653837a842b4100babc8906c&spec=svnb42f70c71d7c05c8653837a842b4100babc8906c

// Selections, rectangles, custom inline objects, custom renderers and format specifiers:
// Background rectangles
// - Draw using custom renderer.
// - Use DrawGlyphRun.
// - Pass the custom format specifier through client drawing effect pointer.
// - http://www.charlespetzold.com/blog/2014/01/Character-Formatting-Extensions-with-DirectWrite.html


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
__app_init(App *app)
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

    layout->w_layout->GetMetrics(&layout->w_metrics);

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
        if (layout->w_layout != NULL) {
            __safe_release(&layout->w_layout);
        }
        text_layout(text_processor,
                    layout,
                    text, text_length,
                    max_width, 9999, // std::numeric_limits<r32>::infinity(),
                    format_base
                    );
    }
    else
    {
        layout->w_layout->SetMaxWidth(max_width);
        layout->w_layout->GetMetrics(&layout->w_metrics);
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

hale_internal __DocumentLayout::Block*
_get_block(DocumentLayout *layout, memi block_index)
{
    __DocumentLayout *impl = &layout->impl;
    if (vector_count(impl->blocks) == 0) {
        return NULL;
    }

    DocumentView *session = layout->session;
    Document *document = session->document;

    Memory<ch16> text = {};
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
        text.reallocate(count);
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

    r32 max_width = layout->document_rect.max_x - layout->document_rect.min_x;
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

    if (text.e) {
        text.deallocate();
    }

    // TODO: Invalidate session's cursor's vertical anchors if they are on this block.

    return block;
}

hale_internal
inline r32
_get_block_height(__DocumentLayout::Block *block)
{
    return block->layout.w_metrics.height;
}

hale_internal
inline r32
_get_block_height_with_layout(DocumentLayout *layout, memi i)
{
    return _get_block(layout, i)->layout.w_metrics.height;
}

hale_internal memi
_find_block_for_y(DocumentLayout *layout, memi from_i, r32 from_y, r32 target_y, r32 *o_y)
{
    // TODO: Limit the distance, so if the block we're looking for it too far for limits of r32, it'll assert.
    memi i = from_i;
    r32 block_y = from_y;
    r32 block_h;

    auto blocks = &layout->impl.blocks;
    memi block_count = vector_count(blocks);

    // TODO: Remove `i` iterator, possibly replace with Block pointer iterator.
    if (target_y > 0) {
        for (;;)
        {
            block_h = _get_block_height_with_layout(layout, i);
            if (block_y + block_h >= target_y)
            {
                *o_y = - (target_y - block_y);
                return i;
            }
            block_y += block_h;
            i++;
            if (i >= block_count)
            {
                // TODO: Here we snap beyond the document!
                *o_y = block_y + block_h;
                return block_count - 1;
            }
        }
    }
    else
    {
        for (;;)
        {
            if (block_y <= target_y)
            {
                *o_y = - (target_y - block_y);
                return i;
            }

            if (i == 0)
            {
                *o_y = hale_minimum(layout->options.padding.top, - (target_y - block_y));
                return 0;
            }
            i--;

//            i--;
//            if (i < 0) {
//                *o_y = hale_minimum(m_dv->options.padding.top(), - (target_y - block_y));
//                return 0;
//            }
            block_h = _get_block_height_with_layout(layout, i);
            block_y -= block_h;
        }
    }
    *o_y = from_y; // first->y;
    return from_i; // first->i;
}

hale_internal void
_reset_blocks(DocumentLayout *layout, memi i, r32 y)
{
    memi block_count = vector_count(layout->impl.blocks);

    __DocumentLayout::Block *block;
    layout->scroll_block_first_i = i;
    layout->scroll_block_first_y = y;
    layout->scroll_block_last_i = i;

    do {
        block = _get_block(layout, i);
        block->y = y;
        // qDebug() << "Place" << i << block->y;
        y += _get_block_height(block);
        if (y > layout->document_rect.max_y) {
            break;
        }
        i++;
    } while (i != block_count);

    layout->scroll_block_last_i = i;
    // qDebug() << "First" << layout->scroll_block_first_i << layout->scroll_block_first_y << "Last" << i;
    layout->impl.layout_invalid = 0;
}

hale_internal
inline memi
_find_block_for_y(DocumentLayout *layout, r32 delta, r32 *o_y)
{
    memi i = layout->scroll_block_first_i;
    auto first = _get_block(layout, i);
    return _find_block_for_y(layout, i, first->y, delta, o_y);
}

//
// Public API
//

DocumentLayout *
document_layout(TextProcessor *text_processor, DocumentView *session)
{
    Window *window = text_processor->impl.window;
    DocumentLayout *layout = &window->document_layouts[window->document_layouts_count++];
    *layout = {};

    layout->text_processor = text_processor;
    layout->session = session;
    layout->scroll_block_first_i = 0;
    layout->scroll_block_first_y = 0;
    layout->impl.layout_invalid = 1;

    memi block_count = vector_count(session->document->blocks);
    vector_init(&layout->impl.blocks, block_count);
    __DocumentLayout::Block block = {};
    vector_insert(&layout->impl.blocks, 0, block_count, block);

    return layout;
}

void
document_layout_scroll_by(DocumentLayout *layout, r32 dx, r32 dy)
{
    hale_unused(dx);

    r32 y = 0.0;
    memi i = _find_block_for_y(layout, dy, &y);
    // qDebug() << __FUNCTION__ << i << y;
    _reset_blocks(layout, i, y);
}

void
document_layout_set_viewport(DocumentLayout *layout, Rect<r32> viewport)
{
    layout->gutter_rect = viewport;
    layout->gutter_rect.max_x = 50.0f;
    layout->document_rect = viewport;
    layout->document_rect.min_x = layout->gutter_rect.max_x;
    // TODO: Only invalidate the layout to be called lazily?
    layout->impl.layout_invalid = 1;
    // document_layout_layout(layout);
}

void
document_layout_on_insert(DocumentLayout *layout, DocumentEdit *edit)
{
    Vector<__DocumentLayout::Block> *blocks = &layout->impl.blocks;

    (*blocks)[edit->block_changed].flags &= ~(__DocumentLayout::TextValid | __DocumentLayout::FormatValid);
    if (edit->blocks_changed_count)
    {
        // qDebug() << (*blocks)[0].flags << (*blocks)[1].flags << (*blocks)[2].flags << (*blocks)[3].flags;
        __DocumentLayout::Block block = {};
        vector_insert(blocks,
                      edit->blocks_changed_at,
                      edit->blocks_changed_count,
                      block);
        // qDebug() << (*blocks)[0].flags << (*blocks)[1].flags << (*blocks)[2].flags << (*blocks)[3].flags;
    }

    layout->impl.layout_invalid = 1;
}

void
document_layout_layout(DocumentLayout *layout)
{
    if (!layout->impl.layout_invalid) {
        return;
    }

    _reset_blocks(layout,
                  layout->scroll_block_first_i,
                  layout->scroll_block_first_y);
}

hale_internal inline void
_draw_gutter(Window *window,
            DocumentLayout *layout,
            __DocumentLayout::Block *block,
            memi block_index,
            Vector<ch16> *buffer)
{
    to_string(buffer, block_index + 1);

    // TODO: Aling the line number to the baseline of the first block's line.

//    DWRITE_LINE_METRICS line_metrics[1];
//    UINT32 _unused;
//    HRESULT hr = block->layout.w_layout->GetLineMetrics(line_metrics, 1, &_unused);


//    DWRITE_HIT_TEST_METRICS metrics;
//    FLOAT px, py;
//    block->layout.w_layout->HitTestTextPosition(0,
//                                                FALSE,
//                                                &px, &py,
//                                                &metrics);

//    DWRITE_LINE_SPACING_METHOD method;
//    FLOAT spacing, baseline;
//    block->layout.w_layout->GetLineSpacing(&method, &spacing, &baseline);

//    block->layout.w_layout->SetLineSpacing(DWRITE_LINE_SPACING_METHOD_UNIFORM, )

    r32 text_y = block->y;
    r32 text_min_x = layout->gutter_rect.min_x + layout->gutter_options.padding.left;
    r32 text_max_x = layout->gutter_rect.max_x - layout->gutter_options.padding.right;
    r32 text_width = text_max_x - text_min_x;
    r32 text_height = _get_block_height(block);

    draw_text(window,
              text_min_x, text_y,
              text_width,
              text_height,
              layout->gutter_options.text_format,
              vector_begin(buffer),
              vector_count(buffer)
              );

    vector_clear(buffer);
}

void
document_layout_draw(Window *window, DocumentLayout *layout)
{
    document_layout_layout(layout);

    Vector<ch16> buffer;
    vector_init(&buffer, 10);

    memi i = layout->scroll_block_first_i;
    // r32 y = layout->viewport.min_y + layout->scroll_block_y;
    __DocumentLayout::Block *block;
    while (i != (layout->scroll_block_last_i+1))
    {
        // TODO: Draw selections.
        // TODO: Maybe not use block->y directly, but use the document_rect to shift it?
        block = _get_block(layout, i);
        // qDebug() << "Draw" << i << block->y;
        draw_text(window, layout->document_rect.min_x, block->y, &block->layout);
        _draw_gutter(window, layout, block, i, &buffer);
        // TODO: Draw cursors and selections.
        i++;
    }

    vector_release(&buffer);
}

void
document_layout_draw_cursor(Window *window, DocumentLayout *layout, DocumentPosition position)
{
    // TODO: Implement the blinking timer on the window (or even the app),
    // so it's shared accross the application.

    // TODO: Check if the cursor is still within the viewport (maybe do that in caller?)
    __DocumentLayout::Block *block = _get_block(layout, position.block);

    Rect<r32> rect;

    DWRITE_HIT_TEST_METRICS metrics;
    FLOAT px, py;
    HRESULT hr = block->layout.w_layout->HitTestTextPosition(position.position,
                                                             FALSE,
                                                             &px, &py,
                                                             &metrics);

    if (FAILED(hr)) {
        hale_error("HitTestTextPosition");
        return;
    }

    rect.min_x = layout->document_rect.min_x + metrics.left;
    rect.min_y = layout->document_rect.min_y + metrics.top;
    rect.max_x = layout->document_rect.min_x + metrics.left + metrics.width;
    rect.max_y = layout->document_rect.min_y + metrics.top + metrics.height;

    rect.min_y += block->y;
    rect.max_y += block->y;

    rect.min_x = rect.min_x - 1;
    rect.max_x = rect.min_x + 2;

    draw_rectangle(window,
                   rect.min_x, rect.min_y,
                   rect.max_x, rect.max_y,
                   {0xFF, 0xFF, 0xFF, 0x80});
}

// TODO: Change this API to accept multiple positions and anchors (DocumentCursor)
//       so it can do the job in batches.

void
document_layout_get_cursor(DocumentLayout *layout,
                           DocumentLayoutGetCursor which,
                           DocumentPosition *position,
                           r32 *anchor)
{
    Document *document = layout->session->document;

    switch (which)
    {


    //-----------------------------------------------------------------------------------------
    case DocumentLayoutGetCursor::NextCharacter: {
    //-----------------------------------------------------------------------------------------

        memi length = document_block_length_without_le(document,
                                                       position->block);
        if (position->position == length) {
            if (position->block != (vector_count(document->blocks)-1)) {
                position->block++;
                position->position = 0;
            }
            return;
        }

        DWRITE_HIT_TEST_METRICS metrics;
        FLOAT px, py;
        __DocumentLayout::Block *block = _get_block(layout, position->block);
        HRESULT hr = block->layout.w_layout->HitTestTextPosition(position->position,
                                                                 (BOOL)TRUE,
                                                                 &px, &py,
                                                                 &metrics);

        if (FAILED(hr)) {
            position->position++;
            return;
        }
        *anchor = px;
        position->position += metrics.length;
    } break;


    //-----------------------------------------------------------------------------------------
    case DocumentLayoutGetCursor::PreviousCharacter: {
    //-----------------------------------------------------------------------------------------

        if (position->position == 0) {
            if (position->block != 0) {
                position->block--;
                position->position = document_block_length_without_le(document,
                                                                      position->block);
            }
            return;
        }

        position->position--;

        __DocumentLayout::Block *block = _get_block(layout, position->block);

        DWRITE_HIT_TEST_METRICS metrics;
        FLOAT px, py;
        HRESULT hr;

        hr = block->layout.w_layout->HitTestTextPosition(position->position,
                                                         (BOOL)FALSE,
                                                         &px, &py,
                                                         &metrics);
        if (FAILED(hr)) {
            return;
        }

        *anchor = px;
        position->position = metrics.textPosition;
    } break;
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
               r32 min_x, r32 min_y, r32 max_x, r32 max_y,
               Color32 color)
{
    __Window *impl = &window->impl;
    impl->d_brush->SetColor(to_dcolor(color));
    impl->d_render_target->FillRectangle(D2D1::RectF(min_x, min_y, max_x, max_y),
                                         impl->d_brush);
}


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
