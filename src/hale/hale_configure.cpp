#if HALE_INCLUDES
#include "hale_macros.h"
#include "hale_configuration.h"
#include "hale_document.h"
#include "hale_ui.h"
#endif

namespace hale {

hale_internal
HALE_EVENT_HANDLER(_app_configure)
{
    auto app = args->scope->top<App*>("app");

//    AppOptions options = app->options;
//    options.text_font_family = hale_ch("Consolas");
//    options.text_font_size = 16.0f;
//    app->set_options(options);
}

hale_internal struct
{
    EventHandler::Args args;
    static HALE_EVENT_HANDLER(handler) {
        auto event = args->get<KeyEvent*>(0);
        auto view = args->scope->top<DocumentView*>("document");
        DocumentEdit edit;
        document_edit(&edit, view->document, view);
        document_insert(&edit, event->key.codepoint);
    }
} _document_text = {};

#if 0
hale_internal
HALE_EVENT_HANDLER(_document_text)
{
    auto event = args->get<KeyEvent*>(0);
    auto view = args->scope->top<DocumentView*>("document");
    DocumentEdit edit;
    document_edit(&edit, view->document, view);
    document_insert(&edit, event->key.codepoint);
}
#endif


hale_internal
HALE_EVENT_HANDLER(_document_undo)
{
    document_undo(args->scope->top<DocumentView*>("document")->document);
}

hale_internal
HALE_EVENT_HANDLER(_document_next_character)
{
    document_view_move_cursor(stack, args->scope->top<DocumentView*>("document"), cursor_next_character);
}

hale_internal
HALE_EVENT_HANDLER(_document_previous_character)
{
    document_view_move_cursor(stack, args->scope->top<DocumentView*>("document"), cursor_previous_character);
}

#define HANDLER(name) \
    EventHandler {name.handler, &name.args, sizeof(name), &name}

#define PROPERTY(context, name, value)\
    configuration::property(#context, #name, value)

#define ON_KEY_DOWN(modifier, vkey, handler)\
    configuration::property((ch8*)0, A.k_key_down, configuration::key({modifier, vkey}, handler))

#define ON_TEXT(handler)\
    configuration::property((ch8*)0, A.k_text, configuration::text(HANDLER(handler)))

#define COLOR(color)\
    Color32(0x##color)

void
app_configure(ScopeArena &A)
{
    A("app", {
        PROPERTY(app, font_family, "Consolas"),
        PROPERTY(app, font_size, 16.0f),
    });

    A("document", {
        ON_TEXT(_document_text),
//        ON_KEY_DOWN(KeyM_Ctrl, 'Z', _document_undo),
//        ON_KEY_DOWN(KeyM_None, VK_RIGHT, _document_next_character),
//        ON_KEY_DOWN(KeyM_None, VK_LEFT,  _document_previous_character),
//        PROPERTY(document, foreground, COLOR(000F00)),
//        PROPERTY(document, background, COLOR(FFFFFF))
    });
}

#undef PROPERTY
#undef ON_KEY_DOWN
#undef ON_TEXT
#undef COLOR

}
