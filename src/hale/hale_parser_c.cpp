#if HALE_INCLUDES
#include "hale_macros.h"
#include "hale_types.h"
#include "hale_parser_c.h"
#include "hale_document_parser.h"
#endif

// https://github.com/zserge/jsmn

// #include "oniguruma.h"

namespace hale {

enum TokenId : memi
{
    Null = 0, // Reserved.

    Root = 1,

    CommentBlockBegin = 2,
    CommentBlockEnd = 3,
    CommentBlock = 4,

    StringQuotedDoubleBegin = 5,
    StringQuotedDoubleEnd = 6,
    StringQuotedDouble = 7,
};

struct TokenInfo
{
    ch16 *name;
};

hale_internal TokenInfo static_token_info[] =
{
    { hale_ch("null") },

    { hale_ch("source.c") },

    { hale_ch("comment.block.begin") },
    { hale_ch("comment.block.end") },
    { hale_ch("comment.block") },

    { hale_ch("string.quoted.double.begin") },
    { hale_ch("string.quoted.double.end") },
    { hale_ch("string.quoted.double") }
};

inline memi _position(ParserState *S) {
	return S->it - S->it_begin;
}

struct StackLinePosition
{
    memi row;
    memi column;
};

Memory<Token> token_table;

//
// Saves token to a token table.
// - Token table is contains unique token information for each token.
// - Index to this table is used to reference the token in the stacks.

hale_internal memi
token_id(TokenId id)
{
    for (memi i = 0; i != token_table.count; ++i)
    {
        if (token_table.ptr[i].id == id) {
            return i;
        }
    }
    Token *token = token_table.push(1, 16);
    token->id = id;
    return token_table.count - 1;
}

//
//
//

inline ParserStackItem *top(ParserState *S);

//
//
//

inline void
flush(ParserState *S, memi begin)
{
	// Insert a `text` token between this and last token.
	memi text_begin;
	memi text_end = begin;
	if (S->tokens->count != 0){
        text_begin = S->tokens->ptr[S->tokens->count - 1].end;
	}
	else {
		text_begin = 0;
	}

	Token *token;

	if (text_begin != text_end)
	{
        token = S->tokens->push(1, 16);
		token->id = top(S)->token_id;
        hale_assert_debug((text_begin & ~(memi)0xFFFF) == 0);
        hale_assert_debug((text_end & ~(memi)0xFFFF) == 0);
        token->begin = (u16)text_begin;
		token->end = text_end;
	}
}

//
// Stack
//

inline void
push(ParserState *S, memi token_id, Parse *parse, s32 begin)
{
	flush(S, _position(S) + begin);

    ParserStackItem *item = S->stack->push(1, 16);
    *item = {};
    item->parse = parse;
    item->token_id = token_id;
}

inline Parse *
pop(ParserState *S, s32 end)
{
    hale_assert(S->stack->count != 0);

	flush(S, _position(S) + end);
    ParserStackItem *item = S->stack->pop();
    return item->parse;
}

inline ParserStackItem *
top(ParserState *S)
{
    hale_assert_debug(S->stack->count != 0);
    return &S->stack->ptr[S->stack->count - 1];
}

//
// Tokens
//

Token *
token_add(ParserState *S, memi token_id, s32 begin, s32 end)
{
    // TODO: Assert when the new token is overlapping with the token already in the tokens list.
	hale_assert_input(end >= begin);
	memi p = _position(S);
	hale_assert((s64)p >= (s64)begin);

	flush(S, p + begin);

    Token *token = S->tokens->push(1, 16);
	token->id = token_id;
    hale_assert_debug(((p+begin) & ~(memi)0xFFFF) == 0);
    hale_assert_debug(((p+end) & ~(memi)0xFFFF) == 0);
    token->begin = p + begin;
	token->end = p + end;

	return token;
}


//
// Input
//

hale_internal b32
input_match(ParserState *S, const ch16 *needle, memi needle_length)
{
    memi n = needle_length;
    // TODO: Use boyer-moore search here, cache the forward match.
    if ((S->it + needle_length) <= S->it_end)
    {
        switch (needle_length)
        {
        case 6:
            n -= (S->it[5] == needle[5]);
        case 5:
            n -= (S->it[4] == needle[4]);
        case 4:
            n -= (S->it[3] == needle[3]);
        case 3:
            n -= (S->it[2] == needle[1]);
        case 2:
            n -= (S->it[1] == needle[1]);
        case 1:
            n -= (S->it[0] == needle[0]);
            break;
        default:
            hale_not_implemented;
            break;
        }

        if (n == 0) {
            S->it += needle_length;
            return 1;
        }
    }
    return 0;
}

//
//
//

hale_internal
HALE_PARSE(_comment_block)
{
    if (input_match(S, hale_ch("*/"), 2)) {
        token_add(S, TokenId::CommentBlockEnd, -2, 0);
        pop(S, 0);
        return 1;
    }
    return 0;
}


HALE_PARSE(_string_quoted_double)
{
    if (input_match(S, hale_ch("\""), 1)) {
        token_add(S, TokenId::StringQuotedDoubleEnd, -1, 0);
        pop(S, 0);
        return 1;
    }
    return 0;
}

hale_internal
HALE_PARSE(_root)
{
    if (input_match(S, hale_ch("/*"), 2)) {
        push(S, TokenId::CommentBlock, _comment_block, -2);
        token_add(S, TokenId::CommentBlockBegin, -2, 0);
		return 1;
    } else if (input_match(S, hale_ch("\""), 1)) {
        push(S, TokenId::StringQuotedDouble, _string_quoted_double, -1);
        token_add(S, TokenId::StringQuotedDoubleBegin, -1, 0);
		return 1;
    }
	return 0;
}

//
//
//

hale_internal
HALE_DOCUMENT_PARSER_INIT(_init)
{
    *S = {};
}

//hale_internal
//HALE_DOCUMENT_PARSER_RESET(_reset)
//{
//    S->stack = stack;
//    S->stack->count = 0;
//    S->tokens = tokens;
//    S->tokens->count = 0;
//}

hale_internal
HALE_DOCUMENT_PARSER_PARSE(_parse)
{
    S->it_begin = begin;
    S->it_end   = end;
    S->it       = begin;
    S->tokens   = tokens;
    S->stack    = stack;

    S->tokens->count = 0;
    if (S->stack->count == 0)
    {
        push(S, Root, _root, 0);
    }
//    else
//    {
//        S->token_stack.count = 0;

//        memi *p = S->token_stack.push(S->stack->count, 16);
//        for (memi i = 0; i < S->stack->count; i++) {
//            *p++ = token_add(S, S->stack->e[i].token_id, 0, HALE_S32_MAX) - S->tokens->e;
//        }
//    }

    Token *token;
    ParserStackItem *stack_top;
    Parse *parse;
    for (;;)
    {
        stack_top = top(S);
        if (stack_top->parse(S) == 0)
        {
            if (S->it == S->it_end) {
                break;
            }
            S->it++;
        }
    }

	flush(S, _position(S));
}

ParserC::ParserC()
{
    memi c = __stack__.capacity;

    init     = _init;
    parse    = _parse;
}

} // namespace hale
