namespace hale {

inline u32
hale_utf32(u32* state, u32* codepoint, u32 le, u32 unit)
{
    // `unit` is `u32`, but really we expect `u16`.
    hale_assert_debug((unit & 0xFFFF0000) == 0);

    // D8 1101 1000
    // DC 1101 1100
    //          ^

    // E0 1110 0000

    // Try to use AND
    // unit & ~0xD800
    // unit & ~0xDC00
    // unit & ~0xE000
    if (unit < 0xD800) {
        if (unit != '\n') {
            // Critical.
            *codepoint = unit;
            *state = 0;
        } else {
            // TODO: We're outputting 2 characters here, instead of one.
            *codepoint = le;
        }
    } else if (unit < 0xDC00) {
        *codepoint = (unit - 0xD800) << 10;
        *state = 1;
    } else if (unit < 0xE000) {
        if (*state == 1) {
            *codepoint += (unit - 0xDC00) + 0x0010000;
            *state = 0;
        } else {
            // TODO: Handle this by emitting unicode replacement char.
            hale_panic("Invalid surrogate pair.");
        }
    } else {
        // TODO: This is internal error.
        //       Only way to handle this is with
        //       unicode replacement char.
        hale_panic("Invalid unit in internal encoding.");
    }

    return 0;
}

#define UNI_SUR_HIGH_START  (u32)0xD800
#define UNI_SUR_HIGH_END    (u32)0xDBFF
#define UNI_SUR_LOW_START   (u32)0xDC00
#define UNI_SUR_LOW_END     (u32)0xDFFF

static const int half_shift  = 10; /* used for shifting by 10 bits */
static const u32 half_base = 0x0010000UL;

inline u32
utf16_utf32(u32* state, u32* codepoint, u32 le, u32 unit)
{
    if (unit >= UNI_SUR_HIGH_START && unit <= UNI_SUR_HIGH_END) {
        *codepoint = (unit - UNI_SUR_HIGH_START) << half_shift;
        *state = 1;
    } else if (unit >= UNI_SUR_LOW_START && unit <= UNI_SUR_LOW_END) {
        if (*state == 1) {
            *codepoint += (unit - UNI_SUR_LOW_START) + half_base;
            *state = 0;
        } else {
            // TODO: Handle this by emitting unicode replacement char.
            hale_panic("Invalid surrogate pair.");
        }
    } else {
        *codepoint = unit;
        *state = 0;
    }

    return 0;
}

static const u8 utf32_utf8_m[] =
{
    ~0x0000007F, 0xFF, 1, // critical
    ~0x000007FF, 0x1F, 2,
    ~0x0000FFFF, 0x0F, 3,
    ~0x0001FFFF, 0x07, 4,
    ~0x003FFFFF, 0x03, 5,
    ~0x07FFFFFF, 0x01, 6,
};

inline u8*
utf32_utf8(u8 *out, u32 codepoint)
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

}
