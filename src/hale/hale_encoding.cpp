#include "hale.h"
#include "hale_encoding.h"

//namespace hale {

//template<Encoding, typename T>
//T*
//utf32_from(CodecState *s, T *in);

//template<Encoding, typename T>
//T*
//utf32_to(CodecState *s, T *out);

//} // namespace hale

namespace hale {

}

#include "hale_encoding_utf8.cpp"
#include "hale_encoding_utf16.cpp"
#include "hale_encoding_hale.cpp"

namespace hale {

//template<>
//CodecReturn
//decode<Encoding::UTF8>(ch8 **in, ch8 *in_, ch16 **out, ch16 *out_, CodecState *s)
//{
//    return codec<Encoding::UTF8, Encoding::Hale>(in, in_, out, out_, s);
//}

//template<>
//CodecReturn
//encode<Encoding::UTF8>(ch16 **in, ch16 *in_, ch8 **out, ch8 *out_, CodecState *s)
//{
//    return codec<Encoding::Hale, Encoding::UTF8>(in, in_, out, out_, s);
//}

#if 0
template<>
CodecReturn
decode<Encoding::UTF8>(u8 **in, u8 *in_, u16 **out, u16 *out_, CodecState *s)
{
    out_ = out_ - 2;
    hale_assert_message(*in <= in_,
                        "Insufficient input.");
    hale_assert_message(*out <= out_,
                        "Insufficient output. At least 2 u16 are required.");

    memi oi = 0;
    CodecReturn ret = CodecReturn::Success;
    for (;;)
    {
        *in = utf32_from<Encoding::UTF8>(s, *in);

        if (s->input_state == Utf8State_Accept)
        {
            oi++;
            *out = utf32_to<Encoding::Hale>(s, *out);

            if (*out > out_) {
                ret = CodecReturn::OutputUsed;
                break;
            }
        }
        else if (s->input_state == Utf8State_Reject)
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

template<>
CodecReturn
encode<Encoding::UTF8>(u16 **in, u16 *in_, u8 **out, u8 *out_, CodecState *s)
{
    out_ = out_ - 4; // Was 6, but utf8 is restricted to 10FFFF.
    hale_assert_message(*in <= in_,
                        "Insufficient input.");
    hale_assert_message(*out <= out_,
                        "Insufficient output. At least 4 bytes padding is required.");

    CodecReturn ret = CodecReturn::Success;

    for (;;)
    {
        *in = utf32_from<Encoding::Hale>(s, *in);

        if (s->input_state == 0)
        {
            *out = utf32_to<Encoding::UTF8>(s, *out);

            if (*out > out_) {
                ret = CodecReturn::OutputUsed;
                break;
            }
        }
        else if (s->input_state = 8)
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

}
