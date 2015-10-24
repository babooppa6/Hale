#ifndef HALE_STRING_H
#define HALE_STRING_H

// TODO: String should be able to store 0, so we cannot use str* functions.

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

//
//
//

// simple_print_float(float64 number, uint16 digits = 3, bool32 sign = True, bool32 sign_if_positive)
// https://www.refheap.com/41a61e226d06278aa004ab332

// http://stackoverflow.com/a/4351484

#include <string>

// TODO: Custom to_string, at least for ints and floats.
//template<typename T, typename Ch>
//void
//to_string(Vector<Ch> *string, T value);


inline void
to_string(Vector<ch16> *string, memi value)
{
    std::wstring s = std::to_wstring(value);
    vector_append(string, (ch16*)s.c_str(), s.length());
}

} // namespace hale

#endif // HALE_STRING_H

