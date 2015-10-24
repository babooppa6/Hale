#ifndef HALE_PLATFORM_WIN32_GDI_H
#define HALE_PLATFORM_WIN32_GDI_H

#include "hale.h"
#if 0
namespace hale {

struct Texture
{
    BITMAPINFO info;
    void *memory;
    int width;
    int height;
    int pitch;
    int bpp;
};

#define HALE_FONT_MAX_CODEPOINTS 65535
#define HALE_FONT_MAX_GLYPHS 4096

struct Glyph
{
    // Rect<s32> atlas;

    Pixel32 *atlas;
    V2<s32> size;
    r32 advance;

    u16 baseline;
};

struct TextFormat
{
    HFONT handle;
    // Win32Texture *atlas;
    Pixel32 *atlas;
    TEXTMETRICW text_metrics;
    u16 codepoint_to_glyph[HALE_FONT_MAX_CODEPOINTS];
    Glyph glyphs[HALE_FONT_MAX_GLYPHS];
    memi glyphs_count;
};

} // namespace hale
#endif

#endif // HALE_PLATFORM_WIN32_GDI_H

