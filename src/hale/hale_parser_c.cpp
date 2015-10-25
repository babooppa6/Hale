#include "hale.h"
#include "hale_parser_c.h"

// https://github.com/zserge/jsmn

// #include "oniguruma.h"

namespace hale {

enum TokenId : memi
{
    Root,

    CommentBlockBegin,
    CommentBlockEnd,
    CommentBlock,

    StringQuotedDoubleBegin,
    StringQuotedDoubleEnd,
    StringQuotedDouble,
};

struct State;

#define HALE_PARSE(N) b32 N(State *state, memi ix)
typedef HALE_PARSE(Parse);

struct StackItem
{
    Parse *parse;
    memi state_ix;
    memi state_p;
};

struct Token
{
    memi ix;
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
    ch16 *it_;
    Memory<StackItem> stack;
    Memory<Token> *tokens;

    // TODO: Do the AST later.
//    Memory<Node> node_stack;
//    // TODO: This should be stored on the document.
//    Memory<Token> node_table;
};

//
// Tokens
//

void
token_add(State *S, memi ix, memi begin, memi end)
{
    auto token = memory_push(S->tokens, 1, 16);
//    token->ix = ix;
//    token->begin = begin;
//    token->end = end;
}

void
token_push(State *S, memi token, memi begin)
{

}

void
token_pop(State *S)
{

}

//
// Stack
//

inline void
stack_push(State *S, Parse *parse)
{
    auto item = S->stack.push(1, 16);
    item->parse = parse;
    // TODO: Clear cache.
}

inline Parse *
stack_pop(State *S)
{
    return S->stack.pop()->parse;
}

inline StackItem *
stack_top(State *S)
{
    return &S->stack.e[S->stack.count - 1];
}

//
// Input
//

hale_internal b32
input_match(State *S, const ch16 *needle, memi needle_length)
{
    memi n = 0;
    // TODO: Use boyer-moore search here, cache the forward match.
    if ((S->it + needle_length) <= S->it_)
    {
        switch (needle_length)
        {
        case 1:
            n = (S->it[0] == needle[0]);
            break;
        case 2:
            // TODO: Use SIMD check?
            n = (S->it[1] == needle[1]) &&
                (S->it[0] == needle[0])
                ;
            break;
        case 3:
            n = (S->it[2] == needle[2]) &&
                (S->it[1] == needle[1]) &&
                (S->it[0] == needle[0])
                ;
            break;
        default:
            hale_not_implemented;
            break;
        }

        if (n) {
            S->it += needle_length;
            return 1;
        }
    }
    return 0;
}

//
//
//

hale_internal void
push(State *S, memi token, Parse *parse, memi b)
{
    stack_push(S, parse);
    token_push(S, token, b);
}

hale_internal Parse *
pop(State *S)
{
    token_pop(S);
    return stack_pop(S);
}

//
//
//

#if 0
hale_internal void
_comment_block(State *S)
{
    if (input_match(S, "*/", 2)) {
        token_add(S, TokenId::CommentBlockEnd, 0, 2);
        pop(S);
    }
}

hale_internal void
_string_double_quoted(State *S)
{
    if (input_match(S, "\"", 1)) {
        token_add(S, token(S, TokenId::StringQuotedDoubleEnd), 0, 1);
        pop(S);
    }
}

hale_internal void
_root(State *S)
{
    if (input_match(S, "/*", 2)) {
        push(S, TokenId::CommentBlock, _comment_block, -2);
        token_add(S, token(S, TokenId::CommentBlockBegin), -2, 0);
    } else if (input_match(S, "\"", 1)) {
        push(S, TokenId::StringQuotedDouble, _string_quoted_double, -1);
        token_add(S, token(S, TokenId::StringQuotedDoubleBegin), 0, 1);
    }
}
#endif

hale_internal b32
_comment_block(State *S, memi ix)
{
    if (input_match(S, hale_u("*/"), 2)) {
        token_add(S, TokenId::CommentBlockEnd, 0, 2);
        pop(S);
        return 1;
    }
    return 0;
}

hale_internal b32
_string_quoted_double(State *S, memi ix)
{
    if (input_match(S, hale_u("\""), 1)) {
        token_add(S, TokenId::StringQuotedDoubleEnd, 0, 1);
        pop(S);
        return 1;
    }
    return 0;
}

// [...--] [-----]

hale_internal b32
_root(State *S, memi ix)
{
    switch (ix + __COUNTER__ - 1)
    {
    case __COUNTER__:
        if (input_match(S, hale_u("/*"), 2)) {
            push(S, TokenId::CommentBlock, _comment_block, -2);
            token_add(S, TokenId::CommentBlockBegin, -2, 0);
        };
        return 1;
    case __COUNTER__:
        if (input_match(S, hale_u("\""), 1)) {
            push(S, TokenId::StringQuotedDouble, _string_quoted_double, -1);
            token_add(S, TokenId::StringQuotedDoubleBegin, 0, 1);
        }
        return 1;

    default:
        return 0;
    }
}

//
//
//

void
_parse_init(Memory<u8> *storage)
{
    auto S = (State*)storage->push(sizeof(State));
    *S = {};

    push(S, TokenId::Root, _root, 0);
}

void
_parse(Memory<u8> *storage, ch16 *begin, ch16 *end)
{
    auto S = (State*)storage->e;
    S->it  = begin;
    S->it_ = end;

    memi ix = stack_top(S)->state_ix;
    Parse *parse;
    while (S->it != S->it_)
    {
        parse = stack_top(S)->parse;
        while (parse(S, ix)) {
            ++ix;
        }
        ix = 0;

        if (S->it != S->it_) {
            S->it++;
            // input_advance(S, 1);
        }
    }
}

CParser::CParser()
{
    parse_init = _parse_init;
    parse = _parse;
}

} // namespace hale
