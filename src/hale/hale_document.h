#ifndef HALE_DOCUMENT_H
#define HALE_DOCUMENT_H

#include "hale.h"
#include "hale_gap_buffer.h"
#include "hale_fixed_gap_buffer.h"
#include "hale_document_types.h"
#include "hale_document_view.h"
// #include "hale_ui.h"

#include "undostream.h"

#include <QTextLayout>
#include "parser.h"

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

#define DOCUMENT_PARSER_DONE(document)    (document->parser_status == Document::ParserStatus_Done)
#define DOCUMENT_PARSER_WORKING(document) (document->parser_status & Document::ParserStatus_Working)
#define DOCUMENT_PARSER_PAUSED(document)  (document->parser_status & Document::ParserStatus_Paused)

struct Document
{
    static const int MAX_INDENTATION_SIZE = 16;
    static const int MAX_MS_PER_INCREMENTAL_PARSE = 5;

    enum UndoEvent
    {
        UndoEvent_Insert = UndoStream::EventType_User,
        UndoEvent_Remove,
    };

    struct Block
    {
        enum Flags
        {
            F_TextChanged = 0x01,
            F_FormatsInvalidated = 0x02
        };

        memi end;
        u32 flags;

        // TODO: Use gap_buffer for tokens, as it's a structure similar to text.
        Parser::Tokens tokens;
        // TODO: We probably won't have to store whole stack per line.
        // The tokens will be in a tree structure, so we only need to know the token.
        Parser::Stack stack;

        // Formats are kept here in case there are multiple sessions for the document.
        QList<QTextLayout::FormatRange> formats;
    };

    DocumentArena *arena;
    Vector<ch16> path;
    IndentationMode indentation_mode;
    memi indentation_size;
    Vector<Block> blocks;

    DocumentView views[16];
    memi views_count;

    //
    // Parser
    //

    QSharedPointer<Grammar> grammar;
    enum ParserStatus
    {
        ParserStatus_Done = 0x0,
        ParserStatus_Working = 0x02,    // The parser is parsing (but can be paused).
        ParserStatus_Paused = 0x04,     // Happens when the document is not active.
    };

    u32 parser_status;
    Parser::Stack parser_stack;
    memi parser_head;
    Parser *parser;

    FixedGapArena buffer;
    UndoStream *undo;
};

//template<typename BufferT, typename UndoT>
//struct DocumentT
//{
//    Document common;
//    BufferT buffer;
//    UndoT *undo;

//    operator Document*() {
//        return &common;
//    }
//};

//typedef DocumentT<GapBuffer, UndoStream> DocumentGap;
//typedef DocumentT<FixedGapArena, UndoStream> DocumentFGA;


//
// Arena
//

//union Document {
//    DocumentGap document_gap;
//    DocumentFGA document_fga;
//};

struct DocumentArena
{
    // TODO: Change from ptr to value.
    Vector<Document*> documents;
    Vector<DocumentView*> sessions;
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

    /// Undo head.
    memi undo_head;
};

//
// Blocks
//

inline memi
document_block_begin(Document *document, memi block)
{
    hale_assert(block < vector_count(document->blocks))
    return block == 0 ? 0 : document->blocks[block - 1].end;
}

inline memi
document_block_end(Document *document, memi block)
{
    hale_assert(block < vector_count(document->blocks))
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
void document_insert(DocumentEdit *edit, DocumentPosition position, ch *text, memi text_length);
void document_insert(DocumentEdit *edit, DocumentPosition *begin, DocumentPosition *end, ch *text, memi text_length);
inline void
document_insert(DocumentEdit *edit, DocumentPosition position, const char *text) {
    QString string(text);
    document_insert(edit, position, (ch*)string.data(), string.length());
}

inline void
document_append(DocumentEdit *edit, ch *text, memi text_length) {
    DocumentPosition position;
    position.block = vector_count(&edit->document->blocks) - 1;
    position.position = document_block_length(edit->document, position.block);
    document_insert(edit, position, text, text_length);
}

void document_abner(DocumentEdit *edit, DocumentPosition position, memi length);

void document_text(Document *document, memi offset, memi length, ch *buffer, memi buffer_length);

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

b32
document_parse(Document *document);
void
document_parse_set_head(Document *document, memi head);

} // namespace hale

#endif // HALE_DOCUMENT_H
