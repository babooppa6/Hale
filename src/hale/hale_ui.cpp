#include "hale_ui.h"
#include "hale_perf.h"

namespace hale {

//
//
//

b32
app_init(App *app)
{
    app->options.text_font_family = (ch16*)L"Nitti";
    return 1;
}

void
app_on_key_event(App *app,
                 Window *window,
                 KeyEvent e)
{
    DocumentView *view = window->panels[0].document_view;
    if (e.type == KeyEvent::KeyDown)
    {
        switch (e.vkey)
        {
        case VK_RIGHT: {
            document_view_move_cursor(view, cursor_next_character);
            window_invalidate(window);
        } break;
        case VK_LEFT: {
            document_view_move_cursor(view, cursor_previous_character);
            window_invalidate(window);
        } break;
        }
    }
    else if (e.type == KeyEvent::Text)
    {
        HALE_PERFORMANCE_TIMER(type_character);

        DocumentEdit edit;
        document_edit(&edit, view->document, view);
        document_insert(&edit, e.codepoint);
        window_invalidate(window);
    }
}

//
// --
//

b32
window_init(Window *window)
{
    static Document document;
    document_init(&document);
    // document_load(&document, (ch8*)(__PROJECT__ "tests/jquery-2.1.4.min.js"));
    document_load(&document, (ch8*)(__PROJECT__ "tests/hale_fixed_gap_buffer.cpp"));

    document_add_view(&document, &window->text_processor);
    Panel *panel = &window->panels[0];
    panel->window = window;

    DocumentView *view = &document.views[0];
    panel->document_view = view;

    // TODO: Base and Gutter formats should be actually defined on App, as they are shared.

    DocumentLayout *layout = view->layout;
    layout->base_format   = text_format(&window->text_processor,
                                        18.0f,
                                        TextFormat::Weight::Light,
                                        TextFormat::Style::Normal,
                                        {0xff, 0xff, 0xFF, 0xff});  // {0xe8, 0xf0, 0xFF, 0x80}

    r32 todo_line_height = 18.0f;

    layout->options = {};
    layout->options.padding.top = todo_line_height * 2;
    layout->gutter_options = {};
    layout->gutter_options.padding.left = 8;
    layout->gutter_options.padding.right = 8;
    layout->gutter_options.text_format =
            text_format(&window->text_processor,
                        layout->base_format->size,
                        TextFormat::Weight::Normal,
                        TextFormat::Style::Normal,
                        {0x39, 0x48, 0x50, 0xFF});

    window->panels_count = 1;

    return 1;
}

void
window_layout(Window *window)
{
    Panel *panel;
    for (memi i = 0; i != window->panels_count; i++)
    {
        panel = &window->panels[i];
        // TODO: Layout.
        panel->client_rect = window->client_rect;
        panel_layout(panel);
    }
}

void
window_render(Window *window)
{
    HALE_PERFORMANCE_TIMER(window_render);
    Panel *panel;
    for (memi i = 0; i != window->panels_count; i++)
    {
        panel = &window->panels[i];
        panel_render(panel);
    }
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
_scroll_animate(r32 t, Animation *animation)
{
    auto view = (DocumentView *)animation->data;

    r32 p = animation->final * sine_ease_out(t);

    document_view_scroll_by(view, 0, p - animation->actual);
    animation->actual = p;
}

void
window_scroll_by(Window *window, r32 x, r32 y, r32 delta_x, r32 delta_y)
{
    // TODO: Use default line height from the typographical grid.
    r32 todo_line_height = 18.0f;
    // TODO: Use operating system settings for default scroll amount.
    r32 scroll_amount = todo_line_height * 4.0f;

    // TODO: Find panel by x and y.
    Panel *panel = &window->panels[0];

    // TODO: Store the animation with (?) the DocumentView we're scrolling in.
    void *key = &panel->document_view->layout->scroll_block_first_i;
    Animation *a = window_get_animation(window, key);
    if (a == 0) {
        Animation animation = {};
        animation.animate = _scroll_animate;
        animation.duration = 0.2f;
        animation.data = panel->document_view;
        animation.final = delta_y * scroll_amount;
        if (!(a = window_add_animation(window, key, &animation))) {
            document_view_scroll_by(panel->document_view, 0, animation.final);
        }
    } else {
        a->final = a->final + (delta_y * scroll_amount) - a->actual;
        a->actual = 0;
        a->duration = hale_minimum(0.2f, a->duration + 0.1f - a->elapsed);
        a->elapsed = 0;
    }
}

//
// --
//

void
panel_layout(Panel *panel)
{
    // TODO: Layout current view.
    document_layout_set_viewport(panel->document_view->layout,
                                 panel->client_rect);
}

void
panel_render(Panel *panel)
{
    draw_rectangle(panel->window, panel->client_rect, {0x15, 0x1C, 0x24, 0xff});
    // TODO: Draw current view.
    DocumentLayout *layout = panel->document_view->layout;
    document_layout_draw(panel->window, layout);
    document_layout_draw_cursor(panel->window, layout, panel->document_view->cursors[0].range.first);
}

} // namespace hale
