#include "hale.h"
#include "hale_encoding.h"

namespace hale {


//struct EncodingDescriptor
//{
//    memi output_padding;
//    memi state_mask;

//    ch8 *preamble;
//    memi preamble_size;
//};

hale_internal
EncodingDescriptor encoding_table[Encoding::MAX] =
{
    { (ch8*)"Null", 0, 0, 0, 0 },
    { (ch8*)"Other", 0, 0, 0, 0 },
    { (ch8*)"Unknown", 0, 0, 0, 0 },
    { (ch8*)"ASCII", 0, 0, 0, 0 },
    { (ch8*)"UTF-8", 4, 12, (ch8*)"\xEF\xBB\xBF", 3 },
    { (ch8*)"UTF-16LE", 2, 1, (ch8*)"\xFF\xFE", 2 },
    { (ch8*)"Hale", 2, 1, (ch8*)"\xFF\xFF", 2 },
};

EncodingDescriptor *encoding_get(Encoding encoding)
{
    hale_assert(((memi)encoding) > 3 && ((memi)encoding) < 7);
    return &encoding_table[(memi)encoding];
}

template<Encoding E>
EncodingDescriptor *encoding_get()
{
    return &encoding_table[E];
}

}

#include "hale_encoding_utf8.cpp"
#include "hale_encoding_utf16.cpp"
#include "hale_encoding_hale.cpp"
