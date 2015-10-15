#include "hale.h"
#include "hale_stream.h"
#include "hale_encoding.h"
#include "hale_perf.h"

namespace hale {

hale_internal void
test_utf8_decoder_speed()
{
    ch8 source[16384];
    ch16 destination[16384];

    memi s = 0;
    File in;
    if (open(&in, (ch8*)(__PROJECT__ "tests/encoding/utf8_greek_1.txt"), File::Read))
    {
        s = read(&in, source, hale_array_count(source));
        close(&in);
    }
    else
    {
        hale_panic("open");
    }

    CodecReturn cr;
    Codec<Encoding::UTF8, Encoding::Hale> c;

    c.s = {};

    c.out  = destination;
    c.out_ = destination + hale_array_count(destination);
    c.in  = source;
    c.in_ = source + s;

    qDebug() << "begin";
    {
        HALE_PERFORMANCE_TIMER(decoder_time);
        for (int i = 0; i < 100000; i++)
        {
            cr = codec(&c);
            c.out  = destination;
            c.in   = source;
            c.s    = {};
        }
    }
    qDebug() << "end";

}

//
//
//

#define _test_decoder(from)\
    Codec<from, Encoding::Hale> d;\
    d.s = {};\
    d.out  = (ch16*)destination;\
    d.out_ = (ch16*)(destination + (hale_array_count(destination)));\
    d.in  = (ch8*)&source;\
    d.in_ = (ch8*)&source + (hale_array_count(source));\
    CodecReturn e = codec(&d)

hale_internal void
test_utf8_decoder_new_lines()
{
    {   ch16 destination[] = {0, 0, 0, 0, 0, 0, 0, 0};
        ch8  source[] = {'\n','\r','\r','\n'};
        _test_decoder(Encoding::UTF8);
        hale_test(d.in  == source + 4);
        hale_test(d.out == destination + 3);
        hale_test(e     == CodecReturn::InputPending);
        hale_test(destination[0] == L'\n');
        hale_test(destination[1] == L'\n');
        hale_test(destination[2] == L'\n');
        hale_test(destination[3] == 0);
        hale_test(destination[4] == 0);
        hale_test(destination[5] == 0);
        hale_test(destination[6] == 0);
        hale_test(destination[7] == 0);
    }
}

hale_internal void
test_utf8_decoder_tiny_buffers()
{
    {   ch16 destination[] = {0, 0, 0};
        ch8  source[] = {'a'};
        _test_decoder(Encoding::UTF8);
        hale_test(d.in  == source + 1);
        hale_test(d.out == destination + 1);
        hale_test(e    == CodecReturn::InputPending);
        hale_test(destination[0] == L'a');
        hale_test(destination[1] == 0);
    }

    {   ch16 destination[] = {0, 0};
        ch8  source[] = {0xC2, 0xA2};
        _test_decoder(Encoding::UTF8);
        hale_test(d.in  == source + 2);
        hale_test(d.out == destination + 1);
        hale_test(e    == CodecReturn::OutputUsed);
        hale_test(destination[0] == 0x00A2);
        hale_test(destination[1] == 0);
    }

    {   ch16 destination[] = {0, 0};
        ch8  source[] = {0xE2, 0x82, 0xAC};
        _test_decoder(Encoding::UTF8);
        hale_test(d.in  == source + 3);
        hale_test(d.out == destination + 1);
        hale_test(e    == CodecReturn::OutputUsed);
        hale_test(destination[0] == 0x20AC);
        hale_test(destination[1] == 0);
    }


    {   ch16 destination[] = {0, 0};
        ch8  source[] = {0xF0, 0x90, 0x8D, 0x88};
        _test_decoder(Encoding::UTF8);
        hale_test(d.in  == source + 4);
        hale_test(d.out == destination + 2);
        hale_test(e    == CodecReturn::OutputUsed);
        hale_test(destination[0] == 0xd800);
        hale_test(destination[1] == 0xdf48);
    }

    //
    // Partials
    //


    // Partial destination
    {   ch16 destination[] = {0, 0};
        ch8  source[] = {'a', 0xF0, 0x90, 0x8D, 0x88};
        _test_decoder(Encoding::UTF8);
        hale_test(d.in  == source + 1);
        hale_test(d.out == destination + 1);
        hale_test(e    == CodecReturn::OutputUsed);
        hale_test(destination[0] == L'a');
        hale_test(destination[1] == 0);

        // Continue with new buffer.

        d.out = destination;
        e = codec(&d);
        hale_test(d.in  == source + 5);
        hale_test(d.out == destination + 2);
        hale_test(e    == CodecReturn::OutputUsed);
        hale_test(destination[0] == 0xd800);
        hale_test(destination[1] == 0xdf48);
    }

    // Partial source (2+2 bytes)
    {   ch16 destination[] = {0, 0};
        ch8  source[] = {0xF0, 0x90};
        _test_decoder(Encoding::UTF8);
        hale_test(d.in  == source + 2);
        hale_test(d.out == destination);
        // e wouldn't be set to success, as d.utf8.state == 36; (36/12)-1 = 3-1 = 2; 2 more characters expected.
        hale_test(e    == CodecReturn::InputPending);
        hale_test(destination[0] == 0);
        hale_test(destination[1] == 0);

        // Keep D, set source to a new buffer
        ch8 source2[] = {0x8D, 0x88};
        d.in  = source2;
        d.in_ = source2 + 2;
        e = codec(&d);
        hale_test(d.in  == source2 + 2);
        hale_test(d.out == destination + 2);
        hale_test(e     == CodecReturn::OutputUsed);
        hale_test(destination[0] == 0xd800);
        hale_test(destination[1] == 0xdf48);
    }

    // Partial source (1+1+1 bytes)
    {   ch16 destination[] = {0, 0};
        ch8  source[] = {0xE2};
        _test_decoder(Encoding::UTF8);
        hale_test(d.in  == source + 1);
        hale_test(d.out == destination);
        hale_test(d.s.input_state == 36);
        // e wouldn't be set to 1, as d.utf8.state == 36; (36/12)-1 = 3-1 = 2; 2 more characters expected.
        hale_test(e    == CodecReturn::InputPending);
        hale_test(destination[0] == 0);
        hale_test(destination[1] == 0);

        // Keep D, set source to a new buffer
        ch8 source2[] = {0x82};
        d.in  = source2;
        d.in_ = source2 + 1;
        e = codec(&d);
        hale_test(d.in  == source2 + 1);
        hale_test(d.out == destination);
        hale_test(d.s.input_state == 24);
        // e wouldn't be set to 1, as d.utf8.state == 24; (24/12)-1 = 2-1 = 1; 1 more character expected.
        hale_test(e    == CodecReturn::InputPending);
        hale_test(destination[0] == 0);
        hale_test(destination[1] == 0);

        ch8 source3[] = {0xAC};
        d.in  = source3;
        d.in_ = source3 + 1;
        e = codec(&d);
        hale_test(d.in  == source3 + 1);
        hale_test(d.out == destination + 1);
        hale_test(e     == CodecReturn::OutputUsed);
        hale_test(destination[0] == 0x20AC);
    }
}

void
test_utf8_decoder_large_buffers()
{
    ch8 source[128];
    ch16 destination[128];

    Codec<Encoding::UTF8, Encoding::Hale> d;
    d.s = {};

    d.out  = destination;
    d.out_ = destination + hale_array_count(destination);

    // Emulate InputDrain
    d.in  = source;
    d.in_ = source;

    CodecReturn cr;
    memi s;

    File in;
    File out;
    hale_assert(open(&in,  (ch8*)(__PROJECT__ "tests/encoding/utf8_greek_1.txt"), File::Read));
    hale_assert(open(&out, (ch8*)(__PROJECT__ "tests/encoding/hale_greek_1.txt"), File::Write));
    write(&out, (u8*)"\xFF\xFE", 2);
    for (;;)
    {
        if (d.in == d.in_) {
            s = read(&in, source, hale_array_count(source));
            qDebug() << "Reading input" << s;
            if (s == 0) {
                break;
            }
            d.in  = source;
            d.in_ = d.in + s;
        }

        cr = codec(&d);


        switch (cr)
        {
        case CodecReturn::OutputUsed:
            s = d.out - destination;
            qDebug() << "Writing output" << s;
            write(&out, (u8*)destination, s*sizeof(u16));
            d.out  = destination;
            break;
        case CodecReturn::Error:
            hale_assert_message_debug(false, "Invalid character set sequence.");
            break;
        }
    }

    s = d.out - destination;
    if (s) {
        qDebug() << "Writing output (tail)" << s;
        write(&out, (u8*)destination, s*sizeof(u16));
    }

    close(&in);
    close(&out);
}

//
// Encoding
//

void
test_utf_boundaries()
{
    ch8 source[4096] = {};
    ch16 destination[4096] = {};
    ch16 reference[4096] = {};

    ch8  *in   = source;
    ch8  *in_  = in;

    ch16 *out  = destination;
    ch16 *out_ = out + hale_array_count(destination);

    ch16 *ref  = reference;
    ch16 *ref_ = ref;

    File inf;
    hale_assert(open(&inf, (ch8*)(__PROJECT__ "tests/encoding/utf8.txt"), File::Read));
    seek(&inf, 3);
    in_ += read(&inf, in, hale_array_count(source));
    close(&inf);

    CodecState s = {};
    auto cr = codec<Encoding::UTF8, Encoding::Hale>(&in, in_, &out, out_, &s);
    hale_assert(cr == CodecReturn::InputPending);

    hale_assert(open(&inf,(ch8*)(__PROJECT__ "tests/encoding/utf16le.txt"), File::Read));
    seek(&inf, 2);
    ref_ += read(&inf, ref, hale_array_count(reference) * sizeof(u16)) / sizeof(u16);
    close(&inf);

    memi rl = ref_ - ref;
    memi ol = out - destination;
    out = destination;
    hale_test((rl) == (ol));
    for (memi i = 0; i < (ol); i++)
    {
        u16 o = *out++;
        u16 r = *ref++;
        hale_test(o == r);
    }
}

// Writes codepoints around known UTF-8 and UTF-16 boundaries (+-0xF)
void
write_utf32le()
{
    File out;
    if (open(&out, (ch8*)(__PROJECT__ "tests/encoding/utf32le.txt"), File::Write))
    {
#if 0
        // Full 31-bit.
        u32 boundaries[] = {0x80, 0x800, 0x10000, 0x10FFFF, 0x200000, 0x4000000, 0x80000000};
#else
        // Restricted to RFC 3629 (0x10FFFF)
        u32 boundaries[] = {
            // UTF-8 2-byte boundary.
            0x000080,
            // UTF-8 3-byte boundary.
            0x000800,
            // UTF-8  4-byte boundary.
            // UTF-16 2-word boundary.
            0x010000,
            // Maximum Unicode codepoint.
            0x10FFFF
        };
#endif
        u32 nl = '\n';
        u32 sp = ' ';
        write(&out, "\xFF\xFE\x00\x00", 4);
        for (memi i = 0; i < hale_array_count(boundaries); i++)
        {
            for (memi j = boundaries[i]-0xF; j != boundaries[i]+0xF; j++)
            {
                if (j <= 0x0010FFFF) {
                    write(&out, (u8*)&j, sizeof(u32));
                    write(&out, (u8*)&sp, sizeof(u32));
                }
            }
            write(&out, (u8*)&nl, sizeof(u32));
        }
        close(&out);
    }
}

void
test_encoding()
{    
    // write_utf32le();



    test_utf8_decoder_tiny_buffers();
    test_utf_boundaries();
//    test_utf8_decoder_new_lines();
//    test_utf8_decoder_large_buffers();
//    test_utf8_decoder_speed();
}

} // namespace hale
