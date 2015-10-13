namespace hale {

template<>
inline u16*
utf32_from<Encoding::UTF16LE, u16>(CodecState *s, u16 *in)
{
    u32 unit = *in;

    if (unit >= 0xD800 && unit <= 0xDBFF) {
        s->codepoint = (unit - 0xD800) << 10;
        s->input_state = 1;
    } else if (unit >= 0xDC00 && unit <= 0xDFFF) {
        if (s->input_state == 1) {
            s->codepoint += (unit - 0xDC00) + 0x10000;
            s->input_state = 0;
        } else {
            s->input_state = 8;
        }
    } else {
        s->codepoint = unit;
        s->input_state = 0;
    }

    return in + 1;
}

//
//
//

template<>
inline u16*
utf32_to<Encoding::UTF16LE, u16>(CodecState *s, u16 *out)
{
    if (s->codepoint & ~0xFFFF) {
        *out++ = HALE_UTF16_SUR_H(s->codepoint);
        *out++ = HALE_UTF16_SUR_L(s->codepoint);
    } else {
        *out++ = (u16)s->codepoint;
    }
    return out;
}

} // namespace hale
