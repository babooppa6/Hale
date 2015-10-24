#ifndef HALE_HELPER_FILE_H
#define HALE_HELPER_FILE_H

#include "hale.h"
#include "hale_stream.h"
#include "hale_encoding.h"

namespace hale {

template<Encoding E>
struct ReadFile
{
    Vector<typename EncodingInfo<E>::Storage> out;
    err status;
};

template<Encoding In, Encoding Out>
ReadFile<Out>
read_file(const ch8* path)
{
    ReadFile<Out> ret;
    ret.status = 0;
    File f;
    if (open(&f, path, File::Read))
    {
        memi s = size(&f);

        memi in_s = s / sizeof(EncodingInfo<In>::Storage);
        Vector<EncodingInfo<In>::Storage> in_v;
        vector_init(&in_v, in_s);
        vector_resize(&in_v, in_s);

        vector_init(&ret.out, in_s);
        vector_resize(&ret.out, in_s);

        read(&f, (u8*)vector_begin(&in_v), s);

        EncodingInfo<In>::Storage *in, *in_;
        EncodingInfo<Out>::Storage *out, *out_;

        in = vector_begin(&in_v);
        in_ = vector_end(&in_v);
        out = vector_begin(&ret.out);
        out_ = vector_end(&ret.out);

        memi out_size = 0;
        CodecState cs = {};
        CodecReturn cr;
        for (;;)
        {
            if (in == in_) {
                break;
            }

            cr = codec<In, Out>(&in, in_, &out, out_, &cs);

            if (cr == CodecReturn::OutputUsed) {
                memi p = out - vector_begin(&ret.out);
                vector_resize(&ret.out, vector_count(ret.out) + 1024);
                out  = vector_begin(&ret.out) + p;
                out_ = vector_end(&ret.out);
            } else if (cr == CodecReturn::Error) {
                // TODO: Handle.
                hale_assert_message_debug(false, "Invalid character set sequence.");
            }
        }

        vector_resize(&ret.out, out - vector_begin(&ret.out));
        vector_release(&in_v);
        close(&f);

        ret.status = 1;
    }
    return ret;
}

} // namespace hale

#endif // HALE_HELPER_FILE_H

