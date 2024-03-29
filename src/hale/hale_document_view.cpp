#if HALE_INCLUDES
#include "hale_document.h"
#include "hale_ui.h"
#endif

namespace hale {

void
document_view_init(DocumentView *view, Document *document, TextProcessor *text_processor)
{
    *view = {};
    view->document = document;
    view->layout = document_layout(text_processor, view);

    view->cursors = {};
    auto cursor = view->cursors.push(1, 16);
    *cursor = {};
}

void
document_view_release(DocumentView *view)
{
    view->cursors.deallocate_safe();
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
// Remove
//

void
document_remove(HALE_STACK, DocumentEdit *edit, RemoveCommand remove)
{
    hale_assert_input(edit->view);
    hale_assert_input(edit->document);

    // TODO: Cycle through the carets or just make a specialized API within the document?

    DocumentCursor *cursor = &edit->view->cursors[0];
    DocumentPosition position;
    r32 anchor;
    switch (remove)
    {
    case Remove_CharacterBackward: {
        hale_assert(cursor->range.first == cursor->range.second);
        position = cursor->range.first;
        anchor = cursor->anchor;
        cursor_previous_character(stack, edit->view, &position, &anchor);
        document_abner(edit, position, cursor->range.first);
    } break;
    }

    cursor->range.first = cursor->range.second = edit->pos_begin;
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
document_view_on_remove(DocumentView *view, DocumentEdit *edit)
{
    document_layout_on_remove(view->layout, edit);
}

void
document_view_scroll_by(HALE_STACK, DocumentView *view, r32 dx, r32 dy)
{
    document_layout_scroll_by(stack, view->layout, dx, dy);
}

void
document_view_scroll_to(DocumentView *view, r32 x, r32 y)
{

}

//
//
//

void
document_view_move_cursor(HALE_STACK, DocumentView *view, MoveCursor move_cursor_function)
{
    auto cursor = &view->cursors[0];
    DocumentPosition position = cursor->range.first;
    r32 anchor = 0;
    move_cursor_function(stack, view, &position, &anchor);
    cursor->range.first = cursor->range.second = position;
    cursor->anchor = anchor;
}

HALE_MOVE_CURSOR(cursor_next_character)
{
    document_layout_get_cursor(stack,
                               view->layout,
                               DocumentLayoutGetCursor::NextCharacter,
                               position, anchor);
}

HALE_MOVE_CURSOR(cursor_previous_character)
{
    document_layout_get_cursor(stack,
                               view->layout,
                               DocumentLayoutGetCursor::PreviousCharacter,
                               position, anchor);
}

} // namespace hale
