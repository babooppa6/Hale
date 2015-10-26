#if HALE_INCLUDES
#include "hale.h"
#include "hale_parser_c.h"
#endif

// https://github.com/zserge/jsmn

// #include "oniguruma.h"

namespace hale {

enum TokenId : memi
{
    Root = 1,

    CommentBlockBegin,
    CommentBlockEnd,
    CommentBlock,

    StringQuotedDoubleBegin,
    StringQuotedDoubleEnd,
    StringQuotedDouble,
};

//
// Tokens
//

Token *
token_add(ParserState *S, memi token_id, s32 begin, s32 end)
{
	hale_assert_input(end >= begin);

	memi p = (S->it - S->it_begin);
	hale_assert((s64)p >= (s64)begin);

	Token *token = S->tokens->push(1, 16);
	token->id = token_id;
	token->begin = p + begin;
	token->end = p + end;

	return token;
}

void
token_push(ParserState *S, memi token_id, s32 begin)
{
    // TODO: Use MAX_INT for ends of open tokens.
	Token *token = token_add(S, token_id, begin, begin);
    memi *token_s = S->token_stack.push(1, 4);
    *token_s = token - &S->tokens->e[0];
}

void
token_pop(ParserState *S, s32 end)
{
    memi p = (S->it - S->it_begin);
    hale_assert(p >= end);
    memi *token_s = S->token_stack.pop();
    S->tokens->e[*token_s].end = (S->it - S->it_begin) + end;
}

//
// Stack
//

inline void
stack_push(Memory<ParserStackItem> *stack, Parse *parse)
{
    auto item = stack->push(1, 16);
	*item = {};
    item->parse = parse;
    // TODO: Clear cache.
}

inline Parse *
stack_pop(Memory<ParserStackItem> *stack)
{
    return stack->pop()->parse;
}

inline ParserStackItem *
stack_top(Memory<ParserStackItem> *stack)
{
    hale_assert(stack->count);
    return &stack->e[stack->count - 1];
}

//
// Input
//

hale_internal b32
input_match(ParserState *S, const ch16 *needle, memi needle_length)
{
    memi n = 0;
    // TODO: Use boyer-moore search here, cache the forward match.
    if ((S->it + needle_length) <= S->it_end)
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
push(ParserState *S, memi token, Parse *parse, s32 begin)
{
    stack_push(S->stack, parse);
    token_push(S, token, begin);
}

hale_internal Parse *
pop(ParserState *S, s32 end)
{
    token_pop(S, end);
    return stack_pop(S->stack);
}

//
//
//

HALE_PARSE(_comment_block)
{
    if (input_match(S, hale_u("*/"), 2)) {
        token_add(S, TokenId::CommentBlockEnd, -2, 0);
        pop(S, 0);
        return 1;
    }
    return 0;
}


HALE_PARSE(_string_quoted_double)
{
    if (input_match(S, hale_u("\""), 1)) {
        token_add(S, TokenId::StringQuotedDoubleEnd, -1, 0);
        pop(S, 0);
        return 1;
    }
    return 0;
}

// [...--] [-----]

/* ....... */

hale_internal
HALE_PARSE(_root)
{
    if (input_match(S, hale_u("/*"), 2)) {
        push(S, TokenId::CommentBlock, _comment_block, -2);
        token_add(S, TokenId::CommentBlockBegin, -2, 0);
		return 1;
    } else if (input_match(S, hale_u("\""), 1)) {
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
HALE_DOCUMENT_PARSER_ALLOCATE(_allocate)
{
    *S = {};
}

hale_internal
HALE_DOCUMENT_PARSER_RESET(_reset)
{
    stack_push(stack, _root);
}

hale_internal
HALE_DOCUMENT_PARSER_PARSE(_parse)
{
    S->it_begin = begin;
    S->it_end   = end;
    S->it       = begin;
    S->tokens   = tokens;
    S->stack    = stack;

    Parse *parse;
    for (;;)
    {
        do {
            parse = stack_top(S->stack)->parse;
		} while (parse(S));

        if (S->it != S->it_end) {
            S->it++;
        } else {
			break;
		}
    }

    // TODO: Pop everything from the token stack.
}

CParser::CParser()
{
    allocate = _allocate;
    reset    = _reset;
    parse    = _parse;
}

} // namespace hale
