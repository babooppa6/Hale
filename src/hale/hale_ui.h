#ifndef HALE_UI_H
#define HALE_UI_H

#include "hale.h"
#include "hale_document.h"

#include "hale_os_ui.h"

namespace hale {

struct View;
struct Panel;
struct Window;
struct App;

struct DocumentLayout;
struct DocumentView;

//
// Options
//

struct CommonOptions
{
    TextFormat *text_format;
    Margin<r32> padding;
    Color32 foreground;
    Color32 background;
};

struct GutterOptions : CommonOptions
{
    // TODO: Reuse format from prototype.
    ch *format;
};

struct AppOptions : public CommonOptions
{
    ch *text_font_family;
};

//
//
//

// TextProcessor is just a way how to keep the reference to RenderTarget on Win32/DX.
// We should perhaps find a better way to implement that.
struct TextProcessor
{
    __TextProcessor impl;
};

struct DocumentLayout
{
    TextProcessor *text_processor;
    DocumentView *session;
    Rect<r32> document_rect;
    Rect<r32> gutter_rect;
    memi scroll_block_first_i;
    r32 scroll_block_first_y;
    memi scroll_block_last_i;
    TextFormat *base_format;

    CommonOptions options;
    GutterOptions gutter_options;

    __DocumentLayout impl;
};

DocumentLayout *document_layout(TextProcessor *text_processor, DocumentView *session);
void document_layout_scroll_by(DocumentLayout *layout, r32 dx, r32 dy);
void document_layout_set_viewport(DocumentLayout *layout, Rect<r32> viewport);
void document_layout_invalidate_format(DocumentLayout *layout, memi from, memi to);
void document_layout_layout(DocumentLayout *layout);
void document_layout_draw(Window *window, DocumentLayout *layout);
void document_layout_draw_cursor(Window *window, DocumentLayout *layout, DocumentPosition position);

void document_layout_on_insert(DocumentLayout *layout, DocumentEdit *edit);

enum struct DocumentLayoutGetCursor
{
    NextCharacter,
    PreviousCharacter
};

void document_layout_get_cursor(DocumentLayout *layout,
                                DocumentLayoutGetCursor which,
                                DocumentPosition *position,
                                r32 *anchor);

//
// View
//

struct View
{
    Panel *panel;
    Rect<r32> client_rect;
    // TODO: Model *model;
};

//
// Panel
//

struct Panel
{
    Window *window;
    Rect<r32> client_rect;
    DocumentView *document_view;
};

void panel_layout(Panel *panel);
void panel_render(Panel *panel);


//
// Window
//

struct Animation;
#define HALE_ANIMATE(name) void name(r32 t, Animation *a)
typedef HALE_ANIMATE(Animate);

struct Animation
{
    void *_key;

    r32 duration;
    r32 elapsed;
    void *data;
    r32 actual;
    r32 final;
    Animate *animate;
};

struct Window
{
    __Window impl;

    Rect<r32> client_rect;
    App *app;
    TextProcessor text_processor;

    Panel panels[16];
    memi panels_count;

    DocumentLayout document_layouts[64];
    memi document_layouts_count;

    typedef StaticMemory<Animation, 32> Animations;
    Animations animations;
};

b32 window_init(Window *window);
void window_layout(Window *window);
void window_render(Window *window);
void window_invalidate(Window *window);
void window_scroll_by(Window *window, r32 x, r32 y, r32 delta_x, r32 delta_y);

Animation *window_get_animation(Window *window, void *key);
Animation *window_add_animation(Window *window, void *key, Animation *animation);

//
// App
//

struct App
{
    __App impl;

    b32 running;
    Window windows[16];
    memi windows_count;

    AppOptions options;
};

struct KeyEvent
{
    enum Modifiers
    {
        Shift,
        Alt,
        AltGr,
        Ctrl,
        WinLeft,
        WinRight
    };

    enum Type
    {
        KeyDown = 0,
        KeyUp = 1,
        Text = 2
    };

    Type type;
    ch32 codepoint;
    u8   modifiers;
    u32  vkey;
};

extern App g_app;

b32 app_init(App *app);
void app_on_key_event(App *app, Window *window, KeyEvent e);

struct Status
{
    // Helper for Panel.
    // Content is in hands of a Model.
};

//
//
//

TextFormat *
text_format(TextProcessor *text_processor,
            r32 size,
            TextFormat::Weight weight,
            TextFormat::Style style,
            Color32 color);

b32
text_layout(TextProcessor *text_processor,
            TextLayout *layout,
            const ch16 *text,
            memi text_length,
            r32 max_width,
            r32 max_height,
            TextFormat *base_style);

void
text_layout_release(TextProcessor *text_processor,
                    TextLayout *layout);

void
text_layout_update(TextProcessor *text_processor,
                   TextLayout *layout,
                   ch16 *text,
                   memi text_length,
                   TextFormat *format_base,
                   TextFormatRange *formats,
                   memi formats_count,
                   r32 max_width);
void
draw_text(Window *window,
          r32 x, r32 y, r32 width, r32 height,
          TextFormat *format,
          const ch16 *text,
          memi text_length);

void
draw_text(Window *window,
          r32 x, r32 y,
          TextLayout *layout);

//
//
//

void
draw_rectangle(Window *window,
               r32 min_x, r32 min_y, r32 max_x, r32 max_y,
               Color32 color);


void
draw_rectangle(Window *window,
               Rect<r32> rect,
               Color32 color);

} // namespace hale

#endif // HALE_UI_H
