#ifndef HALE_DOCUMENT_PARSER_H
#define HALE_DOCUMENT_PARSER_H

#if HALE_INCLUDES
#include "hale.h"
#endif

namespace hale {

//struct DocumentScopeInfo
//{
//    TextFormat format;
//};

struct Token
{
    // TODO: Points to DocumentScopeInfo
    memi id;
    memi begin;
    memi end;
};

struct ParserState;
#define HALE_PARSE(N) b32 N(ParserState *S)
typedef HALE_PARSE(Parse);

struct ParserStackItem
{
    Parse *parse;
    bool operator != (const ParserStackItem &other) const {
        return parse == other.parse;
    }
};

typedef Memory<ParserStackItem> ParserStack;
typedef Memory<Token> Tokens;

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
    Memory<memi> token_stack;

    // Output

    ParserStack *stack;
    Tokens *tokens;

    // TODO: Do the AST later.
//    Memory<Node> node_stack;
//    // TODO: This should be stored on the document.
//    Memory<Token> node_table;
};


#define HALE_DOCUMENT_PARSER_ALLOCATE(N) void N(ParserState *S)
typedef HALE_DOCUMENT_PARSER_ALLOCATE(DocumentParserAllocate);

#define HALE_DOCUMENT_PARSER_RESET(N) void N(ParserState *S, ParserStack *stack)
typedef HALE_DOCUMENT_PARSER_RESET(DocumentParserReset);

#define HALE_DOCUMENT_PARSER_PARSE(N) void N(ParserState *S,\
                                             ch16 *begin, ch16 *end,\
                                             Tokens *tokens,\
                                             ParserStack *stack)
typedef HALE_DOCUMENT_PARSER_PARSE(DocumentParserParse);

struct DocumentParser
{
    DocumentParserAllocate *allocate;
    DocumentParserReset *reset;
    DocumentParserParse *parse;
};

}

#endif // HALE_DOCUMENT_PARSER_H

