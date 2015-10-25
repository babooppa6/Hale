#include "hale.h"
#include "hale_gap_buffer.h"
#include "hale_document.h"

namespace hale {

//
// View notifications.
//

// TODO: Is there any way we can notify the views in less granular way? Ideally when the edit is committed? Can we keep a list of edits for that? Maybe even a fixed array that'll get "flushed" when it's full.

hale_internal inline void
_notify_views_about_block_updated(DocumentEdit *edit)
{

}

hale_internal inline void
_notify_views_about_insert(DocumentEdit *edit)
{
    for (memi i = 0; i != edit->document->views_count; i++) {
        document_view_on_insert(&edit->document->views[i], edit);
    }
}

hale_internal inline void
_notify_views_about_blocks_abnered(DocumentEdit *edit)
{

}

hale_internal inline void
_notify_views_about_edit_begin(DocumentEdit *edit)
{

}

hale_internal inline void
_notify_views_about_edit_end(DocumentEdit *edit)
{

}

//
// DocumentEdit
//

DocumentEdit::~DocumentEdit() {
    document_commit(this);
}

void
document_edit(DocumentEdit *edit, Document *document, DocumentView *session)
{
    edit->document = document;
    edit->view = session;
    edit->undo = false;
    edit->internal = session == NULL;

    edit->type = DocumentEdit::Insert;
    edit->block_changed = 0;
    edit->blocks_changed_at = 0;
    edit->blocks_changed_count = 0;
    edit->pos_begin.block = 0;
    edit->pos_begin.position = 0;
    edit->pos_end.block = 0;
    edit->pos_end.position = 0;
    edit->offset_begin = 0;
    edit->offset_end = 0;
    edit->undo_head = 0;

    // TODO: Notify views about edit being started.
    _notify_views_about_edit_begin(edit);
}

void
document_commit(DocumentEdit *edit)
{
    hale_assert(edit->document);
    edit->document = 0;

    // TODO: Notify views about edit being commited.
    _notify_views_about_edit_end(edit);
}

// TODO: Think this through.

//void
//document_discard(DocumentEdit *edit)
//{
//    hale_assert(edit->document);
//    edit->document = 0;

//    // TODO: Notify views about edit being commited.
//    _notify_views_about_edit_discarded(edit);
//}

//
//
//

hale_internal inline void
_buffer_init(FixedGapArena *arena) {
    fixed_gap_arena_init(arena, 1);
}

hale_internal inline void
_buffer_release(FixedGapArena *arena) {
    fixed_gap_arena_release(arena);
}

hale_internal inline void
_buffer_insert(FixedGapArena *arena, memi offset, ch *text, memi size) {
    fixed_gap_arena_insert(arena, offset * sizeof(ch), (ch8*)text, size * sizeof(ch));
}

hale_internal inline void
_buffer_remove(FixedGapArena *arena, memi offset, memi size) {
    fixed_gap_arena_remove(arena, offset * sizeof(ch), size * sizeof(ch));
}

hale_internal inline void
_buffer_text(FixedGapArena *arena, memi offset, memi size, ch *text) {
    fixed_gap_arena_text(arena, offset * sizeof(ch), size * sizeof(ch), (ch8*)text);
}

hale_internal inline memi
_buffer_length(FixedGapArena *arena) {
    return arena->size / sizeof(ch);
}

hale_internal inline memi
_buffer_length(FixedGapArena &arena) {
    return arena.size / sizeof(ch);
}

//
//
//

hale_internal inline void
_buffer_init(GapBuffer *buffer) {
    gap_buffer_init(buffer, 1024, 256);
}

hale_internal inline void
_buffer_release(GapBuffer *buffer) {
    gap_buffer_release(buffer);
}

hale_internal inline void
_buffer_insert(GapBuffer *buffer, memi offset, ch *text, memi size) {
    gap_buffer_insert(buffer, offset, text, size);
}

hale_internal inline void
_buffer_remove(GapBuffer *buffer, memi offset, memi size) {
    gap_buffer_remove(buffer, offset, size);
}

hale_internal inline void
_buffer_text(GapBuffer *buffer, memi offset, memi size, ch *text) {
    gap_buffer_text(buffer, offset, size, text);
}

//
//
//

// TODO: Make default arena.

Document *
document_alloc(DocumentArena *arena)
{
    hale_unused(arena);
    return (Document*)malloc(sizeof(Document));
}

//
//
//

// defined in hale_document_view.cpp
extern void
document_view_init(DocumentView *session, Document *document, TextProcessor *text_processor);

// defined in hale_document_view.cpp
extern void
document_view_release(DocumentView *session);

void
document_init(Document *document)
{
    document->arena = 0;

    vector_init(&document->path);
    _buffer_init(&document->buffer);

    document->indentation_mode = IndentationMode::Spaces;
    document->indentation_size = 4;

    // document->undo = new UndoStream(NULL);

    // Parser
    document->parser_status = 0;
    document->parser_head = 0;
    // vector_init(&document->parser_state.stack);
    // document->parser = new Parser();

    document->indentation_size = 4;
    document->indentation_mode = IndentationMode::Spaces;

    Document::Block block;
    block.end = 0;
    block.flags = 0;
    vector_init(&document->blocks, 1);
    vector_push(&document->blocks, block);
    // vector_push(&document->block_info, (Document::Block*)NULL);
}

void
document_release(Document *document)
{
    hale_assert(document);

    _buffer_release(&document->buffer);

    for (memi i = 0;
         i < document->views_count;
         ++i)
    {
        document_view_release(&document->views[i]);
    }
    document->views_count = 0;

    // TODO: Free sessions.

    vector_release(&document->blocks);
    // vector_release(&document->views);
    // vector_free(&document->blocks);
    // vector_free(&document->sessions);
}

//
// Options
//

void
document_set_indentation_mode(Document *document, IndentationMode indentation_mode)
{
    if (indentation_mode != document->indentation_mode) {
        document->indentation_mode = indentation_mode;
        // TODO: emit indentationModeChanged();
    }
}

void
document_set_indentation_size(Document *document, u32 indentation_size)
{
    // qWarning() << __FUNCTION__ << "Indentation size should be in range 1 .." << Document::MAX_INDENTATION_SIZE;
    hale_assert_input(indentation_size >= 1 && indentation_size <= Document::MAX_INDENTATION_SIZE);

    if (indentation_size != document->indentation_size) {
        document->indentation_size = indentation_size;
        // emit indentationSizeChanged();
    }
}

//void
//document_set_grammar(Document *document, QSharedPointer<Grammar> grammar)
//{
//    hale_assert(grammar->rule);
//    document->grammar = grammar;

//    document_parse_set_head(document, 0);
//}

//
// Parsing
//

//void
//document_set_grammar(Document *document, DocumentGrammar *grammar)
//{
//    DocumentGrammar *previous = document->grammar;
//    document->grammar = grammar;
//    if (grammar) {
//        ref(grammar);
//    }
//
//    DocumentSession *session;
//    for (memp i = 0; i < document->sessions.count; i++) {
//        session = document->views[i];
//        // TODO: Update the view's grammar here. (status/cache invalidation)
//    }
//
//    if (previous) {
//        unref(previous);
//    }
//}

//
//
//

hale_internal void
check_edit(DocumentEdit *edit)
{
    hale_assert(edit);
    hale_assert(edit->view || edit->undo || edit->internal);
}

hale_internal void
write_undo(DocumentEdit *edit, s32 type, memi offset, memi length)
{
#if 0
    if (edit->undo) {
        return;
    }

    QString text;

    edit->document->undo->write(type, [&] (QDataStream &s) {
//        Vector<ch> text;
//        vector_resize(&text, length);
//        document_text(edit->document, offset, length, vector_ptr(&text), length);

        s << offset;
        s << length;

        text.resize((int)length);
        document_text(edit->document, offset, length, (ch*)text.data(), length);

        s << text;
    });
#endif
}

#if 0
hale_internal void
read_undo(DocumentEdit *edit, QDataStream &s, s32 event, b32 redo)
{
    int offset;
    int length;
    QString text;

    s >> offset;
    s >> length;

    auto position(offset_to_position(edit->document, offset));

    if (redo) {
        switch (event) {
        case Document::UndoEvent_Insert:
            event = Document::UndoEvent_Remove;
            break;
        case Document::UndoEvent_Remove:
            event = Document::UndoEvent_Insert;
            break;
        }
    }

    switch (event)
    {
    case Document::UndoEvent_Insert:
        document_abner(edit, position, length);
        break;
    case Document::UndoEvent_Remove:
        s >> text;
        document_insert(edit, position, (ch*)text.data(), text.length());
        break;
    }
}
#endif

hale_internal memi
convert_and_insert(Document *document, memi offset, ch *text, memi text_length, Vector<memi> *offsets)
{
    ch *it = text;
    ch *it_text = it;
    ch *end = text + text_length;

    memi len;
    memi off = offset;
    for (;;)
    {
        auto le = string_find_next_line_ending(&it, end);
        len = (it-it_text) - le.length;
        _buffer_insert(&document->buffer,
                       off,
                       it_text,
                       len);

        off += len;

        if (le.type != LineEnding_Unknown) {
            // Insert LF new line.
            _buffer_insert(&document->buffer,
                           off,
                           (ch*)L"\n", 1);
            off += 1;

            vector_push(offsets, off);
        } else {
            break;
        }
        it_text = it;
    }

    return off - offset;
}

//
// Blocks
//

//Document::Block *
//document_get_block_info(Document *document, memi block)
//{
//    Document::Block *block = document->blocks_info[block];
//    if (block == NULL)
//    {
//        // TODO: Rework block_info to a more efficient structure.
//        block = new Document::Block;
//        block->flags = 0;
//        document->block_info[block_index] = block;
//    }
//    return block;
//}

memi
document_block_at(Document *document, memi offset, memi search_begin, memi search_end)
{
    hale_assert(search_begin <= search_end);
    hale_assert(search_begin < vector_count(document->blocks));
    hale_assert(search_end < vector_count(document->blocks));
    hale_assert(offset <= _buffer_length(document->buffer));
    if (offset >= vector_last(&document->blocks).end) {
        return vector_count(document->blocks) - 1;
    }

//    memi search_begin = search_begin;
//    memi search_end = search_end < 0 ? document->blocks.count - 1 : search_end;


    for (;;)
    {
        switch (search_end - search_begin)
        {
        case 0:
            if (document->blocks[search_begin].end <= offset)
                return search_begin + 1;
            else
                return search_begin;
        case 1:
            if (document->blocks[search_begin].end <= offset)
            {
                if(document->blocks[search_end].end <= offset)
                    return search_end + 1;
                else
                    return search_end;
            }
            else
                return search_begin;
        default:
            memi pivot = (search_end + search_begin) / 2;
            memi value = document->blocks[pivot].end;
            if (value == offset) {
                return pivot + 1;
            } else if (value < offset) {
                search_begin = pivot + 1;
            } else {
                search_end = pivot - 1;
            }

            break;
        }
    }
}

//
//
//

void
document_insert(DocumentEdit *edit, DocumentPosition position, ch *text, memi text_length)
{
    check_edit(edit);

    // TODO(cohen) Protect insertText in release.

    // TODO(cohen) If a new line is inserted at the beginning of a line,
    // that line will remain untouched (by changed or added callbacks).
    // We need to take care to not let line changes affect breakpoints, line data, etc.
    // That will also make it easier to reuse data in editor.

    auto document = edit->document;

    hale_assert(is_valid_position(document, position));

    // TODO: emit beforeTextChanged();

    edit->type = DocumentEdit::Insert;
    // edit->undo_head = document->undo->writingHead();
    edit->offset_begin = position_to_offset(document, position);

    memi old_document_end = _buffer_length(document->buffer);

    Vector<memi> offsets;
    vector_init(&offsets);
    text_length = convert_and_insert(document, edit->offset_begin, text, text_length, &offsets);

    edit->offset_end = edit->offset_begin + text_length;

    edit->blocks_changed_count = vector_count(offsets);

    if (edit->blocks_changed_count == 0) {
        edit->block_changed = position.block;
        edit->blocks_changed_at = position.block;
    } else if (position.position == 0) {
        // Inserting at the beginning of a block.

        if (edit->offset_begin == old_document_end) {
            // If we're inserting at the end of the document to an empty block.
            edit->block_changed = position.block;
            edit->blocks_changed_at = position.block + 1;
        } else {
            // Anywhere at the block begin.
            edit->block_changed = position.block + vector_count(offsets);
            edit->blocks_changed_at = position.block;
        }
    } else {
        // Inside the block
        edit->block_changed = position.block;
        edit->blocks_changed_at = position.block + 1;
    }

    //
    // Blocks
    //

    // Invalidate the changed block.

    document->blocks[edit->block_changed].flags = Document::Block::F_TextChanged;

    // TODO: Invalidate the block also for the sessions.

    // Insert new blocks.

    if (edit->blocks_changed_count > 0)
    {
        Document::Block block;
        block.end = 0;
        block.flags = Document::Block::F_TextChanged;
        // block.meta = NULL;

        // We're inserting this *before* the position.block.
        vector_insert(&document->blocks,
                      position.block,
                      vector_count(offsets),
                      block);

        // TODO: Insert the blocks also to the sessions.
        for (memi i = 0; i != edit->blocks_changed_count; i++) {
            document->blocks[position.block + i].end = offsets[i];
        }
    }

    // Shift the offsets.
    for (memi i = position.block + edit->blocks_changed_count;
         i != vector_count(document->blocks);
         i++)
    {
        document->blocks[i].end += text_length;
    }

    edit->pos_begin = position;
    edit->pos_end = position_plus_offset(document, edit->pos_begin, text_length);


    // Invalidate block
    _notify_views_about_insert(edit);

    //
    // Undo
    //

    // write_undo(edit, Document::UndoEvent_Insert, edit->offset_begin, text_length);

    //
    // Parser
    //

    // Move the parser head
    document_parse_set_head(document, edit->pos_begin.block);

    // TODO(cohen): If the time distance between caret types is small, do not parse immediately.
    // document_parse_immediate(document, edit->begin.block, edit->end.block);

    // TODO: emit textChanged(&edit);
}

void
document_abner(DocumentEdit *edit, DocumentPosition position, memi length)
{
    hale_unused(edit);
    hale_unused(position);
    hale_unused(length);
    hale_not_implemented;
}

void
document_text(Document *document, memi offset, memi length, ch *buffer, memi buffer_length)
{
    hale_assert(offset + length <= _buffer_length(document->buffer));
    hale_assert(buffer_length >= length);

    if (length != 0) {
        _buffer_text(&document->buffer, offset, length, buffer);
    }
}

//
// Undo
//

void
document_undo(Document *document)
{
    hale_unused(document);
    hale_not_implemented;
}

void
document_redo(Document *document)
{
    hale_unused(document);
    hale_not_implemented;
}

} // namespace hale
