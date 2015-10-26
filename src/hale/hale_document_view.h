#ifndef HALE_DOCUMENT_VIEW_H
#define HALE_DOCUMENT_VIEW_H

#if HALE_INCLUDES
#include "hale.h"
#include "hale_encoding.h"
#include "hale_document_types.h"
#endif

namespace hale {

struct Document;
struct DocumentLayout;

struct DocumentCursor
{
    DocumentRange range;
    // TODO: Do we really need the anchor to be remembered?
    //       It can be calculated before we move from it's current line
    //       to another. The problem is with subsequent moves up/down
    //       where we want to keep the movement approx. to the anchor.
    //       We can also try to use column, which would be more geometrically
    //       correct.
    // TODO: We can also store the anchors separately and recalculate them
    //       in one go.
    r32 anchor;
};

struct TextProcessor;
struct DocumentLayout;
struct DocumentEdit;

struct DocumentView
{
    Document *document;
    Vector<DocumentCursor> cursors;

    // TODO: Make DocumentLayout into a platform implementation of DocumentView.
    DocumentLayout *layout;
};

DocumentView *
document_add_view(Document *document, TextProcessor *text_processor);
void document_remove_view(DocumentView *view);

// TODO: inline
void document_insert(DocumentEdit *edit, ch16 *text, memi text_length);
// TODO: inline
void document_insert(DocumentEdit *edit, ch32 codepoint);

void document_view_on_insert(DocumentView *view, DocumentEdit *edit);
void document_view_scroll_by(DocumentView *view, r32 dx, r32 dy);
void document_view_scroll_to(DocumentView *view, r32 x, r32 y);

#define HALE_MOVE_CURSOR(name) void name(DocumentView *view, DocumentPosition *position, r32 *anchor)
typedef HALE_MOVE_CURSOR(MoveCursor);
HALE_MOVE_CURSOR(cursor_next_character);
HALE_MOVE_CURSOR(cursor_previous_character);

void document_view_move_cursor(DocumentView *view, MoveCursor move_cursor_function);

}

#endif // HALE_DOCUMENT_VIEW_H

