#ifndef HALE_FTOA_H
#define HALE_FTOA_H

#if HALE_INCLUDES
#include "hale_types.h"
#include "hale_itoa.h"
#endif

#include <math.h>
#include <stdio.h>
// - swprintf

namespace hale {


#if 0
//All of the following is in the public domain, or CC-0 where that's not possible.
//Do with it what you want to. It's untested, but it may just do what you need it to do.
//If not, that's a shame. Improvements welcome.

//This was originally written by me some 25 years ago as part of an arbitrary precision
//set of routines. Float64 has less precision, but it appears to be stable over the
//expected range nonetheless, according to Andrew Chronister's play around with it.

//Cheers,
//Jeroen van Rijn (Twitter: @J_vanRijn / Twitch: @Kelimion)

enum FtoAFlags
{
    FtoAFlags_Sign = 0x01,
    FtoAFlags_SignIfPositive = 0x02
};

struct PrintF
{
    template<typename Ch>
    void simple_print_float(r64 number, u16 digits, u8 flags, Ch *out) {

        // The range of a float64 is approx +/- 1.7976931348623157 × 10^308
        // For the whole part, we need at most log10(number)+1 digits
        // in the output string to represent it:
        // log10(1000) = 3, log10(10000) = 4
        // Given the max range tops out at about 10^308, you're unlikely to need more than
        // (+/-) = 1 + 308 + (.) 1 + 30 (how many digits does one really need?) + (\0)
        // So in theory you could preallocate a zero-filled 350 bytes or so, fill as needed,
        // and return the first 'n' actually used. But, we can be smarter about it.
        //
        // To know how large the output string needs to be, we need this, in order:
        // output_length = 0;
        // if (sign && (sign_if_positive || signbit(number))) output_length += 1;
        // output_length = log10(abs(number)) + 1;
        // output_length += 1; // add one for the decimal point
        // output_length += digits;
        // output_length += 1; // \0 string terminator
        //
        // This way, we can predetermine the needed space and write to the string in place,
        // in whatever is the quickest way to do so.
        //
        // Exercise for the reader:
        // If you want thousand separators, reserve space for another (log10(number) / 3)
        //

        // the rest here is pseudocode-ish. This is all just to demonstrate the general idea

        s32 sign_length = 0;
        if (flags & FtoAFlags_Sign)
        {
            if (signbit(number)) {
                sign_length = 1;
                //write_sign_to_string('-');
                *out++ = '-';
            } else if (flags & FtoAFlags_SignIfPositive) {
                sign_length = 1;
                // write_sign_to_string('+');
                *out++ = '+';
            }
        }

        // Why trunc and not round, floor or ceil? http://www.cplusplus.com/reference/cmath/trunc/
        r64 whole_part = trunc(abs(number));
        r64 fract_part = number - whole_part;
        s32 log10_count = log10(whole_part); // 0-based, so log10(1000) = 3 -> 0..3 works fine
        s32 location = string_base + log10_count + sign_length;
        s32 decimal_location = location + 1;
        do {
            d = whole_part % 10;  // alternatively, d = fmod(whole_part, 10);
            // even more alternatively, you could divide the whole number part
            // by keeping a separate divisor that increases each time around
            // so as not to accumulate error.
            whole_part /= 10;

            // add digit to string at string index log_count
            // *(string_base + location)-- = d+0x30 in an optimised version?
            *out++ =
            print_current_digit_to_string(d, location--);
        // because n < 10 will be log(0), we need to go through this at least once
        // hence also post-decrement
        } while(log10_count-- > 0)

        print_decimal_to_string(decimal_location);

        // And now multiply the fractional part by 10 for as many digits as you want,
        // clamp to integer with trunc, and write to the string as well
        location = decimal_location++
        if (digits) { // since if you want none, no need for this rigmarole
            for (digits-- > 0) {
                fract_parth *= 10;
                d = trunc(fract_part);
                print_current_digit_to_string(d, location);
                location++;
            }
        }
        // if you didn't pre-fill with zeroes
        print_zero_terminator_to_string(location);
    }

    memi estimate_length(r64 number, u16 digits, b32 sign, bool32 sign_if_positive)
    {
        output_length = 0;
        output_length += (sign && (sign_if_positive || signbit(number)));
        output_length = log10(abs(number)) + 1;
        output_length += 1; // add one for the decimal point
        output_length += digits;
        output_length += 1; // \0 string terminator
    }
}
#endif

// TODO: We really need our own implementation ^^^.

inline memi
ftoa_estimate(r64)
{
    return 32;
}

inline ch16*
ftoa(r64 value, ch16 *buffer)
{
    return buffer + swprintf((wchar_t*)buffer, 32, L"%.16g", value) - 1;
}

#define HALE_IMPLEMENT_SINK_FTOA(FtoaT) \
    template<typename SinkT> \
    inline SinkT &sink(SinkT &sink, FtoaT value) \
    { \
        auto ptr = sink.push(0, ftoa_estimate(value)); \
        auto end = ftoa(value, ptr); \
        sink.push(end-ptr, 0); \
        return sink; \
    }

HALE_IMPLEMENT_SINK_FTOA(r64)
HALE_IMPLEMENT_SINK_FTOA(r32)

}

#endif // HALE_FTOA_H

