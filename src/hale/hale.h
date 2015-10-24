#ifndef HALE_H
#define HALE_H

// #define HALE_PLATFORM_WIN32_GDI
#define HALE_PLATFORM_WIN32_DX

//
// Platforms
//

#define HALE_OS_WIN 1

//
//
//

#ifdef _DEBUG
#define HALE_DEBUG 1
#endif

// Enables additional testing and benchmarking code.
// #define HALE_TEST 1

// Enables additional asserts to check the consistency.
// A special variant of assert that can be removed in case the situation it
// checks is proved to be correct. It's recommended to be enabled in debug.
#if HALE_DEBUG
#define HALE_REQUIREMENT 1
#endif

//
// Statics
//

// Stuff that is internal in the module.
#define hale_internal static
// Local persistence stuff.
#define hale_persist static
// Global stuff.
#define hale_global static

//
// Panic
//

#define hale_panic_(message, file, line) *(int *)0 = 0
#define hale_panic(message)  hale_panic_(message, __FILE__, __LINE__)
#define hale_not_implemented hale_panic_("not implemented", __FILE__, __LINE__)
#define hale_not_tested      hale_panic_("not tested", __FILE__, __LINE__)
#define hale_not_reached     hale_panic_("not reached", __FILE__, __LINE__)

// TODO: Log and crash.
#define hale_error(message)  hale_panic(message)
// TODO: Log only.

//
// Assert
//

// General assert.
#define hale_assert(expression)\
    if(!(expression)) { hale_panic_("assert " #expression, __FILE__, __LINE__); }

#define hale_assert_message(expression, message)\
    if(!(expression)) { hale_panic_(message " (" #expression ")", __FILE__, __LINE__); }

// Assert for user input checking.
#define hale_assert_input(expression)\
    hale_assert(expression)

// Assert for internal consistency.
#ifdef HALE_REQUIREMENT
#define hale_assert_requirement(expression)\
    hale_assert(expression)
#else
#define hale_assert_requirement(expression)
#endif

// General assert for debug mode.
#if HALE_DEBUG
#define hale_assert_debug(expression)\
    hale_assert(expression)
#define hale_assert_message_debug(expression, message)\
    hale_assert_message(expression, message)
#define hale_debug(expression)\
    (expression)
#else
#define hale_assert_debug(expression)
#define hale_assert_message_debug(expression, message)
#define hale_debug(expression)
#endif

//
// Compiler macros
//

#define hale_unused(x) (void)x;

#ifdef __GNUC__
#define hale_deprecated __attribute__((deprecated))
#define hale_noinline   __attribute__((noinline))
#elif defined(_MSC_VER)
#define hale_deprecated __declspec(deprecated)
#define hale_noinline   __declspec(noinline)
#else
static_assert(
    sizeof(int) == -1,
    "hale_deprecated() not defined for this compiler"
);
#endif

//
// Utilities
//

template<typename T>
inline T
hale_minimum(T A, T B) { return A < B ? A : B; }

template<typename T>
inline T
hale_maximum(T A, T B) { return A > B ? A : B; }


#define hale_array_count(array) (sizeof(array) / sizeof((array)[0]))

#define hale_align_pow2(Value, Alignment) ((Value + ((Alignment) - 1)) & ~((Alignment) - 1))
#define hale_align_4(Value) ((Value + 3) & ~3)
#define hale_align_8(Value) ((Value + 7) & ~7)
#define hale_align_16(Value) ((Value + 15) & ~15)

//
// Common includes
//

#include "hale_custom.h"
#include "hale_types.h"
#include "hale_math.h"
#include "hale_math_interpolation.h"
#include "hale_util.h"

#include "hale_os.h"

#include "hale_vector.h"
#include "hale_string.h"
#include "hale_memory.h"

#include "hale_print.h"

#endif // HALE_H

