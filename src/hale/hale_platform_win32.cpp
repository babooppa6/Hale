#include "hale.h"
#include "hale_platform.h"

#include <Windows.h>

namespace hale {

void
win32_print_error(const char *context)
{
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL);

    platform.debug_print_0_8((ch8*)context);
    platform.debug_print_0_8((ch8*)" ");
    platform.debug_print_0_16((ch16*)lpMsgBuf);

//	INO_LOG(_T("%s: %s\n"), caller_str, lpMsgBuf);
    // ::MessageBox(NULL, (LPCWSTR)lpMsgBuf, _T("Win32 API Error Log"), MB_OK);

    LocalFree(lpMsgBuf);
}

void *
win32_reserve_memory(memi size)
{
    void *memory = ::VirtualAlloc(0, size, MEM_RESERVE, 0);
    hale_assert(memory);
    return memory;
}

void
win32_commit_memory(void *memory, memi size)
{
    void *m = ::VirtualAlloc(memory, size, MEM_COMMIT, PAGE_READWRITE);
    hale_assert(m == memory);
    hale_assert(m);
}

HALE_PLATFORM_ALLOCATE_MEMORY(win32_allocate_memory)
{
    void *memory = ::VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    hale_assert(memory);
    return memory;
}

HALE_PLATFORM_DEALLOCATE_MEMORY(win32_deallocate_memory)
{
    hale_assert(memory);
    ::VirtualFree(memory, 0, MEM_RELEASE);
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

//
// Files
//

HALE_PLATFORM_OPEN_FILE(win32_open_file)
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

#include <string>

HALE_PLATFORM_DEBUG_PRINT_N_16(win32_debug_print_N_16)
{
    std::basic_string<ch16> s(string, length);
    OutputDebugStringW((LPCWSTR)s.c_str());
}

HALE_PLATFORM_DEBUG_PRINT_N_8(win32_debug_print_N_8)
{
    std::basic_string<ch8> s(string, length);
    OutputDebugStringA((LPCSTR)s.c_str());
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
    page_shift = log(page_size) / log(2);
    page_mask = si.dwPageSize - 1;

    allocate_memory = win32_allocate_memory;
    deallocate_memory = win32_deallocate_memory;
    copy_memory = win32_copy_memory;
    move_memory = win32_move_memory;
    read_time_counter = win32_read_time_counter;
    get_file_size_32 = win32_get_file_size_32;
    get_file_size_64 = win32_get_file_size_64;

    open_file = win32_open_file;
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
