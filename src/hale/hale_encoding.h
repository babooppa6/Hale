#ifndef HALE_ENCODING_H
#define HALE_ENCODING_H

#if HALE_INCLUDES
#include "hale.h"
#include "hale_encoding_mib.h"
#endif

//#define HALE_UTF16_SUR_H_START  (u32)0xD800
//#define HALE_UTF16_SUR_H_END    (u32)0xDBFF
//#define HALE_UTF16_SUR_L_START  (u32)0xDC00
//#define HALE_UTF16_SUR_L_END    (u32)0xDFFF

namespace hale {

// UTF-8 Samples
// - http://www.columbia.edu/~kermit/utf8.html

// Encodings MIB Enum
// http://www.iana.org/assignments/character-sets/character-sets.xml
// http://www.iana.org/assignments/ianacharset-mib/ianacharset-mib

// Unicode mappings:
// http://www.unicode.org/Public/MAPPINGS

enum struct CodecReturn
{
    Error = 0,
    Success,
    // Input was insufficient. The sequence was not read completely.
    InputPending,
    // Output buffer has been used.
    OutputUsed
};

struct CodecState
{
    ch32 codepoint;
    u32 input_state;
    u32 output_state;
    u32 input_option;
};

struct EncodingDescriptor
{
    ch8 *name;
    memi output_padding;
    memi state_mask;
    ch8 *preamble;
    memi preamble_size;
};

EncodingDescriptor *encoding_get(Encoding encoding);
template<Encoding E>
EncodingDescriptor *encoding_get();

// TODO: Make sure we report the state so that we won't have to cycle twice to read in and out if the buffer sizes are synchronized. Possibly has also something to do with buffer padding.
// - May be, that we won't need to signal InputPending, in case that the utf32_from always eats the whole input. (Ehm)

template<Encoding In, Encoding Out>
CodecReturn
codec(typename EncodingInfo<In>::Storage  **in,
      typename EncodingInfo<In>::Storage  *in_,
      typename EncodingInfo<Out>::Storage **out,
      typename EncodingInfo<Out>::Storage *out_,
      CodecState *s)
{
    out_ = out_ - EncodingInfo<Out>::OutputPadding;
    hale_assert_message(*in < in_,
                        "Insufficient input.");
    hale_assert_message(*out <= out_,
                        "Insufficient output. At least 2 u16 are required.");

    CodecReturn ret = CodecReturn::Success;
    for (;;)
    {
        *in = utf32_from<In>(s, *in);

        if (s->input_state == EncodingInfo<In>::Accept || s->input_state == EncodingInfo<In>::MultiOut)
        {
            *out = utf32_to<Out>(s, *out);

            if (*out > out_) {
                ret = CodecReturn::OutputUsed;
                break;
            }
        }
        else if (s->input_state == EncodingInfo<In>::Reject)
        {
            hale_panic("Invalid sequence.");
        }

        if (*in == in_) {
            ret = CodecReturn::InputPending;
            break;
        }
    }

    return ret;
}

template<Encoding In, Encoding Out>
struct Codec
{
    typename EncodingInfo<In>::Storage  *in;
    typename EncodingInfo<In>::Storage  *in_;
    typename EncodingInfo<Out>::Storage *out;
    typename EncodingInfo<Out>::Storage *out_;
    CodecState s;
};

template<Encoding In, Encoding Out>
CodecReturn
codec(Codec<In, Out> *c)
{
    return codec<In, Out>(&c->in, c->in_, &c->out, c->out_, &c->s);
}


#if 0
template<Encoding In, Encoding Out>
CodecReturn
codec_unrolled(typename EncodingInfo<In>::Storage  **in,
      typename EncodingInfo<In>::Storage  *in_,
      typename EncodingInfo<Out>::Storage **out,
      typename EncodingInfo<Out>::Storage *out_,
      CodecState *s)
{
    out_ = out_ - EncodingInfo<Out>::OutputPadding;
    hale_assert_message(*in < in_,
                        "Insufficient input.");
    hale_assert_message(*out <= out_,
                        "Insufficient output. At least 2 u16 are required.");

    CodecReturn ret = CodecReturn::Success;
    ch32 cp[4];
    for (;;)
    {
        *in = utf32_from<In>(s, cp[0], *in);

        if (s->input_state == EncodingInfo<In>::Accept)
        {
            *out = utf32_to<Out>(s, *out);

            if (*out > out_) {
                ret = CodecReturn::OutputUsed;
                break;
            }
        }
        else if (s->input_state == EncodingInfo<In>::Reject)
        {
            hale_panic("Invalid sequence.");
        }

        if (*in == in_) {
            ret = CodecReturn::InputPending;
            break;
        }
    }

    return ret;
}
#endif

} // namespace hale

#endif // HALE_ENCODING_H

