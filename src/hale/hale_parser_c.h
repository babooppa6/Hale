#ifndef HALE_PARSER_C_H
#define HALE_PARSER_C_H

#if HALE_INCLUDES
#include "hale_document_parser.h"
#endif

namespace hale {

struct CParser : public DocumentParser
{
    CParser();
};

}

#endif // HALE_PARSER_C_H

