#ifndef HALE_DOCUMENT_H
#define HALE_DOCUMENT_H

#if HALE_INCLUDES
#include "hale_macros.h"
#include "hale_types.h"
#include "hale_vector.h"
#include "hale_undo.h"
#include "hale_gap_buffer.h"
#include "hale_fixed_gap_buffer.h"
#include "hale_document_types.h"
#include "hale_document_view.h"
#include "hale_document_parser.h"
#endif

namespace hale {

#define HALE_DOCUMENT_FORMAT_CHANGED(name) void name(memi begin, memi end)
typedef HALE_DOCUMENT_FORMAT_CHANGED(hale_document_format_changed);

//
// Forwad declarations.
//

struct DocumentView;
struct DocumentEdit;
struct DocumentPosition;
struct DocumentRange;
struct DocumentCursor;
struct DocumentRange;

struct DocumentArena;
struct Document;

//
// Document
//

struct DocumentDescriptor
{
    Vector<u16> path;
};

// Only temporary.
struct App;


enum DocumentBlockFlags
{
    DocumentBlockFlags_TextValid = 0x01,
    DocumentBlockFlags_TokensValid = 0x02
};

#define HALE_DOCUMENT_MAX_VIEWS 16
#define HALE_DOCUMENT_VIEW_LAYOUT_INVALID       ((u64)-1)
// 0101 -- 5
#define HALE_DOCUMENT_BLOCK_FLAGS_TEXT_VALID    ((u64)0x5555555555555555LL)
// 1010 -- A
#define HALE_DOCUMENT_BLOCK_FLAGS_TOKENS_VALID  ((u64)0xAAAAaaaaAAAAaaaaLL)


typedef u64 DocumentViewsFlags;

struct Document
{
    static const int MAX_INDENTATION_SIZE = 16;

    // Tempoary.
    App *app;

    DocumentArena *arena;
    Vector<ch16> path;
    IndentationMode indentation_mode;
    memi indentation_size;

    // A few quarters can be kept in a good condition,
    // the others might be just partially valid / setup.
    // Proposal. Not yet used in the code.
    // TODO: Probably implement as struct of arrays?
    struct Quarter
    {
        // Base offset for the first block.
        memi offset_base;
        // End offsets relative to `offset_base`.
        memi offset_ends[256];
        // Text and Tokens invalidation info for each view.
        DocumentViewsFlags view_flags;
        // Memory for parser meta information.
        Memory<u8> parser_meta;
    };

    struct Block
    {
        memi end;
        // We're storing 2 bits per view here.
        // Lower bit signals that the text has changed for the block.
        // Higher bit signals that the tokens has changed for the block.
        DocumentViewsFlags view_flags;
        // TODO: Get rid of these, and reserve one slot in view_flags instead.
        u32 flags;
        Memory<ParserStackItem> stack_end;
        Memory<ParserStackItem> stack_begin;
        Memory<Token> tokens;
    };

    Vector<Block> blocks;

    //
    // Views
    //

    DocumentView views[HALE_DOCUMENT_MAX_VIEWS];
    // Keeps a single flag per view that tells view it's layout has to be validated.
    DocumentViewsFlags view_flags;
    memi views_count;

    //
    // Parser
    //

    memi _parser_head;
    ParserState parser_state;
    Memory<ParserStackItem> parser_work_stack;
    // Memory<ParserStackItem> parser_root_stack;
    DocumentParser parser;

    //
    // Buffer
    //

    FixedGapArena buffer;

    //
    // Undo
    //

