#ifndef HALE_ATOI_H
#define HALE_ATOI_H

// See the LICENSE.md
// Adopted from:
// https://github.com/miloyip/itoa-benchmark

namespace hale {

namespace itoa_impl {
    struct Naive
    {
        template<typename Ch>
        static inline Ch *
        itoa(u32 value, Ch* buffer)
        {
            Ch temp[10];
            Ch *p = temp;
            do {
                *p++ = Ch(value % 10) + '0';
                value /= 10;
            } while (value > 0);

            do {
                *buffer++ = *--p;
            } while (p != temp);

            return buffer;
        }

        template<typename Ch>
        static inline Ch *
        itoa(s32 value, Ch* buffer)
        {
            u32 u = static_cast<u32>(value);
            if (value < 0) {
                *buffer++ = '-';
                u = ~u + 1;
            }
            return itoa(u, buffer);
        }

        template<typename Ch>
        static inline Ch *
        itoa(u64 value, Ch* buffer)
        {
            Ch temp[20];
            Ch *p = temp;
            do {
                *p++ = Ch(value % 10) + Ch('0');
                value /= 10;
            } while (value > 0);

            do {
                *buffer++ = *--p;
            } while (p != temp);

            return buffer;
        }

        template<typename Ch>
        static inline Ch *
        itoa(s64 value, Ch* buffer)
        {
            u64 u = static_cast<u64>(value);
            if (value < 0) {
                *buffer++ = '-';
                u = ~u + 1;
            }
            return itoa(u, buffer);
        }
    };

    struct BranchLut
    {
        // Branching for different cases (forward)
        // Use lookup table of two digits

        static const char gDigitsLut[200];

        template<typename Ch>
        static inline Ch *
        itoa(u32 value, Ch* buffer)
        {
            if (value < 10000) {
                const u32 d1 = (value / 100) << 1;
                const u32 d2 = (value % 100) << 1;

                if (value >= 1000)
                    *buffer++ = gDigitsLut[d1];
                if (value >= 100)
                    *buffer++ = gDigitsLut[d1 + 1];
                if (value >= 10)
                    *buffer++ = gDigitsLut[d2];
                *buffer++ = gDigitsLut[d2 + 1];
            }
            else if (value < 100000000)
            {
                // value = bbbbcccc
                const u32 b = value / 10000;
                const u32 c = value % 10000;

                const u32 d1 = (b / 100) << 1;
                const u32 d2 = (b % 100) << 1;

                const u32 d3 = (c / 100) << 1;
                const u32 d4 = (c % 100) << 1;

                if (value >= 10000000)
                    *buffer++ = gDigitsLut[d1];
                if (value >= 1000000)
                    *buffer++ = gDigitsLut[d1 + 1];
                if (value >= 100000)
                    *buffer++ = gDigitsLut[d2];
                *buffer++ = gDigitsLut[d2 + 1];

                *buffer++ = gDigitsLut[d3];
                *buffer++ = gDigitsLut[d3 + 1];
                *buffer++ = gDigitsLut[d4];
                *buffer++ = gDigitsLut[d4 + 1];
            }
            else
            {
                // value = aabbbbcccc in decimal

                const u32 a = value / 100000000; // 1 to 42
                value %= 100000000;

                if (a >= 10)
                {
                    const unsigned i = a << 1;
                    *buffer++ = gDigitsLut[i];
                    *buffer++ = gDigitsLut[i + 1];
                }
                else
                {
                    *buffer++ = '0' + static_cast<Ch>(a);
                }

                const u32 b = value / 10000; // 0 to 9999
                const u32 c = value % 10000; // 0 to 9999

                const u32 d1 = (b / 100) << 1;
                const u32 d2 = (b % 100) << 1;

                const u32 d3 = (c / 100) << 1;
                const u32 d4 = (c % 100) << 1;

                *buffer++ = gDigitsLut[d1];
                *buffer++ = gDigitsLut[d1 + 1];
                *buffer++ = gDigitsLut[d2];
                *buffer++ = gDigitsLut[d2 + 1];
                *buffer++ = gDigitsLut[d3];
                *buffer++ = gDigitsLut[d3 + 1];
                *buffer++ = gDigitsLut[d4];
                *buffer++ = gDigitsLut[d4 + 1];
            }
            return buffer;
        }

