#ifndef HALE_DOCUMENT_PARSER_H
#define HALE_DOCUMENT_PARSER_H

#include "hale.h"

namespace hale {


#define HALE_DOCUMENT_PARSE_INIT(N) void N(Memory<u8> *storage)
typedef HALE_DOCUMENT_PARSE_INIT(DocumentParseInit);

#define HALE_DOCUMENT_PARSE(N) void N(Memory<u8> *storage, ch16 *begin, ch16 *end)
typedef HALE_DOCUMENT_PARSE(DocumentParse);

struct DocumentParser
{
    DocumentParseInit *parse_init;
    DocumentParse *parse;
};

}

#endif // HALE_DOCUMENT_PARSER_H

