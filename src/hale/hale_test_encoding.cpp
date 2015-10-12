#include "hale.h"
#include "hale_stream.h"
#include "hale_encoding.h"
#include "hale_perf.h"

namespace hale {

hale_internal void
test_utf8_decoder_speed()
{
    u8 source[16384];
    u16 destination[16384];

    memi s = 0;
    File in;
    if (open(&in, __WPROJECT__ L"tests/encoding/utf8_greek_1.txt", File::Read))
    {
        s = read(&in, source, hale_array_count(source));
        close(&in);
    }
    else
    {
        hale_panic("open");
    }

    CodecReturn cr;
    Decode d;

    d.utf8 = {};
    d.hale = {};

    d.out  = destination;
    d.out_ = destination + hale_array_count(destination);
    d.in  = source;
    d.in_ = source + s;

    qDebug() << "begin";
    {
        HALE_PERFORMANCE_TIMER(decoder);
        for (int i = 0; i < 100000; i++)
        {
            cr = decoder_utf8(&d);
            d.out  = destination;
            d.in  = source;
            d.utf8 = {};
            d.hale = {};
        }
    }
    qDebug() << "end";

}

//
//
//

#define _test_decoder(decoder)\
    Decode d;\
    d.utf8 = {};\
    d.hale = {};\
    d.out  = (u16*)destination;\
    d.out_ = (u16*)(destination + (hale_array_count(destination)));\
    d.in  = (u8*)&source;\
    d.in_ = (u8*)&source + (hale_array_count(source));\
    CodecReturn e = decoder(&d)

hale_internal void
test_utf8_decoder_new_lines()
{
    {   u16 destination[] = {0, 0, 0, 0, 0, 0, 0, 0};
        u8  source[] = {'\n','\r','\r','\n'};
        _test_decoder(decoder_utf8);
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
    {   u16 destination[] = {0, 0, 0};
        u8  source[] = {'a'};
        _test_decoder(decoder_utf8);
        hale_test(d.in  == source + 1);
        hale_test(d.out == destination + 1);
        hale_test(e    == CodecReturn::InputPending);
        hale_test(destination[0] == L'a');
        hale_test(destination[1] == 0);
    }

    {   u16 destination[] = {0, 0};
        u8  source[] = {0xC2, 0xA2};
        _test_decoder(decoder_utf8);
        hale_test(d.in  == source + 2);
        hale_test(d.out == destination + 1);
        hale_test(e    == CodecReturn::OutputUsed);
        hale_test(destination[0] == 0x00A2);
        hale_test(destination[1] == 0);
    }

    {   u16 destination[] = {0, 0};
        u8  source[] = {0xE2, 0x82, 0xAC};
        _test_decoder(decoder_utf8);
        hale_test(d.in  == source + 3);
        hale_test(d.out == destination + 1);
        hale_test(e    == CodecReturn::OutputUsed);
        hale_test(destination[0] == 0x20AC);
        hale_test(destination[1] == 0);
    }


    {   u16 destination[] = {0, 0};
        u8  source[] = {0xF0, 0x90, 0x8D, 0x88};
        _test_decoder(decoder_utf8);
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
    {   u16 destination[] = {0, 0};
        u8  source[] = {'a', 0xF0, 0x90, 0x8D, 0x88};
        _test_decoder(decoder_utf8);
        hale_test(d.in  == source + 1);
        hale_test(d.out == destination + 1);
        hale_test(e    == CodecReturn::OutputUsed);
        hale_test(destination[0] == L'a');
        hale_test(destination[1] == 0);

        // Continue with new buffer.

        d.out = destination;
        e = decoder_utf8(&d);
        hale_test(d.in  == source + 5);
        hale_test(d.out == destination + 2);
        hale_test(e    == CodecReturn::OutputUsed);
        hale_test(destination[0] == 0xd800);
        hale_test(destination[1] == 0xdf48);
    }

    // Partial source (2+2 bytes)
    {   u16 destination[] = {0, 0};
        u8  source[] = {0xF0, 0x90};
        _test_decoder(decoder_utf8);
        hale_test(d.in  == source + 2);
        hale_test(d.out == destination);
        // e wouldn't be set to success, as d.utf8.state == 36; (36/12)-1 = 3-1 = 2; 2 more characters expected.
        hale_test(e    == CodecReturn::InputPending);
        hale_test(destination[0] == 0);
        hale_test(destination[1] == 0);

        // Keep D, set source to a new buffer
        u8 source2[] = {0x8D, 0x88};
        d.in  = source2;
        d.in_ = source2 + 2;
        e = decoder_utf8(&d);
        hale_test(d.in  == source2 + 2);
        hale_test(d.out == destination + 2);
        hale_test(e     == CodecReturn::OutputUsed);
        hale_test(destination[0] == 0xd800);
        hale_test(destination[1] == 0xdf48);
    }

    // Partial source (1+1+1 bytes)
    {   u16 destination[] = {0, 0};
        u8  source[] = {0xE2};
        _test_decoder(decoder_utf8);
        hale_test(d.in  == source + 1);
        hale_test(d.out == destination);
        hale_test(d.utf8.state == 36);
        // e wouldn't be set to 1, as d.utf8.state == 36; (36/12)-1 = 3-1 = 2; 2 more characters expected.
        hale_test(e    == CodecReturn::InputPending);
        hale_test(destination[0] == 0);
        hale_test(destination[1] == 0);

        // Keep D, set source to a new buffer
        u8 source2[] = {0x82};
        d.in  = source2;
        d.in_ = source2 + 1;
        e = decoder_utf8(&d);
        hale_test(d.in  == source2 + 1);
        hale_test(d.out == destination);
        hale_test(d.utf8.state == 24);
        // e wouldn't be set to 1, as d.utf8.state == 24; (24/12)-1 = 2-1 = 1; 1 more character expected.
        hale_test(e    == CodecReturn::InputPending);
        hale_test(destination[0] == 0);
        hale_test(destination[1] == 0);

        u8 source3[] = {0xAC};
        d.in  = source3;
        d.in_ = source3 + 1;
        e = decoder_utf8(&d);
        hale_test(d.in  == source3 + 1);
        hale_test(d.out == destination + 1);
        hale_test(e     == CodecReturn::OutputUsed);
        hale_test(destination[0] == 0x20AC);
    }
}

void
test_utf8_decoder_large_buffers()
{
    u8 source[128];
    u16 destination[128];

    Decode d;
    d.utf8 = {};

    d.out  = destination;
    d.out_ = destination + hale_array_count(destination);

    // Emulate InputDrain
    d.in  = source;
    d.in_ = source;

    CodecReturn cr;
    memi s;

    File in;
    File out;
    hale_assert(open(&in,  __WPROJECT__ L"tests/encoding/utf8_greek_1.txt", File::Read));
    hale_assert(open(&out, __WPROJECT__ L"tests/encoding/hale_greek_1.txt", File::Write));
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

        cr = decoder_utf8(&d);


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
test_utf8_encoder_tiny_buffers()
{
    {   u16 source[] = {0x70,  0x700, 0xF000, 0x1F000, 0x3F00000, 0x7F000000, 0x80000001};
        u8  destination[] = {0, 0, 0, 0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0};

        Encode e;
        e.in = source;
        e.in_ = source + hale_array_count(source);
        e.out = destination;
        e.out_ = destination + hale_array_count(destination);
        e.hale = {};

        encoder_utf8(&e);
        hale_test(destination[0] == 0x70);
    }

    {   u16 source[] = {0xd800, 0xdf48};
        // 0xF0, 0x90, 0x8D, 0x88
        u8  destination[] = {0, 0, 0, 0, 0, 0, 0, 0};

        Encode e;
        e.in = source;
        e.in_ = source + hale_array_count(source);
        e.out = destination;
        e.out_ = destination + hale_array_count(destination);
        e.hale = {};

        encoder_utf8(&e);
    }
}

void
test_encoding()
{
    test_utf8_encoder_tiny_buffers();

    // test_utf8_decoder_tiny_buffers();
    // test_utf8_decoder_new_lines();
    // test_utf8_decoder_large_buffers();
    // test_utf8_decoder_speed();
}

} // namespace hale
