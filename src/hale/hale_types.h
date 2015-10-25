#ifndef HALE_TYPES_H
#define HALE_TYPES_H

#include <stdint.h>
#include <float.h>

namespace hale {

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef intptr_t sptr;
typedef uintptr_t uptr;

typedef size_t memi;

typedef float r32;
typedef double r64;

typedef int8 s8;
typedef int8 s08;
typedef int16 s16;
typedef int32 s32;
typedef int64 s64;
typedef bool32 b32;

typedef uint8 u8;
typedef uint8 u08;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

#define HALE_R32_MAX (FLT_MAX)

static_assert(sizeof(wchar_t) == sizeof(u16), "wchar_t");
static_assert(sizeof(char) == sizeof(u8), "char");

typedef u8  ch8;
typedef u16 ch16;
typedef u32 ch32;

typedef ch16    ch;

typedef u32     err;

// TODO:
//struct ch {
//    u16 unicode;

//    bool operator ==(const ch &other) const {
//        return unicode == other.unicode;
//    }
//};

//
// Type-specific utilities.
//

inline u32
safe_truncate_u64(u64 val)
{
    hale_assert(val <= 0xFFFFFFFF);
    return (u32)val;
}

} // namespace hale

#endif // HALE_TYPES_H

