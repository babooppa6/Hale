#ifndef HALE_MACROS_H
#define HALE_MACROS_H

namespace hale
{

//#define hale_widen2(x) L ## x
//#define hale_widen(x) hale_widen2(x)
//#define __WFILE__ hale_widen(__FILE__)
//#define hale_tostring(x)  hale_stringify(x)
//#define hale_towstring(x) hale_widen(hale_tostring(x))

#define hale_stringify_(x) #x
#define hale_stringify(x) hale_stringify_(x)

#define hale_ch_(S) (L ## S)
#define hale_ch_w(S) ((ch16*)S)
#define hale_ch(S) hale_ch_w(hale_ch_(S))

// hale_ch("hello")
// hale_ch_w(hale_ch_("hello") hale_ch_(" world"))

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

void hale_panic_(const ch *message, const ch *description, const ch *function, const ch *file, s32 line);

#define hale_panic(message)  hale_panic_(hale_ch(message), hale_ch(message), hale_ch(__FUNCTION__), hale_ch(__FILE__), __LINE__)

#define hale_not_implemented hale_panic("not implemented")
#define hale_not_tested      hale_panic("not tested")
#define hale_not_reached     hale_panic("not reached")

// TODO: Log and crash.
#define hale_error(message)  hale_panic(message)
// TODO: Log only.

//
// Assert
//

// General assert.

//#include <assert.h>
//assert

#define _hale_assert_message(expression, message)\
    (void)( (!!(expression)) ||\
        (hale_panic_(message, hale_ch(hale_stringify(expression)), hale_ch(__FUNCTION__), hale_ch(__FILE__), __LINE__), 0) )


#define hale_assert_message(expression, message)\
    _hale_assert_message(expression, hale_ch(message))

#define hale_assert(expression)\
    _hale_assert_message(expression, hale_ch("Assertion message"))

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

#define hale_array_count(array) (sizeof(array) / sizeof((array)[0]))

#define hale_align_pow2(Value, Alignment) ((Value + ((Alignment) - 1)) & ~((Alignment) - 1))
#define hale_align_4(Value) ((Value + 3) & ~3)
#define hale_align_8(Value) ((Value + 7) & ~7)
#define hale_align_16(Value) ((Value + 15) & ~15)

// TODO: Convert to inlines?

#define hale_kilobytes(value) ((value)*1024LL)
#define hale_megabytes(value) (hale_kilobytes(value)*1024LL)
#define hale_gigabytes(value) (hale_megabytes(value)*1024LL)
#define hale_terabytes(value) (hale_gigabytes(value)*1024LL)


}

#endif // HALE_MACROS_H