        template<typename Ch>
        static inline Ch *
        itoa(s32 value, Ch* buffer)
        {
            u32 u = static_cast<u32>(value);
            if (value < 0) {
                *buffer++ = '-';
                u = ~u + 1;
            }

            return itoa(u, buffer);
        }

        template<typename Ch>
        static inline Ch *
        itoa(u64 value, Ch* buffer)
        {
            if (value < 100000000)
            {
                u32 v = static_cast<u32>(value);
                if (v < 10000) {
                    const u32 d1 = (v / 100) << 1;
                    const u32 d2 = (v % 100) << 1;

                    if (v >= 1000)
                        *buffer++ = gDigitsLut[d1];
                    if (v >= 100)
                        *buffer++ = gDigitsLut[d1 + 1];
                    if (v >= 10)
                        *buffer++ = gDigitsLut[d2];
                    *buffer++ = gDigitsLut[d2 + 1];
                }
                else
                {
                    // value = bbbbcccc
                    const u32 b = v / 10000;
                    const u32 c = v % 10000;

                    const u32 d1 = (b / 100) << 1;
                    const u32 d2 = (b % 100) << 1;

                    const u32 d3 = (c / 100) << 1;
                    const u32 d4 = (c % 100) << 1;

                    if (value >= 10000000)
                        *buffer++ = gDigitsLut[d1];
                    if (value >= 1000000)
                        *buffer++ = gDigitsLut[d1 + 1];
                    if (value >= 100000)
                        *buffer++ = gDigitsLut[d2];
                    *buffer++ = gDigitsLut[d2 + 1];

                    *buffer++ = gDigitsLut[d3];
                    *buffer++ = gDigitsLut[d3 + 1];
                    *buffer++ = gDigitsLut[d4];
                    *buffer++ = gDigitsLut[d4 + 1];
                }
            }
            else if (value < 10000000000000000)
            {
                const u32 v0 = static_cast<u32>(value / 100000000);
                const u32 v1 = static_cast<u32>(value % 100000000);

                const u32 b0 = v0 / 10000;
                const u32 c0 = v0 % 10000;

                const u32 d1 = (b0 / 100) << 1;
                const u32 d2 = (b0 % 100) << 1;

                const u32 d3 = (c0 / 100) << 1;
                const u32 d4 = (c0 % 100) << 1;

                const u32 b1 = v1 / 10000;
                const u32 c1 = v1 % 10000;

                const u32 d5 = (b1 / 100) << 1;
                const u32 d6 = (b1 % 100) << 1;

                const u32 d7 = (c1 / 100) << 1;
                const u32 d8 = (c1 % 100) << 1;

                if (value >= 1000000000000000)
                    *buffer++ = gDigitsLut[d1];
                if (value >= 100000000000000)
                    *buffer++ = gDigitsLut[d1 + 1];
                if (value >= 10000000000000)
                    *buffer++ = gDigitsLut[d2];
                if (value >= 1000000000000)
                    *buffer++ = gDigitsLut[d2 + 1];
                if (value >= 100000000000)
                    *buffer++ = gDigitsLut[d3];
                if (value >= 10000000000)
                    *buffer++ = gDigitsLut[d3 + 1];
                if (value >= 1000000000)
                    *buffer++ = gDigitsLut[d4];
                if (value >= 100000000)
                    *buffer++ = gDigitsLut[d4 + 1];

                *buffer++ = gDigitsLut[d5];
                *buffer++ = gDigitsLut[d5 + 1];
                *buffer++ = gDigitsLut[d6];
                *buffer++ = gDigitsLut[d6 + 1];
                *buffer++ = gDigitsLut[d7];
                *buffer++ = gDigitsLut[d7 + 1];
                *buffer++ = gDigitsLut[d8];
                *buffer++ = gDigitsLut[d8 + 1];
            }
            else
            {
                const u32 a = static_cast<u32>(value / 10000000000000000); // 1 to 1844
                value %= 10000000000000000;

                if (a < 10)
                    *buffer++ = '0' + static_cast<Ch>(a);
                else if (a < 100) {
                    const u32 i = a << 1;
                    *buffer++ = gDigitsLut[i];
                    *buffer++ = gDigitsLut[i + 1];
                }
                else if (a < 1000) {
                    *buffer++ = '0' + static_cast<Ch>(a / 100);

                    const u32 i = (a % 100) << 1;
                    *buffer++ = gDigitsLut[i];
                    *buffer++ = gDigitsLut[i + 1];
                }
                else {
                    const u32 i = (a / 100) << 1;
                    const u32 j = (a % 100) << 1;
                    *buffer++ = gDigitsLut[i];
                    *buffer++ = gDigitsLut[i + 1];
                    *buffer++ = gDigitsLut[j];
                    *buffer++ = gDigitsLut[j + 1];
                }

                const u32 v0 = static_cast<u32>(value / 100000000);
                const u32 v1 = static_cast<u32>(value % 100000000);

                const u32 b0 = v0 / 10000;
                const u32 c0 = v0 % 10000;

                const u32 d1 = (b0 / 100) << 1;
                const u32 d2 = (b0 % 100) << 1;

                const u32 d3 = (c0 / 100) << 1;
                const u32 d4 = (c0 % 100) << 1;

                const u32 b1 = v1 / 10000;
                const u32 c1 = v1 % 10000;

                const u32 d5 = (b1 / 100) << 1;
                const u32 d6 = (b1 % 100) << 1;

                const u32 d7 = (c1 / 100) << 1;
                const u32 d8 = (c1 % 100) << 1;

                *buffer++ = gDigitsLut[d1];
                *buffer++ = gDigitsLut[d1 + 1];
                *buffer++ = gDigitsLut[d2];
                *buffer++ = gDigitsLut[d2 + 1];
                *buffer++ = gDigitsLut[d3];
                *buffer++ = gDigitsLut[d3 + 1];
                *buffer++ = gDigitsLut[d4];
                *buffer++ = gDigitsLut[d4 + 1];
                *buffer++ = gDigitsLut[d5];
                *buffer++ = gDigitsLut[d5 + 1];
                *buffer++ = gDigitsLut[d6];
                *buffer++ = gDigitsLut[d6 + 1];
                *buffer++ = gDigitsLut[d7];
                *buffer++ = gDigitsLut[d7 + 1];
                *buffer++ = gDigitsLut[d8];
                *buffer++ = gDigitsLut[d8 + 1];
            }

            return buffer;
        }

        template<typename Ch>
        static inline Ch *
        itoa(int64_t value, Ch* buffer)
        {
            u64 u = static_cast<u64>(value);
            if (value < 0) {
                *buffer++ = '-';
                u = ~u + 1;
            }

            return itoa(u, buffer);
        }
    };
} // itoa_impl

// Custom

typedef itoa_impl::Naive ItoaDefault;

template<typename I, typename Ch>
inline Ch *
itoa(I value, Ch *buffer) {
    return ItoaDefault::itoa(value, buffer);
}

#define HALE_IMPLEMENT_SINK_ITOA(ItoaT, BufSize) \
    template<typename SinkT> \
    inline SinkT &sink(SinkT &sink, ItoaT value) \
    { \
        auto ptr = sink.push(0, BufSize); \
        auto end = itoa(value, ptr); \
        sink.push(end-ptr, 0); \
        return sink; \
    }

HALE_IMPLEMENT_SINK_ITOA(s32, 25)
HALE_IMPLEMENT_SINK_ITOA(u32, 25)
HALE_IMPLEMENT_SINK_ITOA(u64, 25)
HALE_IMPLEMENT_SINK_ITOA(s64, 25)

} // namespace hale

#endif // HALE_ATOI_H

