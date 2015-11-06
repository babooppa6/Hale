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

#define HALE_R32_MAX  (FLT_MAX)
#define HALE_R64_MAX  (DBL_MAX)

#define HALE_S16_MAX  (INT16_MAX)
#define HALE_U16_MAX  (UINT16_MAX)

#define HALE_S32_MAX  (INT32_MAX)
#define HALE_U32_MAX  (UINT32_MAX)

#define HALE_S64_MAX  (INT64_MAX)
#define HALE_U64_MAX  (UINT64_MAX)

#define HALE_MEMI_MAX (SIZE_MAX)

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

} // namespace hale

#endif // HALE_TYPES_H

