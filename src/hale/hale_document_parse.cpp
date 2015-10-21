#include "hale_document.h"

#include <QElapsedTimer>

namespace hale {

/*

    # Parsing

    Parser is block-based and cooperative.

    Parser walks block-by-block from it's head position. Block is reparsed when:

    - Block's text has been changed since the last visit.
    - Block's starting stack is different from previous block's ending stack.

    When the buffer is changed, parser's head position is changed only if the
    change happened before it's head position.

    Stack is kept with the block meta-data.

    - It is the stack that is used when the corresponding block is being reparsed.
    - Parser keeps it's own stack in it's state.

    When the head is changed, the parser's stack is set to the stack of the block
    pointed to by the head.

    When the parser advances to next block, it's stack is compared to the block's
    stack. If those two do not match, the block is reparsed. This check is
    skipped in case the block's text has changed since the last visit.

    # Future

    A) Parser can stop at an arbitrary position within the block, when it's resumed,
    it will use it's stored stack to continue from that position.

    <...> denotes what part of the line is parsed

    1 <Ut facilisis at massa a placerat.>
    [stack #1]
    2 <Lorem ipsum dolor> sit amet.
    [stack #2, partial]
    3 Pellentesque ornare nibh ac urna varius venenatis.

    4 Maecenas vel orci vel augue mollis rutrum.


    - #2 is being parsed.
    - A change to #1 is made, so the head is moved to it.
        - #2 is left in partial state
    - #1 is parsed fully.
        - continue to #2
    - #2 stack #1 matches the parser stack,
        - we need to check if the #2 was fully parsed at this point!
        - check if we can continue to #3
            - if the stack #2 is partial, then we can't continue to #3 and must finish #2 first.
        - continue to #3
    -

    B) Custom parser can have it's own stack structure.

    If the stack is just a uintptr reference to the parser's own table, it can be done easily, as
    the parser is responsible for maintaining it's own table.

    A common API has to be defined that allows for combining various custom parsers for nesting.

*/


//
//
//

hale_internal void
notify_format_changed(Document *document, memi begin, memi end)
{
    for (memi i = 0; i < document->sessions_count; i++) {
        // document->sessions[i]
        // TODO: invalidate text layout in session in range [begin, end]
    }
}

b32
document_can_parse(Document *document)
{
    return DOCUMENT_PARSER_WORKING(document) && document->grammar;
}

void
document_parse_set_head(Document *document, memi head)
{
    if (document->parser_head > head) {
        document->parser_head = head;
        if (head == 0) {
            document->parser_stack.clear();
            document->parser_stack.push_back({document->grammar->rule});
//            vector_clear(&document->parser_state.stack);
//            vector_push(&document->parser_state.stack, {document->grammar->rule});
        } else {
            document->parser_stack = document->blocks[head - 1].stack;
        }
        // TODO: Try to parse immediately?
    }

    if (!DOCUMENT_PARSER_WORKING(document)) {
        document->parser_status |= Document::ParserStatus_Working;
        // TODO: Wake up the arena?
    }
}

b32
document_parse(Document *document)
{
    memi head = document->parser_head;
    Parser::Stack &stack = document->parser_stack;

    QString block_text;
    Document::Block *block;
    for (; head < vector_count(document->blocks); head++)
    {
        block = &document->blocks[head];
        if (block->flags & Document::Block::F_TextChanged ||
            block->stack != stack)
        {
            block_text.resize((int)document_block_length(document, head));
            document_text(document,
                          document_block_begin(document, head),
                          document_block_length(document, head),
                          (ch*)&block_text[0],
                          block_text.length());

            block->stack = stack;
            block->tokens.clear();
            document->parser->parse(block_text, &stack, &block->tokens);
            block->flags &= ~Document::Block::F_TextChanged;
        } else {
            // No need to parse, just take the stack and continue to next block.
            stack = block->stack;
        }
    }

    // TODO: Do this in-place instead of calling a function.
    notify_format_changed(document, document->parser_head, head);

    if (document->parser_head == vector_count(document->blocks) - 1) {
        document->parser_status = Document::ParserStatus_Done;
        return false;
    } else {
        document->parser_status = Document::ParserStatus_Working;
        return true;
    }
}

void
document_parse(DocumentArena *arena)
{
    memi pending = 0;

    Document *document;
    for (memi i = 0; i < vector_count(arena->documents); i++) {
        document = arena->documents[i];
        if (DOCUMENT_PARSER_WORKING(document) && !DOCUMENT_PARSER_PAUSED(document)) {
            if (document_parse(document)) {
                pending++;
            }
        }
    }

    if (pending) {
        // TODO: defer()
        hale_not_implemented;
    }
}

// TODO(cohen): We must parse the line if:
// - F_TokensInvalidated is set -or-
// - The end stack of the previous line is different with beginning stack of this line.
//   - This can be possibly check after we parse the previous line.
//   - We can simply check if it's ending stack has changed and then force the parse of the next line.
//   - Be careful with the first line parsed.

//memi
//document_parse(Document *document, memi block_first, memi block_last)
//{
//    hale_assert(document->grammar);



//    memi it = block_first;
//    Document::Block *block_previous;
//    Document::Block *block = document_get_block_info(document, it);

//    vector_clear(&block->stack);
//    if (it == 0) {
//        vector_push(&block->stack, {document->grammar->rule});
//    } else {
////        block_previous = document_get_block_info(document, it - 1);
////        vector_push(&block->stack, &block_previous->stack);
////        hale_assert(!block->stack.empty());

//    }

//    QElapsedTimer timer;
//    timer.start();

//    // TODO: Unchain the parser to accept pure ch.
//    QString block_text;

//    for (;;)
//    {
//        block_text.resize(document_block_length(document, it));
//        document_text(document,
//                      document_block_begin(document, it),
//                      document_block_length(document, it),
//                      (ch*)&block_text[0],
//                      block_text.length());

//        block->tokens.clear();
//        document->parser.parse(block_text, &block->stack, &block->tokens);
//        block->flags |= Document::Block::F_FormatsInvalidated;
//        block->flags &= ~Document::Block::F_TextChanged;

//        it++;
//        if (it > block_last || timer.elapsed() > MAX_MS_PER_PARTIAL_PARSE) {
//            break;
//        }

//        block_previous = block;
//        block = document_get_block_info(document, it);
//        block->stack = block_previous->stack;
//    }

//    document->parser_state.head = it;
//    return it;
//}

//void
//document_parse_suspend(Document *document)
//{
//    if (document->parser_state.status == DocumentParserState::ParserStatus_Working) {
//        document->parsing = DocumentParserState::ParserStatus_WorkingPaused;
//        // TODO: Wake up the arena
//        // stopPartialParse();
//    }
//}

//void
//document_parse_resume(Document *document)
//{
//    if (document->parser_state.status == DocumentParserState::ParserStatus_WorkingPaused) {
//        document->parsing = DocumentParserState::ParserStatus_Working;
//    }
//}

}
