#ifndef HALE_STRING_H
#define HALE_STRING_H

#if HALE_INCLUDES
#include "hale_atoi.h"
#endif

namespace hale {

//
// Indentation
//

enum struct IndentationMode
{
    Spaces,
    Tabs
};

//
// LineEnding
//

enum LineEndingType {
    LineEnding_Unknown,
    LineEnding_CRLF,
    LineEnding_LF,
    LineEnding_CR,
};

struct LineEnding
{
    LineEndingType type;
    memi length;
    ch *string;
    ch *name;
};

extern LineEnding line_endings[];

LineEnding & string_find_next_line_ending(ch **begin, ch *end);

inline memi
string_length(const ch8 *string)
{
    const ch8 *it = string;
    while (*it) {
        it++;
    }
    return it - string;
}

inline memi
string_length(const ch16 *string)
{
    const ch16 *it = string;
    while (*it) {
        it++;
    }
    return it - string;
}

#if 0
inline void
vector_insert(Vector<ch> *string, memi offset, ch *other)
{
    memi length = string_length(other);
    vector_insert(string, offset, other, length);
}

template<typename Ch>
inline b32
vector_equal(Vector<Ch> *av, const Ch *b)
{
    if (vector_count(av) == 0) {
        return *b == 0;
    }

    Ch *a  = vector_data(av);
    return equal(a,
                 a + vector_count(av),
                 b);
}

template<typename Ch>
inline b32
vector_equal(Vector<Ch> *av, const Ch *b, memi b_count)
{
    if (vector_count(av) == 0) {
        return b_count == 0;
    }

    Ch *a  = vector_data(av);
    return equal(a,
                 a + vector_count(av),
                 b,
                 b + b_count);
}
#endif

inline void
memory_copy0(ch8 *destination, memi destination_count, ch8 *source)
{
    memi c = minimum(destination_count-1, string_length(source)+1);
    platform.copy_memory(destination, source, c);
    destination[c] = 0;
}

inline void
memory_copy0(ch16 *destination, memi destination_count, ch16 *source)
{
    memi c = minimum(destination_count-1, string_length(source)+1);
    platform.copy_memory(destination, source, c * sizeof(ch16));
    destination[c] = 0;
}

//
//
//

// simple_print_float(float64 number, uint16 digits = 3, bool32 sign = True, bool32 sign_if_positive)
// https://www.refheap.com/41a61e226d06278aa004ab332

// http://stackoverflow.com/a/4351484

//#include <string>

// TODO: Custom to_string, at least for ints and floats.
//template<typename T, typename Ch>
//void
//to_string(Vector<Ch> *string, T value);


// https://github.com/miloyip/itoa-benchmark/tree/master/src

inline void
to_string(Vector<ch16> *string, memi value)
{
    ch16 buffer[24];
    ch16 *b = itoa<BranchLut, memi, ch16>(value, buffer);
    hale_assert_debug((b-buffer) < 21);
    vector_append(string, buffer, b - buffer);
}

//inline void
//ch16_ch8(Memory<ch16> *memory, ch8 *string)
//{

//}

} // namespace hale

#endif // HALE_STRING_H

