#include "hale_document.h"

namespace hale {

void
document_session_init(DocumentSession *session, Document *document, TextProcessor *text_processor)
{
    session->document = document;
    session->layout = document_layout(text_processor, session);

    vector_init(&session->cursors, 1);
    vector_push<DocumentCursor>(&session->cursors, {});
}

void
document_session_release(DocumentSession *session)
{
    vector_release(&session->cursors);
}

DocumentSession *
document_add_session(Document *document, TextProcessor *text_processor)
{
    // TODO: Clone other, get rid of document.
    // TODO: Reuse older session instances.

    if (document->sessions_count == hale_array_count(document->sessions)) {
        hale_error("Too many sessions for document.");
        return NULL;
    }

    auto session = &document->sessions[document->sessions_count];
    document->sessions_count++;

    // Init.
    // TODO: A different window's text processor might be comming here.
    document_session_init(session, document, text_processor);
}

void
document_remove_session(DocumentSession *session)
{
    hale_not_implemented;
}

//
//
//

void
document_session_scroll_by(DocumentSession *session, r32 dx, r32 dy)
{

}

void
document_session_scroll_to(DocumentSession *session, r32 x, r32 y)
{

}

//
//
//

// TODO: Implement something like this to make the document text retrieval from document faster.
struct TextBuffer
{
    memi buf_length;

    // Uses static if the text is smaller than 4096.
    ch16 buf_static[4096];
    // Otherwise uses the heap.
    ch16 *buf_heap;
};

// TODO: All this is platform-dependent implementation detail.
// TODO: All this stuff should happen only after an edit (before we go for render),
//       it can be optimized to only happen within a working set. Working set can also
//       use a separate temporary buffer, to make the text retrievals faster. Or at least
//       be kept in a good fast-to-read state before we pass it to the renderer.
// TODO: We can also make the layout updates being active, managed directly from
//       `document_session_block_invalidate()`.

#if 0
DocumentSession::Block *
document_session_block(DocumentSession *session, memi block_index)
{
    ch16 *text = NULL;
    memi text_length = 0;
    TextFormatRange *formats = NULL;
    memi formats_count = 0;

    auto block = &session->blocks[block_index];

    // Text

    if (((block->flags & DocumentSession::Block::TextValid) == 0))
    {
        // TODO: Optimize this with TextBuffer, see above.
        // TODO: If we would keep only a shorter cache of the blocks, we can optimize there.

        text_length = document_block_length_without_le(session->document, block_index);
        text = memory_allocate<ch16>(text_length);

        memi begin = document_block_begin(session->document, block_index);
        document_text(session->document, begin, text_length, text, text_length);

        block->flags |= DocumentSession::Block::TextValid;
        block->flags &= ~DocumentSession::Block::FormatValid;
    }

    // Formats

    if ((block->flags & DocumentSession::Block::FormatValid) == 0)
    {
        block->flags |= DocumentSession::Block::FormatValid;
        // TODO: Update the formats.
    }

    if (text || formats)
    {
        text_layout_update(session->text_processor,
                           &block->layout,
                           text, text_length,
                           formats, formats_count,
                           session->layout_width);

        if (text) {
            memory_deallocate(text);
        }
    }

    // TODO: Invalidate cursor's vertical anchors if they are on this block.

    return block;
}
#endif

} // namespace hale
