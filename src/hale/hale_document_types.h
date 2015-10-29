#ifndef HALE_DOCUMENT_TYPES_H
#define HALE_DOCUMENT_TYPES_H

namespace hale {

struct DocumentPosition
{
    memi block;
    memi position;

    bool operator ==(const DocumentPosition &other) const
    { return block == other.block && position == other.position; }
    bool operator !=(const DocumentPosition &other) const
    { return block != other.block || position != other.position; }

    bool operator >(const DocumentPosition &other) const
    {
        if (block == other.block) {
            return position > other.position;
        }
        return block > other.block;
    }

    bool operator <(const DocumentPosition &other) const
    {
        if (block == other.block) {
            return position < other.position;
        }
        return block < other.block;
    }

    bool operator >=(const DocumentPosition &other) const
    {
        if (block == other.block) {
            return position >= other.position;
        }
        return block >= other.block;
    }

    bool operator <=(const DocumentPosition &other) const
    {
        if (block == other.block) {
            return position <= other.position;
        }
        return block <= other.block;
    }
};

struct DocumentRange
{
    DocumentPosition first;
    DocumentPosition second;
};

}

#endif // HALE_DOCUMENT_TYPES_H

