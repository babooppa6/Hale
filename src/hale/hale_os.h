#ifndef HALE_PLATFORM_H
#define HALE_PLATFORM_H

namespace hale {

// Defined in platform.
struct File;

#define HALE_PLATFORM_OPEN_FILE_8(name) b32 name(File *file, const ch8 *path, u32 mode)
typedef HALE_PLATFORM_OPEN_FILE_8(PlatformOpenFile8);

#define HALE_PLATFORM_OPEN_FILE_16(name) b32 name(File *file, const ch16 *path, u32 mode)
typedef HALE_PLATFORM_OPEN_FILE_16(PlatformOpenFile16);

#define HALE_PLATFORM_CLOSE_FILE(name) void name(File *file)
typedef HALE_PLATFORM_CLOSE_FILE(PlatformCloseFile);

#define HALE_PLATFORM_READ_FILE(name) memi name(File *file, const void *memory, memi size)
typedef HALE_PLATFORM_READ_FILE(PlatformReadFile);

#define HALE_PLATFORM_WRITE_FILE(name) memi name(File *file, const void *memory, memi size)
typedef HALE_PLATFORM_WRITE_FILE(PlatformWriteFile);

#define HALE_PLATFORM_SEEK_FILE(name) memi name(File *file, memi pos)
typedef HALE_PLATFORM_SEEK_FILE(PlatformSeekFile);

#define HALE_PLATFORM_GET_FILE_SIZE_64(name) u64 name(File *file)
typedef HALE_PLATFORM_GET_FILE_SIZE_64(PlatformGetFileSize64);

#define HALE_PLATFORM_GET_FILE_SIZE_32(name) u32 name(File *file)
typedef HALE_PLATFORM_GET_FILE_SIZE_32(PlatformGetFileSize32);

//struct TextWriter;

//#define HALE_PLATFORM_READ_TEXT(name) b32 name(Stream *stream, TextWriter *writer)
//typedef HALE_PLATFORM_READ_TEXT(PlatformReadTextFile);

//
// Memory
//

#define HALE_PLATFORM_ALLOCATE_PAGED_MEMORY(name) void *name(memi size)
typedef HALE_PLATFORM_ALLOCATE_PAGED_MEMORY(PlatformAllocatePagedMemory);

#define HALE_PLATFORM_DEALLOCATE_PAGED_MEMORY(name) void name(void *memory)
typedef HALE_PLATFORM_DEALLOCATE_PAGED_MEMORY(PlatformDeallocatePagedMemory);

#define HALE_PLATFORM_REALLOCATE_PAGED_MEMORY(name) void *name(void *memory, memi new_size)
typedef HALE_PLATFORM_REALLOCATE_PAGED_MEMORY(PlatformReallocatePagedMemory);

#define HALE_PLATFORM_COPY_MEMORY(name) void name(void *destination, void *source, memi size)
typedef HALE_PLATFORM_COPY_MEMORY(PlatformCopyMemory);

#define HALE_PLATFORM_MOVE_MEMORY(name) void name(void *destination, void *source, memi size)
typedef HALE_PLATFORM_MOVE_MEMORY(PlatformMoveMemory);

//
// Print
//

#define HALE_PLATFORM_DEBUG_PRINT_N_16(name) void name(ch16 *string, memi length)
typedef HALE_PLATFORM_DEBUG_PRINT_N_16(PlatformDebugPrintN16);
#define HALE_PLATFORM_DEBUG_PRINT_0_16(name) void name(ch16 *string)
typedef HALE_PLATFORM_DEBUG_PRINT_0_16(PlatformDebugPrint016);

#define HALE_PLATFORM_DEBUG_PRINT_N_8(name) void name(ch8 *string, memi length)
typedef HALE_PLATFORM_DEBUG_PRINT_N_8(PlatformDebugPrintN8);
#define HALE_PLATFORM_DEBUG_PRINT_0_8(name) void name(ch8 *string)
typedef HALE_PLATFORM_DEBUG_PRINT_0_8(PlatformDebugPrint08);

// Returns time counter in seconds.
#define HALE_PLATFORM_READ_TIME_COUNTER(name) r64 name()
typedef HALE_PLATFORM_READ_TIME_COUNTER(PlatformReadTimeCounter);

struct Platform
{
    Platform();

    memi page_size;
    memi page_shift;
    memi page_mask;

    PlatformAllocatePagedMemory *allocate_paged_memory;
    PlatformDeallocatePagedMemory *deallocate_paged_memory;
    PlatformReallocatePagedMemory *reallocate_paged_memory;
    PlatformCopyMemory *copy_memory;
    PlatformMoveMemory *move_memory;

    PlatformReadTimeCounter *read_time_counter;

    PlatformOpenFile8 *open_file_8;
    PlatformOpenFile16 *open_file_16;
    PlatformCloseFile *close_file;
    PlatformReadFile *read_file;
    PlatformWriteFile *write_file;
    PlatformSeekFile *seek_file;
    PlatformGetFileSize64 *get_file_size_64;
    PlatformGetFileSize32 *get_file_size_32;

    PlatformDebugPrintN16 *debug_print_N_16;
    PlatformDebugPrint016 *debug_print_0_16;
    PlatformDebugPrintN8 *debug_print_N_8;
    PlatformDebugPrint08 *debug_print_0_8;

    inline void debug_print(ch16 *string) {
        debug_print_0_16(string);
    }

    inline void debug_print(ch8 *string) {
        debug_print_0_8(string);
    }

    inline void debug_print(ch16 *string, memi length) {
        debug_print_N_16(string, length);
    }

    inline void debug_print(ch8 *string, memi length) {
        debug_print_N_8(string, length);
    }

    inline b32 open_file(File *file, const ch16 *path, u32 mode)
    {
        return open_file_16(file, path, mode);
    }

    inline b32 open_file(File *file, const ch8 *path, u32 mode)
    {
        return open_file_8(file, path, mode);
    }
};

extern Platform platform;

#define hale_align_up_to_page_size(n) (((n)+platform.page_mask) & ~platform.page_mask)
#define hale_align_up_to_page_size_t(n, t) (hale_align_up_to_page_size((n)*(t))/(t))

} // namespace hale

#include "os_win32.h"

#endif // HALE_PLATFORM_H