    enum UndoEvent
    {
        UndoEvent_Insert = 0,
        UndoEvent_Remove = 1,
    };
    Undo undo;
};

inline memi
document_view_index(DocumentView *view)
{
    return view - view->document->views;
}

inline u8
document_read_block_view_flags(DocumentView *view,
                               DocumentViewsFlags flags)
{
    hale_assert_debug(view);
    hale_assert_debug(view->document);
    // View must be within these boundaries.
    hale_assert_debug(view >= view->document->views);
    hale_assert_debug(view <  view->document->views + HALE_DOCUMENT_MAX_VIEWS);

    memi index = document_view_index(view) << 1;
    return (flags >> index) & 3;
}

inline DocumentViewsFlags
document_write_block_view_flags(DocumentView *view,
                                DocumentViewsFlags block_flags,
                                u8 view_block_flags)
{
    hale_assert_debug(view);
    hale_assert_debug(view->document);
    // View must be within these boundaries.
    hale_assert_debug(view >= view->document->views);
    hale_assert_debug(view <  view->document->views + HALE_DOCUMENT_MAX_VIEWS);
    hale_assert_debug(view_block_flags <= 3);

    memi index = document_view_index(view) << 1;
    block_flags &= ~(3 << index);
    block_flags |= ((u64)(view_block_flags & 3)) << index;
    return block_flags;
}

//
// Arena
//

struct DocumentArena
{
    // TODO: Change from ptr to value.
    Vector<Document*> documents;
    Vector<DocumentView*> views;
};

err document_create(DocumentArena *arena, Document **document);
err document_load(Document *document, const ch8 *path);
err document_save(Document *document, const ch8 *path);
err document_abner(Document *document, const ch8 *path);

struct DocumentEdit
{
    ~DocumentEdit();

    /// Target document
    Document *document;
    /// View that called the edit. Can be NULL.
    DocumentView *view;
    /// Set to true if this operation is undo.
    b32 undo;
    /// Set to true if this operation comes from other place than undo or session.
    b32 internal;

    enum EditType
    {
        Insert,
        Remove,
    };

    EditType type;

    /// Index of the block that has changed it's text.
    memi block_changed;
    /// Index at which the blocks were inserted or removed
    memi blocks_changed_at;
    /// Number of blocks inserted or removed.
    /// Note that the insert/remove block is in pos_begin.block.
    memi blocks_changed_count;

    /// Document position where the change began.
    DocumentPosition pos_begin;
    /// Document position where the change end.
    /// NOTE: END position is important when we are removing text.
    DocumentPosition pos_end;

    /// Offset where the change began.
    memi offset_begin;
    /// Offset where the change ended.
    memi offset_end;

    /// Length of the text changed.
    memi length;

