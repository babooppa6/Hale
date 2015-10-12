#include "hale.h"
#include "hale_encoding.h"

#include "hale_encoding_decode_helpers.cpp"
#include "hale_encoding_encode_helpers.cpp"

namespace hale {

CodecReturn
decoder_utf8(Decode *s)
{
    u8  *in  = s->in;
    u8  *in_ = s->in_;
    hale_assert(in <= in_);

    u16 *out  = s->out;
    u16 *out_ = s->out_-2;
    // There has to be at least 2 u16s available.
    hale_assert_message(out <= out_, "Insufficient output. At least 2 u16 are required.");

    CodecReturn ret = CodecReturn::Success;
    u32 utf8_state = s->utf8.state;
    u32 utf8_codepoint = s->utf8.codepoint;
    u32 hale_state = s->hale.state;
    for (;;)
    {
        // TODO: This is not neccessary to be done on first iteration.
        if (in == in_) {
            ret = CodecReturn::InputPending;
            break;
        }

        // TODO: Check for state == Utf8State_Reject.
        if (utf8_utf32(&utf8_state, &utf8_codepoint, *in++) != Utf8State_Accept) {
            continue;
        }

        // TODO: Encode the error sequence using an invalid surrogate prefix.
        //       Be sure to stay compatible with UTF-16 readers (text layouting engines).
        //       Write it back in encoder_utf8.

#if 1
        // ~16% faster than utf32_hale
        out = utf32_hale_fast(&hale_state, out, utf8_codepoint);
#else
        // out = utf32_utf16(&hale_state, out, utf8_codepoint);
        out = utf32_hale(&hale_state, out, utf8_codepoint);
#endif

        if (out > out_) {
            ret = CodecReturn::OutputUsed;
            break;
        }
    }

    s->in = in;
    s->out = out;
    s->utf8.state = utf8_state;
    s->utf8.codepoint = utf8_codepoint;
    s->hale.state = hale_state;

    return ret;
}

CodecReturn
encoder_utf8(Encode *s)
{
    u16 *in  = s->in;
    u16 *in_ = s->in_;
    hale_assert(in <= in_);

    u8  *out  = s->out;
    u8  *out_ = s->out_-6;
    hale_assert_message(out <= out_, "Insufficient output. At least 6 u8 are required.");

    CodecReturn ret = CodecReturn::Success;
    u32 hale_state = s->hale.state;
    u32 hale_codepoint = s->hale.codepoint;
    for (;;)
    {
        if (in == in_) {
            ret = CodecReturn::InputPending;
            break;
        }

        hale_utf32(&hale_state, &hale_codepoint, 0x000D000A, *in++);

        // TODO: Check for error. (quite impossible with hale encoding)
        if (hale_state != 0) {
            continue;
        }

        out = utf32_utf8(out, hale_codepoint);

        if (out > out_) {
            ret = CodecReturn::OutputUsed;
            break;
        }
    }

    s->in = in;
    s->out = out;
    s->hale.state = hale_state;
    s->hale.codepoint = hale_codepoint;
    // s->hale.state = hale_state;

    return ret;
}

}
