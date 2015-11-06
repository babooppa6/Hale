#ifndef HALE_PLATFORM_WIN32_H
#define HALE_PLATFORM_WIN32_H

#ifndef NOMINMAX
#  define NOMINMAX
#endif
#define WINVER 0x0600
#include <windows.h>

#define HALE_PLATFORM_INVALID_SIZE_64 ((u64)-1)
#define HALE_PLATFORM_INVALID_SIZE_32 ((u32)-1)

#define HALE_STACK PagedMemory *stack

static_assert(sizeof(FLOAT) == sizeof(hale::r32), "Incompatible FLOAT size.");

namespace hale {

struct File
{
    enum {
        Read,
        Write
    };

    File() : handle(INVALID_HANDLE_VALUE) {}
    HANDLE handle;
};

void
win32_print_error(const char *context);

} // namespace hale

#endif // HALE_PLATFORM_WIN32_H

