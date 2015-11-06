#ifndef HALE_UI_H
#define HALE_UI_H

#if HALE_INCLUDES
#include "hale.h"
#include "hale_document.h"

#include "hale_os_ui.h"
#include "hale_configuration.h"
#endif

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
    r32 text_font_size;
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
    DocumentView *view;
    Rect<r32> document_rect;
    Rect<r32> gutter_rect;
    memi scroll_begin;
    r32 scroll_begin_y;
    memi scroll_end;
    TextFormat *base_format;

    CommonOptions options;
    GutterOptions gutter_options;

    __DocumentLayout impl;
};

DocumentLayout *document_layout(TextProcessor *text_processor, DocumentView *session);
void document_layout_scroll_by(HALE_STACK, DocumentLayout *layout, r32 dx, r32 dy);
void document_layout_set_viewport(DocumentLayout *layout, Rect<r32> viewport);
void document_layout_layout(HALE_STACK, DocumentLayout *layout);
void document_layout_draw(HALE_STACK, Window *window, DocumentLayout *layout);
void document_layout_draw_cursor(HALE_STACK, Window *window, DocumentLayout *layout, DocumentPosition position);

void document_layout_on_insert(DocumentLayout *layout, DocumentEdit *edit);
void document_layout_on_remove(DocumentLayout *layout, DocumentEdit *edit);
void document_layout_on_format(DocumentLayout *layout, memi first, memi last);

enum struct DocumentLayoutGetCursor
{
    NextCharacter,
    PreviousCharacter
};

void document_layout_get_cursor(HALE_STACK,
                                DocumentLayout *layout,
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

void panel_layout(HALE_STACK, Panel *panel);
void panel_render(HALE_STACK, Panel *panel);


//
// Window
//

struct Animation;
#define HALE_ANIMATE(name) void name(PagedMemory *stack, r32 t, Animation *a)
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

b32 window_init(HALE_STACK, Window *window);
void window_layout(HALE_STACK, Window *window);
void window_render(HALE_STACK, Window *window);
void window_invalidate(Window *window);
void window_scroll_by(HALE_STACK, Window *window, r32 x, r32 y, r32 delta_x, r32 delta_y);

Animation *window_get_animation(Window *window, void *key);
Animation *window_add_animation(Window *window, void *key, Animation *animation);

//
// App
//

// Experimental.
template<memi Type>
struct ScopeObject
{
    enum { Id = Type } id;
};

struct App
{
    ScopeObject<1> type;

    s32    argc;
    ch16 **argv;

    __App impl;

    s32 parsing;
    b32 running;

    Window windows[16];
    memi windows_count;

    ScopeArena configuration;
    AppOptions options;
};

// Experimental.
#if 0
#define HALE_SCOPE_TYPE(Type)\
    inline ch8 *scope_type(Type &type) { return #Type; }\
    inline ch8 *scope_type(Type *type) { return #Type; }\

HALE_SCOPE_TYPE(App)
#endif

// extern App g_app;

b32 app_init(PagedMemory *stack, App *app);
void app_configure(ScopeArena &C);
void app_suspend_parsing(App *app);
void app_resume_parsing(App *app);
void app_on_key_event(PagedMemory *stack, App *app, Window *window, KeyEvent e);
b32  app_on_parse_event(App *app);

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
            TextWeight weight,
            TextStyle style,
            Color32 color,
            TextAlignment alignment = TextAlignment::Leading);

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

//
//
//

} // namespace hale

#endif // HALE_UI_H
