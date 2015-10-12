namespace hale {


//
// UTF-8 to UTF-32
//

// Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

enum Utf8State {
    Utf8State_Accept = 0,
    Utf8State_Reject = 12
};

//#define UTF8_ACCEPT 0
//#define UTF8_REJECT 12

static const u8 utf8d[] = {
  // The first part of the table maps bytes to character classes that
  // to reduce the size of the transition table and create bitmasks.
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
   8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

  // The second part is a transition table that maps a combination
  // of a state of the automaton and a character class to a state.
   0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
  12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
  12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
  12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
  12,36,12,12,12,12,12,12,12,12,12,12,
};

// state = 0  -> success (UTF8_ACCEPT)
// state = 12 -> invalid sequence (UTF8_REJECT)
// state = 24 -> 2 more bytes expected
// state = 36 -> 3 more bytes expected
// state = 48 -> 4 more bytes expected

u32 inline
utf8_utf32(u32* state, u32* codep, u32 byte)
{
  uint32_t type = utf8d[byte];

  *codep = (*state != Utf8State_Accept) ?
    (byte & 0x3fu) | (*codep << 6) :
    (0xff >> type) & (byte);

  *state = utf8d[256 + *state + type];
  return *state;
}

//
// Hale to UTF-32
//

// r -> * => LF *
//   -> r => LF LF
//   -> n => LF
// n      => LF
// * -> r
//   -> n

//

//    a        d       *
// 0 {out, 0} {nop, 1} {out, 0}
// 1 {LF, 0} { LF, 0} { LF, 0}

// out - emit codepoint
// nop - no output (eat the codepoint)
//  LF - emit 'LF' instead

//     00000011 00000001
// xor 00000001 00000001
//     00000010 00000000
// not 11111101 11111111

// 0xD 1101
// 0xA 1010

//     1101 0011
// and 1101 1101
//     1101 0001

// TODO: Validate whether it is okay to encode 0xD800 instead of 0xD7C0.
#define HALE_SURROGATE_H(codepoint)\
    (0xD7C0 + ((codepoint) >> 10))
#define HALE_SURROGATE_L(codepoint)\
    (0xDC00 + ((codepoint) & 0x3FF))
#define HALE_ENCODING_PAIR(H, L)\
    (((u32)H) << 16) | (((u32)L) & 0xFFFF)

// Old Intype convertor
#if 0
xuint UnicodeConverter::UTF8_UTF32( const U8* utf8str, xuint utf8len, U32& pch32 )
{
    U8    ch       = *utf8str++;
    U32   val32    = 0;
    xuint trailing = 0;
    xuint len      = 1;
    xuint i;
    static const U32 nonshortest[] = { 0, 0x80, 0x800, 0x10000, 0xffffffff, 0xffffffff };

    // validate parameters
    if (utf8str == NULL || utf8len <= 0) {
        return 0;
    }

    // look for plain ASCII first as this is most likely
    if (ch < 0x80)
    {
        pch32 = ch;
        return 1;
    }
    // LEAD-byte of 2-byte seq: 110xxxxx 10xxxxxx
    else if ((ch & 0xE0) == 0xC0)
    {
        trailing = 1;
        val32    = ch & 0x1F;
    }
    // LEAD-byte of 3-byte seq: 1110xxxx 10xxxxxx 10xxxxxx
    else if ((ch & 0xF0) == 0xE0)
    {
        trailing = 2;
        val32    = ch & 0x0F;
    }
    // LEAD-byte of 4-byte seq: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    else if ((ch & 0xF8) == 0xF0)
    {
        trailing = 3;
        val32    = ch & 0x07;
    }
    // ILLEGAL 5-byte seq: 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    else if ((ch & 0xFC) == 0xF8)
    {
        // range-checking the UTF32 result will catch this
        trailing = 4;
        val32    = ch & 0x03;
    }
    // ILLEGAL 6-byte seq: 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    else if ((ch & 0xFE) == 0xFC)
    {
        // range-checking the UTF32 result will catch this
        trailing = 5;
        val32    = ch & 0x01;
    }
    // ILLEGAL continuation (trailing) byte by itself
    else if ((ch & 0xC0) == 0x80)
    {
        pch32 = UNI_REPLACEMENT_CHAR;
        return 1;
    }
    // any other ILLEGAL form.
    else
    {
        pch32 = UNI_REPLACEMENT_CHAR;
        return 1;
    }

    // process trailing bytes
    for (i = 0; i < trailing && len < utf8len; i++)
    {
        ch = *utf8str++;

        // Valid trail-byte: 10xxxxxx
        if ((ch & 0xC0) == 0x80)
        {
            val32 = (val32 << 6) + (ch & 0x7f);
            len++;
        }
        // Anything else is an error
        else
        {
            pch32 = UNI_REPLACEMENT_CHAR;
            return len;
        }
    }

    // did we
    if (val32 < nonshortest[trailing] || i != trailing)
    {
        pch32 = UNI_REPLACEMENT_CHAR;
        return 0;
    }
    else
    {
        pch32 = val32;
    }
    return len;
}
#endif

#if 0
inline u16*
utf32_hale_fast(u32 *previous, u16 *unit, u32 in)
{
    if (in & 0x10000) {
        unit[0] = HALE_SURROGATE_H(in);
        unit[1] = HALE_SURROGATE_L(in);
        return unit + 2;
    }

    u16 results[] = {
        /* 0, 000 */   in,    0,    1,    0,
        /* 1, 001 */   0,     0,    2,    0,
        /* 2, 010 */ '\n',    0,    1,    0,
        /* 3, 011 */   77,   77,   77,    0, // invalid
        /* 4, 100 */    0,    0,    0,    0,
    };
//    results[0] = in;
//    results[4] = HALE_SURROGATE_H(in);
//    results[5] = HALE_SURROGATE_L(in);

    // surrogate
    // u32 s0 = (!!(in & 0x10000));
    // in == '\r'
    u32 s1 = !(in ^ '\r') << 1;
    // in == '\r' && *previous == '\r';
    u32 s2 = !((in ^ *previous) ^ (0xD ^ 0xA)) << 2;

    // u32 r = (s0 | s1 | s2) << 2; // * 4
    // u32 r = (s1 | s2) << 2; // * 4

//    u32 r = ( ((in & 0x10000) << 1) |
//              (!(in ^ '\r') << 2)   |
//              (!((in ^ *previous) ^ (0xD ^ 0xA)) << 3)
//            ) << 2;

    u32 r = ( (!(in ^ '\r') << 1)   |
              (!((in ^ *previous) ^ (0xD ^ 0xA)) << 2)
            ) << 2;

    hale_assert_debug(r  < 5*4);
    hale_assert_debug(r != 3*4);

    unit[0] = results[r+0];
    unit[1] = results[r+1];
    return unit + results[r+2];
}
#endif

#if 0
inline u16*
utf32_hale_fast(u32 *previous, u16 *unit, u32 in)
{
    if (in & 0x10000) {
        unit[0] = HALE_SURROGATE_H(in);
        unit[1] = HALE_SURROGATE_L(in);
        return unit + 2;
    }

    u32 results[] = {
        /* 0, 00 */   in,    1,
        /* 1, 01 */ '\n',    1,
        /* 2, 10 */    0,    0,
    };

    u32 r = (
              // in == '\r'
              (!(in ^ '\r')) ||
              // in == '\n' && *previous == '\r'
              (!((in ^ *previous) ^ (0xD ^ 0xA)) << 1)
            ) << 1;

    // hale_assert_debug(r < 3*2);

    unit[0] = results[r+0];
    return unit + results[r+1];
}
#endif

inline u16*
utf32_hale_fast(u32 *previous, u16 *unit, u32 in)
{
//    // in == '\r'
//    u32 s0 = !(in ^ '\r');
//    // in == '\r' && *previous == '\r';
//    u32 s2 = (((in == '\n') && (*previous == '\r')) << 1);
//    // surrogate
//    u32 s4 = (!!(in & 0x10000));

//    u32 s2x = in ^ *previous;

    u32 r = // in == '\r' (1)
            (!(in ^ '\r')) |
            // in == '\n' && *previous == '\r' (2)
            (((in == '\n') && (*previous == '\r')) << 1) |
            // surrogate (4)
            ((!!(in & 0x10000)) << 2)
            ;

    *previous = in;

    switch(r)
    {
    case 0:
        *unit++ = in;
        return unit;
    case 1:
        *unit++ = '\n';
        return unit;
    case 2:
        return unit;
    case 4:
        *unit++ = HALE_SURROGATE_H(in);
        *unit++ = HALE_SURROGATE_L(in);
        return unit;
    }

    hale_not_reached;

    return unit;
}


inline u16 *
utf32_hale(u32 *previous, u16 *unit, u32 in)
{
    if (in == '\r') {
        *unit++ = '\n';
    } else if (in == '\n' && *previous == '\r') {
        // skip
    } else if (in & 0x10000) {
        *unit++ = HALE_SURROGATE_H(in);
        *unit++ = HALE_SURROGATE_L(in);
    } else {
        *unit++ = in;
    }

    *previous = in;

    return unit;
}

//
//
//

inline u16 *
utf32_utf16(u32 *, u16 *unit, u32 in)
{
    if (in & 0x10000) {
        *unit++ = HALE_SURROGATE_H(in);
        *unit++ = HALE_SURROGATE_L(in);
    } else {
        *unit++ = (u16)in;
    }
    return unit;
}

} // namespace hale
