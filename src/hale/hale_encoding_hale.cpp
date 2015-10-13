namespace hale {


template<>
inline u16*
utf32_from<Encoding::Hale, u16>(CodecState *s, u16 *in)
{
    if (s->input_state == 2) {
        s->codepoint = s->input_option & 0xFFFF;
        s->input_state = 0;
        return in;
    }

    u32 unit = *in;
    in++;

    // `unit` is `u32`, but really we expect `u16`.
    hale_assert_debug((unit & 0xFFFF0000) == 0);

    if (unit < 0xD800) {
        if (unit != '\n') {
            // Critical.
            s->codepoint = unit;
            s->input_state = 0;
        } else {
            // TODO: We're outputting 2 characters here, instead of one.
            s->codepoint = s->input_option >> 16;
            s->input_state = (s->input_option & 0xFFFF) == 0 ? 2 : 0;
        }
    } else if (unit < 0xDC00) {
        s->codepoint = (unit - 0xD800) << 10;
        s->input_state = 1;
    } else if (unit < 0xE000) {
        if (s->input_state == 1) {
            s->codepoint += (unit - 0xDC00) + 0x10000;
            s->input_state = 0;
        } else {
            s->input_state = 8;
            // TODO: Handle this by emitting unicode replacement char.
            hale_panic("Invalid surrogate pair.");
        }
    } else {
        s->input_state = 8;
        // TODO: This is internal error.
        //       Only way to handle this is with
        //       unicode replacement char.
        hale_panic("Invalid unit in internal encoding.");
    }

    return in;
}

//
//
//

#define HALE_ENCODING_PAIR(H, L)\
    (((u32)H) << 16) | (((u32)L) & 0xFFFF)

#if 1

// 16% faster than naive
template<>
inline u16*
utf32_to<Encoding::Hale, u16>(CodecState *s, u16 *out)
{
    u32 r = // in == '\r' (1)
            (!(s->codepoint ^ '\r')) |
            // in == '\n' && *previous == '\r' (2)
            (((s->codepoint == '\n') && (s->output_state == '\r')) << 1) |
            // surrogate (4)
            ((!!(s->codepoint & ~0xFFFF)) << 2)
            ;

    s->output_state = s->codepoint;

    switch(r)
    {
    case 0:
        *out++ = s->codepoint;
        return out;
    case 1:
        *out++ = '\n';
        return out;
    case 2:
        return out;
    case 4:
        *out++ = HALE_UTF16_SUR_H(s->codepoint);
        *out++ = HALE_UTF16_SUR_L(s->codepoint);
        return out;
    }

    hale_not_reached;

    return out;
}

#else

template<>
inline u16 *
utf32_to<Encoding::Hale>(u32 *previous, u16 *unit, u32 in)
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

#endif

}
