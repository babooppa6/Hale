#include "hale_ui.h"
#include "hale_document.h"
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

}

//
//
//

b32
window_init(Window *window)
{
    static Document document;
    document_init(&document);
    document_load(&document, (ch8*)(__PROJECT__ "tests/hale_fixed_gap_buffer.cpp"));

    document_add_session(&document, &window->text_processor);
    Panel *panel = &window->panels[0];
    panel->window = window;

    DocumentSession *session = &document.sessions[0];
    panel->document_session = session;

    DocumentLayout *layout = session->layout;
    layout->base_format = text_format(&window->text_processor,
                                      18.0f,
                                      TextFormat::Weight::Light,
                                      TextFormat::Style::Normal,
                                      {0xFF, 0xFF, 0xFF, 0xFF});


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
    Panel *panel;
    for (memi i = 0; i != window->panels_count; i++)
    {
        panel = &window->panels[i];
        panel_render(panel);
    }
}

//
//
//

void
panel_layout(Panel *panel)
{
    // TODO: Layout current view.
    panel->document_session->layout->viewport = panel->client_rect;
    qDebug() << "Layout"
             << panel->document_session->layout->viewport.min_x
             << panel->document_session->layout->viewport.min_y
             << panel->document_session->layout->viewport.max_x
             << panel->document_session->layout->viewport.max_y;
    document_layout_layout(panel->document_session->layout);
}

void
panel_render(Panel *panel)
{
    HALE_PERFORMANCE_TIMER(panel_render);

    draw_rectangle(panel->window, panel->client_rect, {0x15, 0x1C, 0x24, 0xff});
    // TODO: Draw current view.
    document_layout_draw(panel->window, panel->document_session->layout, 0, 0);
}

} // namespace hale
