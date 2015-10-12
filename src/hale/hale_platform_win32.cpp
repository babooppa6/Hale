#include "hale_platform.h"

#include <Windows.h>

namespace hale {

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

//
//
//

//#include <QFile>
//#include <QTextStream>

//HALE_PLATFORM_READ_TEXT_FILE(win32_read_text_file)
//{
//    QFile file(QString((QChar*)path));
//    if (file.open(QFile::ReadOnly))
//    {
//        QTextStream stream(&file);
//        stream.setAutoDetectUnicode(true);
//        stream.setCodec("UTF-8");
//        writer->size = file.size();

//        int first = 1;
//        while (!stream.atEnd())
//        {
//            QString string = stream.read(4096);
//            if (first) {
//                writer->encoding = stream.codec()->mibEnum();
//                first = 0;
//            }
//            if (string.length() != 0) {
//                writer->write(writer, (ch*)string.data(), string.length());
//            }
//        }

//        file.close();
//        return true;
//    }
//    return false;
//}

#include <string>

HALE_PLATFORM_DEBUG_PRINT_N_16(win32_debug_print_N_16)
{
    std::wstring s(string, length);
    OutputDebugStringW(s.c_str());
}

HALE_PLATFORM_DEBUG_PRINT_N_8(win32_debug_print_N_8)
{
    std::string s(string, length);
    OutputDebugStringA(s.c_str());
}

HALE_PLATFORM_DEBUG_PRINT_0_16(win32_debug_print_0_16)
{
    OutputDebugStringW(string);
}

HALE_PLATFORM_DEBUG_PRINT_0_8(win32_debug_print_0_8)
{
    OutputDebugStringA(string);
}

Platform::Platform()
{
    QueryPerformanceFrequency((LARGE_INTEGER*)&win32_perf_counter_frequency);

    allocate_memory = win32_allocate_memory;
    deallocate_memory = win32_deallocate_memory;
    copy_memory = win32_copy_memory;
    move_memory = win32_move_memory;
    read_time_counter = win32_read_time_counter;

    open_file = win32_open_file;
    close_file = win32_close_file;
    read_file = win32_read_file;
    write_file = win32_write_file;

    // read_text_file = win32_read_text_file;
    debug_print_0_8 = win32_debug_print_0_8;
    debug_print_N_8 = win32_debug_print_N_8;
    debug_print_0_16 = win32_debug_print_0_16;
    debug_print_N_16 = win32_debug_print_N_16;

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    page_size = si.dwPageSize;
    page_shift = log(page_size) / log(2);
    page_mask = si.dwPageSize - 1;
}

} // namespace hale