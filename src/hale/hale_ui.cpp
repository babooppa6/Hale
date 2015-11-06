#if HALE_INCLUDES
#include "hale_ui.h"
#include "hale_perf.h"
#include "hale_key.h"
#include "hale_configuration.h"
#include "hale_parser_c.h"
#endif

namespace hale {

#if HALE_DEBUG
hale_global r64 _keystroke_time = 0;
hale_global b32 _keystroke = 0;
#endif

//
//
//

b32
app_init(HALE_STACK, App *app)
{
    app->configuration = {};
    scope_arena(&app->configuration);

    app_configure(app->configuration);

    // TODO: Window creation
    // TODO: Documents

    // TODO: Initial configuration

    app->options.text_font_family = (ch16*)L"Consolas";
    app->options.text_font_size = 16.0f;

    return 1;
}

// How does the state change?
// When does the state change?
// Imagine a scope to be a single number.
// - It can be an index to last leaf of a tree.
// 0 {app}, 0
// 1 {window}, 0
// 2 {panel}, 1
// 3 {view}, 2
// 4 {app.switch}, 0
// 5 {window}, 4
// 6 {panel}, 5
// 7 {view}, 6
// 8 [view.focused, view.changed], 7
// 9 [view.focused], 7
// A [view.changed], 7

// 7 = app, app.switch, window, panel, view
// 3 = app, window, panel, view

// - There are attributes that do not function as nodes, but rather as one node

Scope *
get_scope(HALE_STACK, App *app, Window *window, Panel *panel, DocumentView *view)
{
    ScopeArena &A = app->configuration;
//    Scope::E path[4];
//    scope_push()
//    path[0] = {A.k_app, app};
//    path[1] = {A.k_window, window};
//    path[2] = {A.k_panel, panel};
//    path[3] = {A.k_view, view};
//    return scope_get(stack, &A, path, path + 4);
    return 0;
}

//void
//on_panel_model_changed(App *app, Window *window, Panel *panel, Model *model)
//{

//}

void
app_on_key_event(HALE_STACK,
                 App *app,
                 Window *window,
                 KeyEvent e)
{
#if HALE_DEBUG
    _keystroke = 1;
    _keystroke_time = platform.read_time_counter();
#endif

    // Get current scope.
    Panel *panel = &window->panels[0];
    DocumentView *view = panel->document_view;

    EventHandlerCallArgs args;
    args.scope = get_scope(stack, app, window, panel, view);

#if 0
    KeyEventHandler *it, *end;
    switch (e.type)
    {
    case KeyT_KeyDown:
        end = it = args.scope->data.key_down;
        end += args.scope->data.key_down_count;
        break;
    case KeyT_KeyUp:
        end = it = args.scope->data.key_up;
        end += args.scope->data.key_up_count;
        break;
    case KeyT_Text:
        end = it = args.scope->data.text;
        end += args.scope->data.text_count;
        break;
    }

    for (; it != end; ++it)
    {
        if (it->key == e.key) {
            it->handler(&args);
            break;
        }
    }
#endif
#if 1

    DocumentEdit edit;
    if (e.type == KeyT_KeyDown)
    {
        switch (e.key.key)
        {
        case VK_RIGHT: {
            document_view_move_cursor(stack, view, cursor_next_character);
            window_invalidate(window);
        } break;
        case VK_LEFT: {
            document_view_move_cursor(stack, view, cursor_previous_character);
            window_invalidate(window);
        } break;
        case VK_BACK: {
            document_edit(&edit, view->document, view);
            document_remove(stack, &edit, Remove_CharacterBackward);
            window_invalidate(window);
        } break;
        case 'Z': {
            if (e.key.modifiers & KeyM_Ctrl) {
                document_undo(view->document);
                window_invalidate(window);
            }
        } break;
        }
    }
    else if (e.type == KeyT_Text)
    {
        // HALE_PERFORMANCE_TIMER(type_character);

        // TODO: For inserting the new line, we can perhaps make a special document_insert.
        if (e.key.codepoint >= 0x20 || e.key.codepoint == 0x0D)
        {
            document_edit(&edit, view->document, view);
            document_insert(&edit, e.key.codepoint);
            window_invalidate(window);
        }
    }
#endif
}

void
notify_document_needs_parsing(Document *document)
{
    app_resume_parsing(document->app);
}

b32
app_on_parse_event(App *app)
{
    // HALE_PERFORMANCE_TIMER(app_on_parse_event);

    Window *window;
    Panel *panel;
    memi parsed_blocks = 0;
    memi pb = 0;
    for (memi w = 0; w != app->windows_count; ++w)
    {
        window = &app->windows[w];
        pb = 0;
        for (memi p = 0; p != window->panels_count; ++p)
        {
            panel = &window->panels[p];
            if (document_parser_is_working(panel->document_view->document)) {
                pb += document_parse(panel->document_view->document, 0.02f);
            }
        }
        if (pb) {
            parsed_blocks += pb;
            // TODO: Invalidate only if the change happened in the
            //       visible portion of the document.
            window_invalidate(window);
        }
    }

    if (parsed_blocks) {
        // Print() << __FUNCTION__ << hale_ch("Parsed blocks: ") << parsed_blocks;
    }

    return 1;
}

//
// --
//

b32
window_init(HALE_STACK, Window *window)
{
    static Document document;
    document_init(&document);
    document.app = window->app;

    App *app = window->app;
    if (app->argc != 0) {
        // %{sourceDir}\hale\tests\test-large.cpp
		// ..\src\hale\tests\test-large.cpp
        document_load(&document, app->argv[0]);
    }

    document_add_view(&document, &window->text_processor);
    Panel *panel = &window->panels[0];
    panel->window = window;

    DocumentView *view = &document.views[0];
    panel->document_view = view;

    // TODO: Base and Gutter formats should be actually defined on App, as they are shared.

    DocumentLayout *layout = view->layout;
    layout->base_format   = text_format(&window->text_processor,
                                        app->options.text_font_size,
                                        TextWeight::Light,
                                        TextStyle::Normal,
                                        {0xff, 0xff, 0xFF, 0xff});  // {0xe8, 0xf0, 0xFF, 0x80}

    r32 todo_line_height = app->options.text_font_size;

    layout->options = {};
    layout->options.padding.top = todo_line_height * 2;
    view->layout->scroll_begin_y = layout->options.padding.top;

    layout->gutter_options = {};
    layout->gutter_options.padding.left = 16;
    layout->gutter_options.padding.right = 12;
    layout->gutter_options.text_format =
            text_format(&window->text_processor,
                        layout->base_format->size,
                        TextWeight::Normal,
                        TextStyle::Normal,
                        {0x39, 0x48, 0x50, 0xFF},
                        TextAlignment::Trailing);

    window->panels_count = 1;

    ParserC parser;
    document_parser_set(&document, &parser);

    return 1;
}

void
window_layout(HALE_STACK, Window *window)
{
    Panel *panel;
    for (memi i = 0; i != window->panels_count; i++)
    {
        panel = &window->panels[i];
        // TODO: Layout.
        panel->client_rect = window->client_rect;
        panel_layout(stack, panel);
    }
}

void
window_render(HALE_STACK, Window *window)
{
//    HALE_PERFORMANCE_TIMER(window_render);
    Panel *panel;
    for (memi i = 0; i != window->panels_count; i++)
    {
        panel = &window->panels[i];
        panel_render(stack, panel);
    }

#if HALE_DEBUG
    if (_keystroke) {
        r64 t = platform.read_time_counter();
        // Print() << __FUNCTION__ << "Keystroke Time" << ((t - _keystroke_time) * 1e3) << "ms";
        _keystroke_time = t;
        _keystroke = 0;
    }
#endif
}

struct ScrollAnimation
{
    Window *window;
    DocumentView *view;
    r32 value;
    r32 delta;
};

hale_internal
void
_scroll_animate(HALE_STACK, r32 t, Animation *animation)
{
    auto view = (DocumentView *)animation->data;

    r32 p = animation->final * sine_ease_out(t);

    document_view_scroll_by(stack, view, 0, p - animation->actual);
    animation->actual = p;
}

void
window_scroll_by(HALE_STACK, Window *window, r32 x, r32 y, r32 delta_x, r32 delta_y)
{
    // TODO: Use default line height from the typographical grid.
    r32 todo_line_height = 18.0f;
    // TODO: Use operating system settings for default scroll amount.
    r32 scroll_amount = todo_line_height * 4.0f;

    // TODO: Find panel by x and y.
    Panel *panel = &window->panels[0];

    // TODO: Store the animation with (?) the DocumentView we're scrolling in.
    void *key = &panel->document_view->layout->scroll_begin;
    Animation *a = window_get_animation(window, key);
    if (a == 0) {
        Animation animation = {};
        animation.animate = _scroll_animate;
        animation.duration = 0.2f;
        animation.data = panel->document_view;
        animation.final = delta_y * scroll_amount;
        if (!(a = window_add_animation(window, key, &animation))) {
            document_view_scroll_by(stack, panel->document_view, 0, animation.final);
        }
    } else {
        a->final = a->final + (delta_y * scroll_amount) - a->actual;
        a->actual = 0;
        a->duration = minimum(0.2f, a->duration + 0.1f - a->elapsed);
        a->elapsed = 0;
    }
}

//
// --
//

void
panel_layout(HALE_STACK, Panel *panel)
{
    // TODO: Layout current view.
    document_layout_set_viewport(panel->document_view->layout,
                                 panel->client_rect);
}

void
panel_render(HALE_STACK, Panel *panel)
{
    draw_rectangle(panel->window, panel->client_rect, {0x15, 0x1C, 0x24, 0xff});
    // TODO: Draw current view.
    DocumentLayout *layout = panel->document_view->layout;
    document_layout_draw(stack, panel->window, layout);
    document_layout_draw_cursor(stack, panel->window, layout, panel->document_view->cursors[0].range.first);
}

struct StateNibble
{
    uptr index;
    u16 types[16];
    u8 modes[32];
};

// [.type0.type1.type2.type3... > state0 > state1 > state2 > state3]
//

struct State
{
    App *app;
    u8 app_modes[32];
    memi window;
    u8 window_modes[32];
    memi panel;
    u8 panel_modes[32];
    memi model;
    char model_sub_type[16];
    u8 model_modes[32];
};

} // namespace hale
