#ifndef HALE_DOCUMENT_TYPES_H
#define HALE_DOCUMENT_TYPES_H

namespace hale {

struct DocumentPosition
{
    memi block;
    memi position;
};

struct DocumentRange
{
    DocumentPosition first;
    DocumentPosition second;
};

}

#endif // HALE_DOCUMENT_TYPES_H

