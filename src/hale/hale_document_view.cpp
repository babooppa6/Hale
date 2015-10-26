#if HALE_INCLUDES
#include "hale_document.h"
#include "hale_ui.h"
#endif

namespace hale {

void
document_view_init(DocumentView *session, Document *document, TextProcessor *text_processor)
{
    session->document = document;
    session->layout = document_layout(text_processor, session);

    vector_init(&session->cursors, 1);
    vector_push<DocumentCursor>(&session->cursors, {});
}

void
document_view_release(DocumentView *view)
{
    vector_release(&view->cursors);
}

DocumentView *
document_add_view(Document *document, TextProcessor *text_processor)
{
    // TODO: Clone other, get rid of document.
    // TODO: Reuse older session instances.

    if (document->views_count == hale_array_count(document->views)) {
        hale_error("Too many sessions for document.");
        return NULL;
    }

    auto view = &document->views[document->views_count];
    document->views_count++;

    // Init.
    // TODO: A different window's text processor might be comming here.
    document_view_init(view, document, text_processor);

    return view;
}

void
document_remove_view(DocumentView *view)
{
    hale_not_implemented;
}

//
//
//

void
document_insert(DocumentEdit *edit, ch32 codepoint)
{
    hale_assert_input(edit->view);
    ch16 text[2];
    memi text_length = utf32_to_hale_trusted(codepoint, text);
    document_insert(edit, text, text_length);
}

void
document_insert(DocumentEdit *edit, ch16 *text, memi text_length)
{
    hale_assert_input(edit->view);
    hale_assert_input(edit->document);
    // TODO: Cycle through the carets or just make a specialized API within the document?
    document_insert(edit, edit->view->cursors[0].range.first, text, text_length);

    // TODO: Update the cursors.
    edit->view->cursors[0].range.first =
            edit->view->cursors[0].range.second = edit->pos_end;
}

//
//
//

void
document_view_on_insert(DocumentView *view, DocumentEdit *edit)
{
    document_layout_on_insert(view->layout, edit);
}

void
document_view_scroll_by(DocumentView *view, r32 dx, r32 dy)
{
    document_layout_scroll_by(view->layout, dx, dy);
}

void
document_view_scroll_to(DocumentView *view, r32 x, r32 y)
{

}

//
//
//

void
document_view_move_cursor(DocumentView *view, MoveCursor move_cursor_function)
{
    DocumentPosition position = view->cursors[0].range.first;
    r32 anchor = 0;
    move_cursor_function(view, &position, &anchor);
    view->cursors[0].range.first = position;
    view->cursors[0].anchor = anchor;
}

HALE_MOVE_CURSOR(cursor_next_character)
{
    document_layout_get_cursor(view->layout,
                               DocumentLayoutGetCursor::NextCharacter,
                               position, anchor);
}

HALE_MOVE_CURSOR(cursor_previous_character)
{
    document_layout_get_cursor(view->layout,
                               DocumentLayoutGetCursor::PreviousCharacter,
                               position, anchor);
}

} // namespace hale
