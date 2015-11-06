#if HALE_INCLUDES
#include "hale_macros.h"
#include "hale_document.h"
#endif

/*
 * - When a block is changed, we call `set_head` to set the block at which the parsing is started.
 * - `set_head` takes the stack of the previous block, and stores it as `work_parser_stack`.
 * - `work_parser_stack` is then sent to the parser to tell it the state to start parsing.
 * - when parser finishes the parsing of several blocks, the
 */

namespace hale {

hale_internal inline void
_copy_stack(Memory<ParserStackItem> *destination, Memory<ParserStackItem> *source)
{
    destination->count = 0;
    destination->push(source->count, 0);
    memory_copy(destination->ptr, source->ptr, source->count);
}

void
document_parser_set(Document *document, DocumentParser *parser)
{
    hale_assert_input(parser);

    document->parser = *parser;
    document->parser.init(&document->parser_state);
    document_parser_set_head(document, 0);
}

b32
document_parser_set_head(Document *document, memi head)
{
    b32 ret = 0;
    if (document->parser.parse != NULL)
    {
        // TODO(cohen): Should we also check whether the parser_head isn't already on 0?
        if (head == 0)
        {
			document->_parser_head = head;
            // We're parsing from the top, so we reset the work stack.
            document->parser_work_stack.count = 0;

            ret = 1;
        }
        else if (head < document->_parser_head)
        {
            // The start has been set to be before where the parser_head currently is.
            // So we will jump there.
            document->_parser_head = head;
            // We will copy the stack of the block at head - 1 to the work stack.
            _copy_stack(&document->parser_work_stack, &document->blocks[head - 1].stack_end);

            ret = 1;
        }
        notify_document_needs_parsing(document);
    }
    return ret;
}

// [root  ] ..... [stack1]
// [stack1] ..... [stack2]
// [stack2] .....

// [root  ] ..... [stack1]
// [stack1] ..... [stack2']
// [stack2] .....

memi
document_parse(Document *document, r64 max_time)
{
    HALE_PERFORMANCE_TIMER(document_parse);

    hale_assert(document->parser.parse);

    memi head = document->_parser_head;
    Document::Block *block;

    r64 time = platform.read_time_counter();

    Memory<ch16> text = {};

    memi length;
    memi parsed = 0;

    // TODO:

    while(head != vector_count(document->blocks))
    {
        block = &document->blocks[head];
        if ((block->flags & DocumentBlockFlags_TextValid) == 0 ||
            !equal(&block->stack_begin, &document->parser_work_stack))
        {
            length = document_block_length(document, head);
            if (length > text.capacity()) {
                text.reallocate(length);
            }
            text.count = length;

            document_text(document,
                          document_block_begin(document, head),
                          document_block_length(document, head),
                          text.ptr,
                          text.count);

            if (head != 0) {
                // Not needed for the first block.
                _copy_stack(&block->stack_begin, &document->parser_work_stack);
            }
            // We always parse over the `work_stack`, and copy it out
            // to the block's internal storage when the parsing of the block
            // has finished.
            document->parser.parse(&document->parser_state,
                                   text.ptr,
                                   text.ptr + text.count,
                                   &block->tokens,
                                   &document->parser_work_stack);

            _copy_stack(&block->stack_end, &document->parser_work_stack);

			print_parser_stack(hale_ch("B"), &block->stack_begin, head);
            // print_parser_tokens(&block->tokens, head);
            print_parser_stack(hale_ch("E"), &block->stack_end, head);

            block->flags |= DocumentBlockFlags_TextValid;
            block->view_flags = 0;
            parsed++;
        }

        head++;

#if 1
        if (((platform.read_time_counter() - time) > max_time)) {
            break;
        }
#endif
    }

    if (text.ptr) {
        text.deallocate();
    }

//    for (memi i = 0; i < document->views_count; ++i) {
//        document_layout_on_format(document->views[i].layout, document->_parser_head, head);
//    }

    document->_parser_head = head;

    return parsed;
}

void
print_parser_stack(const ch *name, Memory<ParserStackItem> *stack, memi block_index)
{
//    Print print;
//    print << "Stack" << name << block_index << ":";
//    for (memi i = 0; i != stack->count; ++i) {
//        print << "[" << stack->e[i].token_id << "]";
////              <<   "ix:" << stack->e[i].token_ix
////              << "\n";
//    }
}

void
print_parser_tokens(Memory<Token> *tokens, memi block_index)
{
//    Print print;
//    print << "Tokens [" << block_index << "]";
//    for (memi i = 0; i != tokens->count; ++i) {
//        Token *token = &tokens->e[i];
//        print << token->id << "<"
//              << token->begin << "-"
//              << token->end << ">"
//              ;
//    }
}


}
