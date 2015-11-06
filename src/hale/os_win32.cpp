#if HALE_INCLUDES
#include "hale.h"
#include "hale_os.h"
#include "hale_print.h"
#include "hale_stack_memory.h"
#endif

namespace hale {

void
win32_print_error(const char *context)
{
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError();

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR) &lpMsgBuf,
        0, NULL);

//    platform.debug_print_0_8((ch8*)context);
//    platform.debug_print_0_8((ch8*)" ");
//    platform.debug_print_0_16((ch16*)lpMsgBuf);

//	INO_LOG(_T("%s: %s\n"), caller_str, lpMsgBuf);
    ::MessageBoxA(NULL, (LPCSTR)lpMsgBuf, "Win32 API Error Log", MB_OK);

    LocalFree(lpMsgBuf);
}

//
//
//

HALE_PLATFORM_SHOW_ERROR(win32_show_error)
{
    Memory<ch> string;
    string = {};

    StringSink<Memory<ch>>(&string, StringSink<Memory<ch>>::Flag_AddZero)
            << message << "\n"
            << description << "\n"
			<< "Function:" << function << "\n"
            << "File: " << file << "\n"
            << "Line: " << line;

    ::MessageBoxW(NULL, (LPCWSTR)string.ptr, (LPCWSTR)message, MB_OK);
}

void hale_panic_(const ch *message, const ch *description, const ch *function, const ch *file, s32 line)
{
    win32_show_error(message, description, function, file, line);
#if HALE_DEBUG
    _CrtDbgBreak();
#else
    *(int *)0 = 0;
#endif
}

//
//
//

//void *
//win32_reserve_memory(memi size)
//{
//    void *memory = ::VirtualAlloc(0, size, MEM_RESERVE, 0);
//    hale_assert(memory);
//    return memory;
//}

//void
//win32_commit_memory(void *memory, memi size)
//{
//    void *m = ::VirtualAlloc(memory, size, MEM_COMMIT, PAGE_READWRITE);
//    hale_assert(m == memory);
//    hale_assert(m);
//}

HALE_PLATFORM_ALLOCATE_PAGED_MEMORY(win32_allocate_paged_memory)
{
    void *memory = ::VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    hale_assert(memory);
    return memory;
}

HALE_PLATFORM_DEALLOCATE_PAGED_MEMORY(win32_deallocate_paged_memory)
{
    hale_assert(memory);
    ::VirtualFree(memory, 0, MEM_RELEASE);
}

HALE_PLATFORM_REALLOCATE_PAGED_MEMORY(win32_reallocate_paged_memory)
{
    hale_not_tested;
    hale_assert(memory);
    void *new_memory = win32_allocate_paged_memory(new_size);
    CopyMemory(memory, new_memory, new_size);
    win32_deallocate_paged_memory(memory);
    return new_memory;
}

HALE_PLATFORM_COPY_MEMORY(win32_copy_memory)
{
    hale_assert_debug(destination);
    hale_assert_debug(source);
    CopyMemory(destination, source, size);
}

HALE_PLATFORM_MOVE_MEMORY(win32_move_memory)
{
    hale_assert_debug(destination);
    hale_assert_debug(source);
    MoveMemory(destination, source, size);
}

s64 win32_perf_counter_frequency;

HALE_PLATFORM_READ_TIME_COUNTER(win32_read_time_counter)
{
    s64 counter;
    QueryPerformanceCounter((LARGE_INTEGER*)&counter);
    return (r64)((r64)counter / (r64)win32_perf_counter_frequency); // *(1e3);
}

void memory_init(PagedMemory *memory, memi reserve, void *base)
{
    hale_assert(reserve != 0);
    * memory = {};
    memory->_reserved = hale_align_up_to_page_size(reserve);
    memory->base = VirtualAlloc(base,
                                memory->_reserved,
                                MEM_RESERVE,
                                PAGE_EXECUTE_READWRITE);
    if (memory->base == 0) {
        win32_print_error("VirtualAlloc");
        hale_error("VirtualAlloc");
    }
}

