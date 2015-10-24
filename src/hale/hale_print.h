#ifndef HALE_PRINT
#define HALE_PRINT

#include "hale.h"

namespace hale {

inline void
print(const ch8 *string) {
    platform.debug_print((ch8*)string);
}

inline void
print(const char *string) {
    platform.debug_print((ch8*)string);
}

inline void
print(const ch16 *string) {
    platform.debug_print((ch16*)string);
}

inline void
print(Vector<ch8> *string) {
    platform.debug_print(&vector_first(string), vector_count(string));
}

inline void
print(Vector<ch8> &string) {
    platform.debug_print(&vector_first(&string), vector_count(&string));
}

inline void
print(Vector<ch16> *string) {
    platform.debug_print(&vector_first(string), vector_count(string));
}

inline void
print(Vector<ch16> &string) {
    platform.debug_print(&vector_first(&string), vector_count(&string));
}

}

#endif // HALE_PRINT

