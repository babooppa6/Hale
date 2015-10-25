#ifndef HALE_PLATFORM_WIN32_DX_H
#define HALE_PLATFORM_WIN32_DX_H

#include <dwrite.h>

struct ID2D1Factory;
struct ID2D1HwndRenderTarget;
struct ID2D1SolidColorBrush;

//struct IDWriteTextFormat;
//struct IDWriteFactory;
//struct IDWriteTextLayout;

namespace hale {

struct TextFormat
{
    // TODO: Predefined font sizes?
    // HTML uses 6-7 basic sizes. I might only go with 4?
    r32 size;

    // TODO: Bitfields for these? I rarely use more than 4 font weights.
    enum struct Weight
    {
        Thin = 100,
        ExtraLight = 200,
        UltraLight = 200,
        Light = 300,
        Normal = 400,
        Medium = 500,
        SemiBold = 600,
        Bold = 700,
        ExtraBold = 800,
        Black = 900,
        ExtraBlack = 950
    };
    Weight weight;

    // TODO: Bitfields.
    enum struct Style
    {
        Normal,
        Italic
    };
    Style style;

    // TODO: Already packed as u32. But a palette would be much better and much more consistent. But then there's problem with color animations. (is there?)
    Color32 color;

    IDWriteTextFormat *_format;
    ID2D1SolidColorBrush *_brush;
};

// TODO: Do I even need text format? Ranges might be designed just in a way of "start bold" / "start weight_normal", "start size Large", "start size Small".

struct TextFormatRange
{
    memi begin;
    memi end;
    TextFormat *format;
};

struct TextLayout
{
    // Common
    TextFormat *base;

    // Platform
    IDWriteTextLayout *w_layout;
    DWRITE_TEXT_METRICS w_metrics;
};

struct Window;

struct __TextProcessor
{
    Window *window;
};

struct __DocumentLayout
{
    // Window *window;

    // Using `valid` signalling instead of `invalid` as we then can simply
    // initialize the whole structure to 0 and it'll all work.
    enum BlockFlags
    {
        TextValid = 0x01,
        FormatValid = 0x02
    };

    struct Block
    {
        u32 flags;
        r32 y;
        TextLayout layout;
    };
    Vector<Block> blocks;
    b32 layout_invalid;
};

#define __APP_IMPL\
    float dpi_scale_x;\
    float dpi_scale_y;\
    ID2D1Factory *d_factory;\
    IDWriteFactory *w_factory;\
    DWRITE_FONT_METRICS font_metrics

#define __WINDOW_IMPL\
    ID2D1HwndRenderTarget *d_render_target;\
    ID2D1SolidColorBrush *d_brush;\
    TextFormat text_formats[64];\
    memi text_formats_count;

} // namespace hale

#endif // HALE_PLATFORM_WIN32_DX_H