void *memory_push(PagedMemory *memory, memi size)
{
    hale_assert_message(memory->count + size <= memory->_reserved, "StackAllocator overflow.");

    void *ret = ((u8*)memory->base) + memory->count;
    memory->count += size;
    if (memory->count > memory->capacity) {
        memory->capacity = hale_align_up_to_page_size(memory->count);
        LPVOID r = VirtualAlloc(memory->base, memory->capacity, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        hale_assert(r == memory->base);
		if (r == 0) {
			win32_print_error("VirtualAlloc(MEM_COMMIT)");
			hale_error("VirtualAlloc(MEM_COMMIT)");
		}
    }
    return ret;
}

void *memory_pop(PagedMemory *memory, memi size)
{
    memory->capacity -= size;
    return ((u8*)memory->base) + memory->capacity;
}

//
// Files
//

HALE_PLATFORM_OPEN_FILE_8(win32_open_file_8)
{
    DWORD access = mode == File::Read ? GENERIC_READ : GENERIC_WRITE;
    // TODO: Sharin flag.
    DWORD sharing = mode == File::Read ? FILE_SHARE_READ : 0;
    // TODO: Append flag.
    DWORD creation = mode == File::Read ? OPEN_EXISTING : CREATE_ALWAYS; // Cannot combine! (CREATE_NEW|TRUNCATE_EXISTING);

    // CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    file->handle = CreateFileA((LPCSTR)path, access, sharing, 0, creation, 0, 0);
    return file->handle != INVALID_HANDLE_VALUE;
}

HALE_PLATFORM_OPEN_FILE_16(win32_open_file_16)
{
    DWORD access = mode == File::Read ? GENERIC_READ : GENERIC_WRITE;
    // TODO: Sharin flag.
    DWORD sharing = mode == File::Read ? FILE_SHARE_READ : 0;
    // TODO: Append flag.
    DWORD creation = mode == File::Read ? OPEN_EXISTING : CREATE_ALWAYS; // Cannot combine! (CREATE_NEW|TRUNCATE_EXISTING);

    // CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    file->handle = CreateFileW((LPCWSTR)path, access, sharing, 0, creation, 0, 0);
    return file->handle != INVALID_HANDLE_VALUE;
}


HALE_PLATFORM_CLOSE_FILE(win32_close_file)
{
    hale_assert_debug(file->handle);
    if (file->handle) {
        CloseHandle(file->handle);
        file->handle = INVALID_HANDLE_VALUE;
    }
}

HALE_PLATFORM_READ_FILE(win32_read_file)
{
    hale_assert(size <= 0xFFFFffff);
    hale_assert(file->handle != INVALID_HANDLE_VALUE);

    DWORD read = 0;
    BOOL ret = ReadFile(file->handle,
                        (LPVOID)memory,
                        (DWORD)size,
                        &read,
                        NULL
                        );

    hale_assert_debug(ret == TRUE);
    if (ret == FALSE) {
        return 0;
    }
    return read;
}

HALE_PLATFORM_WRITE_FILE(win32_write_file)
{
    hale_assert(size <= 0xFFFFffff);
    hale_assert(file->handle != INVALID_HANDLE_VALUE);

    DWORD wrote = 0;
    BOOL ret = WriteFile(file->handle,
                         memory,
                         (DWORD)size,
                         &wrote,
                         NULL);

    hale_assert_debug(ret == TRUE);
    if (ret == FALSE) {
        return 0;
    }
    return wrote;
}

HALE_PLATFORM_SEEK_FILE(win32_seek_file)
{
    LARGE_INTEGER li;

    li.QuadPart = pos;
    li.LowPart = SetFilePointer(file->handle,
                                li.LowPart,
                                &li.HighPart,
                                FILE_BEGIN);

    if (li.LowPart == INVALID_SET_FILE_POINTER &&
        GetLastError() != NO_ERROR)
    {
        li.QuadPart = -1;
    }

    return li.QuadPart;
}

HALE_PLATFORM_GET_FILE_SIZE_64(win32_get_file_size_64)
{
    LARGE_INTEGER li;
    if (GetFileSizeEx(file->handle, &li)) {
        return li.QuadPart;
    }

    return HALE_PLATFORM_INVALID_SIZE_64;
}

HALE_PLATFORM_GET_FILE_SIZE_32(win32_get_file_size_32)
{
    LARGE_INTEGER li;
    if (GetFileSizeEx(file->handle, &li)) {
        return safe_truncate_u64(li.QuadPart);
    }

    return HALE_PLATFORM_INVALID_SIZE_32;
}


//
//
//

HALE_PLATFORM_DEBUG_PRINT_N_16(win32_debug_print_N_16)
{
    // TODO: We can do all this with just a static buffer and streaming.

    Memory<ch16> s = {};
    s.push(length + 1, 0);
    memory_copy(s.ptr, string, length);
    s.ptr[length] = 0;

    OutputDebugStringW((LPCWSTR)s.ptr);

    s.deallocate();
}

HALE_PLATFORM_DEBUG_PRINT_N_8(win32_debug_print_N_8)
{
    // TODO: We can do all this with just a static buffer and streaming.

    hale_not_tested;

    Memory<ch8> s;
    s.allocate(length + 1);
    memory_copy(s.ptr, string, length);
    s.ptr[length] = 0;

    OutputDebugStringA((LPCSTR)s.ptr);
}

HALE_PLATFORM_DEBUG_PRINT_0_16(win32_debug_print_0_16)
{
    OutputDebugStringW((LPCWSTR)string);
}

HALE_PLATFORM_DEBUG_PRINT_0_8(win32_debug_print_0_8)
{
    OutputDebugStringA((LPCSTR)string);
}

// TODO: Move to main?

Platform::Platform()
{
    QueryPerformanceFrequency((LARGE_INTEGER*)&win32_perf_counter_frequency);

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    page_size = si.dwPageSize;
    page_shift = (memi)(log(page_size) / log(2));
    page_mask = si.dwPageSize - 1;

    allocate_paged_memory = win32_allocate_paged_memory;
    deallocate_paged_memory = win32_deallocate_paged_memory;
    reallocate_paged_memory = win32_reallocate_paged_memory;
    copy_memory = win32_copy_memory;
    move_memory = win32_move_memory;
    read_time_counter = win32_read_time_counter;
    get_file_size_32 = win32_get_file_size_32;
    get_file_size_64 = win32_get_file_size_64;

    open_file_8 = win32_open_file_8;
    open_file_16 = win32_open_file_16;
    close_file = win32_close_file;
    read_file = win32_read_file;
    write_file = win32_write_file;
    seek_file = win32_seek_file;

    // read_text_file = win32_read_text_file;
    debug_print_0_8 = win32_debug_print_0_8;
    debug_print_N_8 = win32_debug_print_N_8;
    debug_print_0_16 = win32_debug_print_0_16;
    debug_print_N_16 = win32_debug_print_N_16;
}

} // namespace hale
