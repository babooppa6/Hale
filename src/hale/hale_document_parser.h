#ifndef HALE_DOCUMENT_PARSER_H
#define HALE_DOCUMENT_PARSER_H

#if HALE_INCLUDES
#include "hale_types.h"
#include "hale_memory.h"
#endif

namespace hale {

//struct DocumentScopeInfo
//{
//    TextFormat format;
//};

#define HALE_TOKEN_INFINITY HALE_U16_MAX

struct Token
{
    // TODO: Points to DocumentScopeInfo
    memi id;
    u16 begin;
    u16 end;   // Should be max of (HALE_TOKEN_INFINITY-1),
               // because HALE_TOKEN_INFINITY is used for "+Infinity".
};

struct ParserState;

#define HALE_PARSE(N) b32 N(ParserState *S)
typedef HALE_PARSE(Parse);

struct ParserStackItem
{
    Parse *parse;
    memi token_id;
//    memi token_ix;

    bool operator != (const ParserStackItem &other) const {
        return parse != other.parse || token_id != token_id;
    }
};

struct Node
{
    memi ix;
    const char *name;
};

struct ParserState
{
    ch16 *it;
    ch16 *it_end;
    ch16 *it_begin;
    // Memory<memi> token_stack;

    // Output

    Memory<ParserStackItem> *stack;
    Memory<Token> *tokens;

    // TODO: Do the AST later.
//    Memory<Node> node_stack;
//    // TODO: This should be stored on the document.
//    Memory<Token> node_table;
};

#define HALE_DOCUMENT_PARSER_INIT(N) void N(ParserState *S)
typedef HALE_DOCUMENT_PARSER_INIT(DocumentParserInit);

#define HALE_DOCUMENT_PARSER_PARSE(N) void N(ParserState *S,\
                                             ch16 *begin, ch16 *end,\
                                             Memory<Token> *tokens,\
                                             Memory<ParserStackItem> *stack)
typedef HALE_DOCUMENT_PARSER_PARSE(DocumentParserParse);

//#define HALE_DOCUMENT_PARSER_RESET(N) void N(ParserState *S, Memory<Token> *tokens, Memory<ParserStackItem> *stack)
//typedef HALE_DOCUMENT_PARSER_RESET(DocumentParserReset);

struct DocumentParser
{
    DocumentParserInit *init;
//    DocumentParserReset *reset;
    DocumentParserParse *parse;
};

void print_parser_stack(const ch *name, Memory<ParserStackItem> *stack, memi block_index);
void print_parser_tokens(Memory<Token> *tokens, memi block_index);

}

#endif // HALE_DOCUMENT_PARSER_H

