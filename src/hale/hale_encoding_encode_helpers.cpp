namespace hale {

#define UNI_SUR_HIGH_START  (UTF32)0xD800
#define UNI_SUR_HIGH_END    (UTF32)0xDBFF
#define UNI_SUR_LOW_START   (UTF32)0xDC00
#define UNI_SUR_LOW_END     (UTF32)0xDFFF

static const int halfShift  = 10; /* used for shifting by 10 bits */
static const u32 halfMask = 0x3FFUL;
static const u32 halfBase = 0x0010000UL;

/*
    ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
       +  (ch2 - UNI_SUR_LOW_START) + halfBase;
*/

inline u32
hale_utf32(u32* state, u32* codepoint, u32 le, u32 unit)
{
    // `unit` is `u32`, but really we expect `u16`.
    hale_assert_debug((unit & 0xFFFF0000) == 0);

//    if (unit >= UNI_SUR_HIGH_START && unit <= UNI_SUR_HIGH_END) {
//        *codepoint = (unit - UNI_SUR_HIGH_START) << halfShift;
//        *state = 1;
//    } else if (unit >= UNI_SUR_LOW_START && unit << UNI_SUR_LOW_END) {
//        *codepoint += (unit - UNI_SUR_LOW_START) + halfBase;
//        *state = 0;
//    } else {
//        *codepoint = unit;
//        *state = 0;
//    }

    return 0;
}

}