    /// Undo head.
    memi undo_head;
};

//
// Blocks
//

inline memi
document_block_begin(Document *document, memi block)
{
    hale_assert(block < vector_count(document->blocks));
    return block == 0 ? 0 : document->blocks[block - 1].end;
}

inline memi
document_block_end(Document *document, memi block)
{
    hale_assert(block < vector_count(document->blocks));
    return document->blocks[block].end;
}

inline memi
document_block_length(Document *document, memi block)
{
    hale_assert(block < vector_count(document->blocks));
    memi length = document_block_end(document, block) - document_block_begin(document, block);
    return length;
}

inline memi
document_block_length_without_le(Document *document, memi block)
{
    memi length = document_block_length(document, block);
    if (block < vector_count(document->blocks) - 1) {
        return length - 1;
    }
    return length;
}

memi
document_block_at(Document *document, memi offset, memi search_begin, memi search_end);

inline memi
document_block_at(Document *document, memi offset, memi search_begin) {
    return document_block_at(document, offset, search_begin, vector_count(document->blocks) - 1);
}

inline memi
document_block_at(Document *document, memi offset) {
    return document_block_at(document, offset, 0, vector_count(document->blocks) - 1);
}

inline memi
document_length(Document *document) {
    return document->buffer.size / sizeof(ch);
}

Document::Block *
document_get_block_info(Document *document, memi block);

//
// Positions
//

inline b32
is_valid_position(Document *document, DocumentPosition position)
{
    if (position.block >= 0 && position.block < vector_count(document->blocks)) {
        memi block_length = document_block_length_without_le(document, position.block);
        return position.position >= 0 && position.position <= block_length;
    }
    return false;
}

inline b32
is_valid_offset(Document *document, memi offset)
{
    if (offset < 0) {
        // TODO: Diagnostics qWarning() << __FUNCTION__ << "Offset" << offset << "< 0";
        return false;
    } else if (offset > document_length(document)) {
        // TODO: Diagnostics qWarning() << __FUNCTION__ << "Offset" << offset << ">" << length();
        return false;
    }
    return true; // offset >= 0 && offset <= length();
}

inline memi
position_to_offset(Document *document, DocumentPosition position)
{
    hale_assert(is_valid_position(document, position));
    memi offset = document_block_begin(document, position.block) + position.position;
    return offset;
}

inline DocumentPosition
offset_to_position(Document *document, memi offset)
{
    hale_assert(is_valid_offset(document, offset));
    DocumentPosition position;
    position.block = document_block_at(document, offset);
    position.position = offset - document_block_begin(document, position.block);
    return position;
}

inline sptr
position_minus_position(Document *document, const DocumentPosition &a, const DocumentPosition &b)
{
    if (a.block == b.block) {
        return a.position - b.position;
    }
    return position_to_offset(document, a) - position_to_offset(document, b);
}

inline DocumentPosition
position_plus_offset(Document *document, const DocumentPosition &a, sptr n)
{
    // I'm doing this instead of calling blockPositionToOffset, as the a is allowed to be virtual at this point.
    // Calling offsetToBlockPosition will do the validation afterwards.
    memi o = document_block_begin(document, a.block) + a.position + n;
    // int o = blockPositionToOffset(a) + n;
    return offset_to_position(document, o);
}

//
//
//

void document_init(Document *document);
void document_release(Document *document);

void document_set(DocumentEdit *edit, ch *text, memi length);

//
// Insert
//

void document_insert(DocumentEdit *edit, DocumentPosition position, ch *text, memi text_length);
void document_insert(DocumentEdit *edit, memi offset, ch *text, memi text_length)
{
    DocumentPosition position = offset_to_position(edit->document, offset);
    document_insert(edit, position, text, text_length);
}

// TODO: Inserts text at multiple positions.
//       Also supersedes the default single-position insert.
void document_insert(DocumentEdit *edit,
                     DocumentPosition *begin,
                     DocumentPosition *end,
                     ch *text,
                     memi text_length);

inline void
document_append(DocumentEdit *edit, ch *text, memi text_length)
{
    DocumentPosition position;
    position.block = vector_count(&edit->document->blocks) - 1;
    position.position = document_block_length(edit->document, position.block);
    document_insert(edit, position, text, text_length);
}

//
// Abner
//

void document_abner(DocumentEdit *edit, DocumentPosition begin, DocumentPosition end);

inline void
document_abner(DocumentEdit *edit, memi offset, memi length)
{
    DocumentPosition begin = offset_to_position(edit->document, offset);
    DocumentPosition end   = position_plus_offset(edit->document, begin, length);
    document_abner(edit, begin, end);
}

//
// Retrieval
//

Memory<Token> *document_tokens(Document *document, memi block_index);
void document_text(Document *document, memi offset, memi length, ch *buffer, memi buffer_length);

//
// Undo
//

void document_undo(Document *document);
void document_redo(Document *document);

//
// Edit
//

void document_edit(DocumentEdit *edit, Document *document, DocumentView *session);
void document_commit(DocumentEdit *edit);

//
// Parser
//

void document_parser_set(Document *document, DocumentParser *parser);
b32  document_parser_set_head(Document *document, memi head);
memi document_parse(Document *document, r64 max_time);

inline b32
document_parser_is_working(Document *document) {
    return document->parser.parse &&
           document->_parser_head < vector_count(document->blocks);
}

//
// External notifications.
//

void notify_document_needs_parsing(Document *document);
} // namespace hale

#endif // HALE_DOCUMENT_H
