namespace hale {

//enum Utf8State {
//    Utf8State_Accept = 0,
//    Utf8State_Reject = 12
//};

const u8* EncodingInfo<Encoding::UTF8>::Preamble = (const u8*)"\xEF\xBB\xBF";

//
// UTF-8 to UTF-32
//

// Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

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

template<>
u8*
utf32_from<Encoding::UTF8>(CodecState *s, u8 *in)
{
    uint32_t type = utf8d[*in];

    s->codepoint = (s->input_state != EncodingInfo<Encoding::UTF8>::Accept) ?
        (*in & 0x3fu) | (s->codepoint << 6) :
        (0xff >> type) & (*in);

    s->input_state = utf8d[256 + s->input_state + type];
    return in + 1;
}

//
//
//

//static const u8 utf32_utf8_m[] =
//{
//    ~0x0000007F, 0xFF, 1, // critical
//    ~0x000007FF, 0x1F, 2,
//    ~0x0000FFFF, 0x0F, 3,
//    ~0x0001FFFF, 0x07, 4,
//    ~0x003FFFFF, 0x03, 5,
//    ~0x07FFFFFF, 0x01, 6,
//};

// UTF-8 RFC 3629 restricted to U+10FFFF to match UTF-16 constraints.
// See utf32_utf8_31 for full 31-bit encoding.

// Maximum output: 4-bytes.
template<>
inline u8*
utf32_to<Encoding::UTF8>(CodecState *s, u8 *out)
{
    u32 codepoint = s->codepoint;
    if (codepoint < 0x80) { // (codepoint & ~0x7F) == 0
        *out++ = codepoint;
    } else if (codepoint < 0x800)   {    // U+0080      U+07FF
        // 110x xxxx
        *out++ = 0xC0 | ((codepoint >> (6 * 1)) & 0x1F);
        // tail
        *out++ = 0x80 | ((codepoint >> (6 * 0)) & 0x3F);
    } else if (codepoint < 0x10000) {    // U+0800      U+FFFF
        // 1110 xxxx
        *out++ = 0xE0 | ((codepoint >> (6 * 2)) & 0x0F);
        // tail
        *out++ = 0x80 | ((codepoint >> (6 * 1)) & 0x3F);
        *out++ = 0x80 | ((codepoint >> (6 * 0)) & 0x3F);
    } else if (codepoint < 0x10FFFF) {   // U+10000     U+1FFFFF
        // 1111 0xxx
        *out++ = 0xF0 | ((codepoint >> (6 * 3)) & 0x07);
        // tail
        *out++ = 0x80 | ((codepoint >> (6 * 2)) & 0x3F);
        *out++ = 0x80 | ((codepoint >> (6 * 1)) & 0x3F);
        *out++ = 0x80 | ((codepoint >> (6 * 0)) & 0x3F);
    } else {
        // Invalid codepoint (beyond Unicode maximum codepoint)
        // TODO: God, please don't let me forget this in here.
        hale_panic("Invalid codepoint.");
    }

    return out;
}

#if 0
template<>
inline u8*
utf32_to<Encoding::UTF8>(u8 *out, u32 codepoint)
{
    if (codepoint < 0x80) { // (codepoint & ~0x7F) == 0
        *out++ = codepoint;
    } else if (codepoint < 0x800)   {    // U+0080      U+07FF
        // 110x xxxx
        *out++ = 0xC0 | ((codepoint >> (6 * 1)) & 0x1F);
        // tail
        *out++ = 0x80 | ((codepoint >> (6 * 0)) & 0x3F);
    } else if (codepoint < 0x10000) {    // U+0800      U+FFFF
        // 1110 xxxx
        *out++ = 0xE0 | ((codepoint >> (6 * 2)) & 0x0F);
        // tail
        *out++ = 0x80 | ((codepoint >> (6 * 1)) & 0x3F);
        *out++ = 0x80 | ((codepoint >> (6 * 0)) & 0x3F);
    } else if (codepoint < 0x200000) {   // U+10000     U+1FFFFF
        // 1111 0xxx
        *out++ = 0xF0 | ((codepoint >> (6 * 3)) & 0x07);
        // tail
        *out++ = 0x80 | ((codepoint >> (6 * 2)) & 0x3F);
        *out++ = 0x80 | ((codepoint >> (6 * 1)) & 0x3F);
        *out++ = 0x80 | ((codepoint >> (6 * 0)) & 0x3F);
    } else if (codepoint < 0x4000000) {  // U+200000	U+3FFFFFF
        // 1111 10xx
        *out++ = 0xF8 | ((codepoint >> (6 * 4)) & 0x03);
        // tail
        *out++ = 0x80 | ((codepoint >> (6 * 3)) & 0x3F);
        *out++ = 0x80 | ((codepoint >> (6 * 2)) & 0x3F);
        *out++ = 0x80 | ((codepoint >> (6 * 1)) & 0x3F);
        *out++ = 0x80 | ((codepoint >> (6 * 0)) & 0x3F);
    } else if (codepoint < 0x80000000) { // U+4000000	U+7FFFFFFF
        // 1111 110x
        *out++ = 0xFC | ((codepoint >> (6 * 5)) & 0x01);
        // tail
        *out++ = 0x80 | ((codepoint >> (6 * 4)) & 0x3F);
        *out++ = 0x80 | ((codepoint >> (6 * 3)) & 0x3F);
        *out++ = 0x80 | ((codepoint >> (6 * 2)) & 0x3F);
        *out++ = 0x80 | ((codepoint >> (6 * 1)) & 0x3F);
        *out++ = 0x80 | ((codepoint >> (6 * 0)) & 0x3F);
    } else {
        // Invalid codepoint (beyond Unicode maximum codepoint)
        // TODO: God, please don't let me forget this in here.
        hale_panic("Invalid codepoint.");
    }

    return out;
}
#endif

} // namespace hale
