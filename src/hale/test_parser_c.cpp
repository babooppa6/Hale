#ifdef HALE_INCLUDES
#include "hale_macros.h"
#include "hale_memory.h"
#include "hale_document_parser.h"
#include "hale_parser_c.h"
#endif

namespace hale {

hale_internal void
_parse(ch *begin)
{
    ParserC parser;
    ParserState state;
    parser.init(&state);

    Memory<ParserStackItem> stack = {};
    Memory<Token> tokens = {};
//    parser.reset(&state, &tokens, &stack);

    ch *end = begin + string_length(begin);
    parser.parse(&state, begin, end, &tokens, &stack);
    print_parser_stack(&stack, 0);
    print_parser_tokens(&tokens, 0);
}

// StackLines
// [0] 0 1 2
// [1] 0 1 3
//     ^

void test_parser_c()
{
    _parse(hale_ch("/* abc"));
}

}
