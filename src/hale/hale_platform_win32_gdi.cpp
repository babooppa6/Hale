#include "hale.h"
#include "hale_platform_win32_ui.h"
#include "hale_platform_win32_gdi.h"

namespace hale {

// *********************************************************************************
//
//   Framebuffer API
//
// *********************************************************************************

// TODO: Add pitch?
void
draw_bitmap(Texture *target,
            Rect<s32> target_rect,
            Pixel32 *pixels)
{
    Rect<s32> tr = rect_intersect(target_rect, 0, 0, target->width, target->height);
    if (!rect_is_valid(tr)) {
        return;
    }

    V2i size = rect_size(target_rect);

    u8 *target_row = (((u8*)target->memory) +
            ((tr.min_y * target->pitch) + (tr.min_x * target->bpp)));

    Pixel32 *target_p;
    s32 y, x;
    for (y = 0; y != size.y; y++)
    {
        target_p = (Pixel32*)target_row;
        for (x = 0; x != size.x; x++)
        {
            // TODO: Alpha blend.
            *target_p++ = *pixels++;
        }
        target_row += target->pitch;
    }
}

void
draw_bitmap(Texture *target,
            Rect<s32> target_rect,
            Texture *source,
            Rect<s32> source_rect)
{
    // TODO: Do we really need to do this here? `rect_is_valid` assertion should be enough.
    Rect<s32> tr = rect_intersect(target_rect, 0, 0, target->width, target->height);
    Rect<s32> sr = rect_intersect(source_rect, 0, 0, source->width, source->height);

    if (!rect_is_valid(tr) || rect_is_valid(sr)) {
        return;
    }

    s32 sw = sr.max_x - sr.min_x;
    s32 sh = sr.max_y - sr.min_y;

    u8 *target_row = (((u8*)target->memory) +
            ((tr.min_y * target->pitch) + (tr.min_x * target->bpp)));
    u8 *source_row = (((u8*)source->memory) +
            ((sr.min_y * source->pitch) + (sr.min_x * source->bpp)));

    Pixel32 *target_p, *source_p;
    s32 y, x;
    for (y = 0; y != sh; y++) {
        // TODO: Can we do this better? (without needing another var?)
        target_p = (Pixel32*)target_row;
        source_p = (Pixel32*)source_row;
        for (x = 0; x != sw; x++) {
            // TODO: Alpha blend.
            *target_p++ = *source_p++;
        }
        target_row += target->pitch;
        source_row += source->pitch;
    }
}

void
draw_text(Texture *buffer, TextFormat *font, ch16 *text, memi text_length)
{
    s32 x = 0;
    for (memi i = 0; i != text_length; i++)
    {
        // TODO: Use utf32_from<Hale>().
        u32 codepoint = text[i];
        hale_assert(codepoint < HALE_FONT_MAX_CODEPOINTS);

        u16 glyph_index = font->codepoint_to_glyph[codepoint];
        hale_assert(glyph_index < font->glyphs_count);

        Glyph *glyph = font->glyphs + glyph_index;

        // Rect<s32> sr = glyph->atlas_coords;
        Rect<s32> tr = {};
        tr.max = glyph->size;
        tr.max_x--;
        tr.max_y--;
        tr.min_x += x;
        tr.max_x += x;
        // tr.max = glyph->size;

        draw_bitmap(buffer, tr, glyph->atlas);
        x += glyph->advance;
    }
}

// *********************************************************************************
//
//   Framebuffer
//
// *********************************************************************************

hale_internal void
win32_framebuffer_resize(Texture *buffer, int width, int height)
{
    if (buffer->memory) {
        platform.deallocate_memory(buffer->memory);
    }

    buffer->width = width;
    buffer->height = height;

    int bpp = 4;
    buffer->bpp = bpp;

    // NOTE(casey): When the biHeight field is negative, this is the clue to
    // Windows to treat this bitmap as top-down, not bottom-up, meaning that
    // the first three bytes of the image are the color for the top left pixel
    // in the bitmap, not the bottom left!
    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = buffer->height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;

    buffer->pitch = hale_align_16(width*bpp);
    int size = (buffer->pitch*buffer->height);
    buffer->memory = platform.allocate_memory(size);
    memset(buffer->memory, 0x80, size);
}

hale_internal void
win32_framebuffer_display(Texture *buffer, HDC dc,
                          int window_width, int window_height)
{
    // TODO: Resize the buffer here?

    StretchDIBits(dc,
                  0, 0, window_width, window_height,
                  0, 0, buffer->width, buffer->height,
                  buffer->memory,
                  &buffer->info,
                  DIB_RGB_COLORS, SRCCOPY);
}

// *********************************************************************************
//
//   Font
//
// *********************************************************************************

// TODO: Add weight, italic, quality.
// TODO: Use CreateFontA for finding another good font: (If lpszFace is NULL or empty string, GDI uses the first font that matches the other specified attributes.) To get the appropriate font on different language versions of the OS, call EnumFontFamiliesEx with the desired font characteristics in the LOGFONT structure, retrieve the appropriate typeface name, and create the font using CreateFont or CreateFontIndirect.

void
win32_fontspec_to_logfont(HDC dc, TextFormat *spec, LOGFONTW *log_font)
{
    log_font->lfHeight = -MulDiv(spec->size, GetDeviceCaps(dc, LOGPIXELSY), 72);
    log_font->lfWidth = 0;
    log_font->lfEscapement = 0;
    log_font->lfOrientation = 0;
    log_font->lfWeight = (LONG)spec->weight;
    log_font->lfItalic = spec->italic;
    log_font->lfUnderline = FALSE;
    log_font->lfStrikeOut = FALSE;
    log_font->lfCharSet = DEFAULT_CHARSET;
    log_font->lfOutPrecision = OUT_DEFAULT_PRECIS;
    log_font->lfClipPrecision = CLIP_DEFAULT_PRECIS;
    log_font->lfQuality = CLEARTYPE_QUALITY; // TODO: ANTIALIASED_QUALITY
    log_font->lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;

    hale_assert(spec->family);

    memory_copy0((ch16*)log_font->lfFaceName, LF_FACESIZE, spec->family);
}

b32
font_init(TextFormat *font, const ch16 *path, TextFormat *spec)
{
    int fonts_added = AddFontResourceExW((LPCWSTR)path, FR_PRIVATE, 0);
    if (fonts_added == 0) {
        return 0;
    }

    // TODO: name
//    vector_init(&font->name);
//    vector_copy(&font->name, name);

    HDC dc0 = GetDC(0);

    LOGFONT lf;
    win32_fontspec_to_logfont(dc0, spec, &lf);
    font->handle = CreateFontIndirectW(&lf);
    if (font->handle != 0)
    {
        HDC dc = CreateCompatibleDC(GetDC(0));
        if (dc)
        {
            SelectObject(dc, font->handle);
            GetTextMetricsW(dc, &font->text_metrics);

            LONG max_char_width = font->text_metrics.tmMaxCharWidth;
            LONG max_char_height = font->text_metrics.tmHeight;

            BITMAPINFO info = {};
            info.bmiHeader.biSize = sizeof(info.bmiHeader);
            info.bmiHeader.biWidth = max_char_width;
            info.bmiHeader.biHeight = max_char_height;
            info.bmiHeader.biPlanes = 1;
            info.bmiHeader.biBitCount = 32;
            info.bmiHeader.biCompression = BI_RGB;
            info.bmiHeader.biSizeImage = 0;
            info.bmiHeader.biXPelsPerMeter = 0;
            info.bmiHeader.biYPelsPerMeter = 0;
            info.bmiHeader.biClrUsed = 0;
            info.bmiHeader.biClrImportant = 0;

            Pixel32 *dib_begin;
            HBITMAP bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, (VOID**)&dib_begin, 0, 0);
            SelectObject(dc, bitmap);

            ch32 range_from = 0x21;
            ch32 range_to = 0x100;
            ch32 range_count = range_to - range_from;

            // TODO: Pool this, as most common use case will be to change the whole main font for another.
            memi pixels_count = range_count * (max_char_width * max_char_height);
            font->atlas = (Pixel32*)platform.allocate_memory(pixels_count * sizeof(Pixel32));

            ch16 unit[2];
            SIZE size;
            INT advance;

            font->glyphs_count = 0;
            Glyph *glyph = font->glyphs;

            Pixel32 *atlas = font->atlas;
            Pixel32 *dib = dib_begin;

            SetBkMode(dc, TRANSPARENT);
            SetBkColor(dc, RGB(0, 0, 0));
            SetTextColor(dc, RGB(255, 255, 255));

            for (ch32 i = range_from; i != range_to; i++)
            {
                // TODO: Use either Hale or UTF16 encoding's utf32_from<>()
                unit[0] = i;
                ZeroMemory(dib_begin, (max_char_width * max_char_height) * sizeof(Pixel32));
                if (i == 0xb4) {
                    _CrtDbgBreak();
                }
                GetTextExtentPoint32W(dc, (LPCWSTR)&unit, 1, &size);
                GetCharWidth32W(dc, i, i, &advance);
#if 1
                ABC ThisABC;
                hale_assert(GetCharABCWidthsW(dc, i, i, &ThisABC));
                r32 advance_abc = (r32)(ThisABC.abcA + ThisABC.abcB + ThisABC.abcC);
#endif


                TextOutW(dc, 0, 0, (LPCWSTR)&unit, 1);

                glyph->atlas = atlas;
                glyph->size.x = size.cx;
                glyph->size.y = size.cy;
                glyph->advance = (r32)advance;

                for (s32 y = 0; y < max_char_height; y++)
                {
                    dib = dib_begin + (y * max_char_width);
                    for (s32 x = 0; x < size.cx; x++)
                    {
                        *atlas++ = *dib++;
                    }
                    // TODO: Use += pitch.
                }


                hale_assert_debug(font->glyphs_count <= HALE_FONT_MAX_GLYPHS);
                font->codepoint_to_glyph[i] = (u16)font->glyphs_count;
                font->glyphs_count++;

                glyph++;
            }

            DeleteObject(bitmap);
            DeleteDC(dc);
        }
        else
        {
            hale_error("CreateCompatibleDC");
        }
    }
    else
    {
        win32_print_error("CreateFontIndirectW");
        hale_error("CreateFontIndirectW");
    }

    ReleaseDC(0, dc0);
    RemoveFontResourceEx((LPCWSTR)path, FR_PRIVATE, 0);

    return 1;
}

void
font_release(TextFormat *font)
{
    if (font)
    {
        platform.deallocate_memory(font->atlas);
    }
}

// *********************************************************************************
//
//   Window
//
// *********************************************************************************

hale_internal b32
__window_init(AppImpl *app, WindowImpl *window)
{
    // TODO: Move to gdi::__window_create()
    RECT rc;
    GetClientRect(window->handle, &rc);
    win32_framebuffer_resize(&window->framebuffer, rc.right, rc.bottom);

    return 1;
}

hale_internal b32
__window_paint()
{
    RECT rc;
    GetClientRect(handle, &rc);
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(handle, &ps);
    win32_framebuffer_display(&window->framebuffer, dc, rc.right, rc.bottom);
    EndPaint(handle, &ps);
}

} // namespace hale
