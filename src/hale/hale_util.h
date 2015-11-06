#ifndef HALE_UTIL_H
#define HALE_UTIL_H

namespace hale {

inline u32
safe_truncate_u64(u64 val)
{
    hale_assert(val <= 0xFFFFFFFF);
    return (u32)val;
}

// TODO: Optimize `equal` (SIMD, loop unwind)

template<typename T>
b32 equal(const T *a, const T *a_, const T *b0)
{
    hale_assert_requirement(a <= a_);

    while (a != a_ && (*b0) != 0)
    {
        if (*a != *b0) {
            return false;
        }
        a++;
        b0++;
    }
    return a == a_ && (*b0) == 0;
}

// TODO: Benchmark using memcmp

template<typename T>
inline b32
equal(const T *a, const T *a_, const T *b, const T *b_)
{
    hale_assert_requirement(a <= a_);
    hale_assert_requirement(b <= b_);

    if ((a_ - a) != (b_ - b)) {
        return false;
    }

    while (a != a_ && b != b_)
    {
        if (*a != *b) {
            return false;
        }
        a++;
        b++;
    }
    return true;
}

#if 0

inline b32
vector_equal(Vector<ch16> *av, const ch16 *b)
{
    // TODO: Optimize
    ch16 *a  = vector_data(av);
    ch16 *a_ = a + vector_count(av);
    while (a != a_ && (*b) != 0 && *a == *b) {
        a++;
        b++;
    }
    return a == a_ && (*b) == 0;
}

inline b32
vector_equal(Vector<ch8> *av, const ch8 *b)
{
    // TODO: Optimize
    ch8 *a  = vector_data(av);
    ch8 *a_ = a + vector_count(av);
    while (a != a_ && (*b) != 0 && *a == *b) {
        a++;
        b++;
    }
    return a == a_ && (*b) == 0;
}

#include <cstring>

inline b32
vector_equal(Vector<ch8> *av, const ch8 *b, memi b_count)
{
    // TODO: Optimize
    ch8 *a  = vector_data(av);
    ch8 *a_ = a + vector_count(av);
    const ch8 *b_ = b + b_count;
    while (a != a_ && b != b_ && *a == *b) {
        a++;
        b++;
    }
    return a == a_ && a == a_;
}

#endif

} // namspace hale

#endif // HALE_UTIL_H
