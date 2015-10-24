#if 0
#include "hale.h"

// https://github.com/zserge/jsmn

#include "oniguruma.h"

namespace hale {

struct State;

#define HALE_PARSE(N) void N(State *state)
typedef HALE_PARSE(Parse);

struct StackItem
{
    Parse *parse;
};

struct Token
{
    // TODO: Token names can actually be stored in a static table, as we usually already know them at compile time.
    StaticMemory<ch16, 32> name;
    memi begin;
    memi end;
};

struct Node
{
    memi ix;
    const char *name;
};

struct State
{
    ch16 *it;
    Memory<StackItem, MallocAllocator> stack;
    Memory<Token, MallocAllocator> tokens;

    // TODO: Do the AST later.
//    Memory<Node> node_stack;
//    // TODO: This should be stored on the document.
//    Memory<Token> node_table;
};

//
// Tokens
//

memi
token(State *S, const char *name)
{
    // Find whether table has a `name` recorded for top(S)->parent.
}

void
token_add(State *S, memi ix, memi begin, memi end)
{
    auto token = memory_push(S->tokens, 1, 16);
//    token->ix = ix;
//    token->begin = begin;
//    token->end = end;
}

//
// Stack
//

inline void
stack_push(State *S, Parse *parse)
{
    S->it += length;
    auto item = memory_insert(S->stack, S->stack.count, 1);
    item->parse = parse;
    // TODO: Clear cache.
}

inline Parse *
stack_pop(State *S)
{
    return memory_pop(&S->stack);
}

inline Parse *
stack_top(State *S)
{
    return S->stack.e[S->stack.count - 1];
}

//
// Input
//

b32
input_match(State *S, const ch16 *needle)
{
    // TODO: Use boyer-moore search here, cache the forward match.
}

b32
input_peek(State *S, regex_t* re)
{

}

void
input_advance(State *S, memi amount)
{

}

//
//
//

hale_internal void
parse_c_block_comment(State *S)
{

}

hale_internal void
parse_c_string_double_quoted(State *S)
{
    if (input_match(S, '"')) {
        token_add(S, token(S, "string.quoted.double.end"), 0, 1);
        stack_pop(S);
    } else {
        input_advance(S, 1);
    }
}

hale_internal void
parse_c(State *S)
{
    if (input_match(S, "/*")) {
        token_add(S, token(S, "comment.block.begin"), 0, 2);
        stack_push(S, parse_c_block_comment);
    } else if (input_match(S, '"')) {
        token_add(S, token(S, "string.quoted.double.begin"), 0, 1);
        stack_push(S, parse_c_string_double_quoted);
    } else {
        input_advance(S, 1);
    }
}

void
parse_init(Memory<u8> *storage)
{
    // memory_reallocate(storage, );
}

void
parse(Memory<u8> *storage)
{
    // memory_reallocate(storage, );
}

}
#endif
