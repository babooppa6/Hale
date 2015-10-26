#include "hale_config.h"
#include "hale_macros.h"
// #include <stdint.h>
// #include <float.h>
#include "hale_types.h"
#include "hale_atoi.h"
// #include <cmath>
#include "hale_math.h"
#include "hale_math_interpolation.h"
// #include <cstring>
#include "hale_util.h"
// #include <windows.h>
//X #include "os_win32.h"
#include "hale_os.h"
// #include <vector>
#include "hale_vector.h"
#include "hale_memory.h"
#include "hale_string.h"
#include "hale_print.h"
#include "hale.h"
#include "hale_test.h"
#include "hale_gap_buffer.h"
#include "hale_fixed_gap_buffer.h"
#include "hale_document_types.h"
#include "hale_encoding_mib.h"
#include "hale_encoding.h"
#include "hale_document_view.h"
#include "hale_document_parser.h"
#include "hale_document.h"
#include "hale_stream.h"
// #include "os_win32_gdi.h"
// #include <dwrite.h>
// ignored #include "os_win32_dx.h"
// ignored #include "os_win32_ui.h"
#include "hale_os_ui.h"
#include "hale_ui.h"
//X #include "_hale_encoding_utf8.cpp"
//X #include "_hale_encoding_utf16.cpp"
//X #include "_hale_encoding_hale.cpp"
//X #include "os_win32.cpp"
// #include <windowsx.h>
// #include <dwmapi.h>
//X #include "os_win32_gdi.cpp"
// #include <d2d1.h>
//X #include "os_win32_dx.cpp"
//X #include "os_win32_ui.cpp"
#include "hale_parser_c.h"
// #include <intrin.h>
#include "hale_perf.h"

#include "hale.cpp"
#include "hale_atoi.cpp"
#include "hale_document.cpp"
#include "hale_document_arena.cpp"
#include "hale_document_parse.cpp"
#include "hale_document_view.cpp"
#include "hale_encoding.cpp"
#include "hale_fixed_gap_buffer.cpp"
#include "hale_gap_buffer.cpp"
#include "hale_os.cpp"
#include "hale_os_ui.cpp"
#include "hale_parser_c.cpp"
#include "hale_print.cpp"
#include "hale_string.cpp"
#include "hale_test.cpp"
#include "hale_ui.cpp"
#include "hale_util.cpp"

// not found: "hale_os_win32_gdi.h" (referenced in 'os_win32_gdi.cpp', line 3)
// not found: "hale_os_win32_ui.h" (referenced in 'os_win32_gdi.cpp', line 2)
// not found: "hale_test_document.cpp" (referenced in 'os_win32_ui.cpp', line 470)
// not found: "hale_test_encoding.cpp" (referenced in 'os_win32_ui.cpp', line 471)

// _hale_encoding_hale.cpp referenced in:
//    hale_encoding.cpp
// _hale_encoding_utf16.cpp referenced in:
//    hale_encoding.cpp
// _hale_encoding_utf8.cpp referenced in:
//    hale_encoding.cpp
// hale.h referenced in:
//    hale_atoi.cpp
//    hale_document.cpp
//    hale_document.h
//    hale_document_arena.cpp
//    hale_document_parser.h
//    hale_document_view.h
//    hale_encoding.cpp
//    hale_encoding.h
//    hale_encoding_mib.h
//    hale_fixed_gap_buffer.h
//    hale_gap_buffer.cpp
//    hale_os_ui.cpp
//    hale_os_ui.h
//    hale_parser_c.cpp
//    hale_perf.h
//    hale_stream.h
//    hale_string.cpp
//    hale_test.h
//    hale_ui.h
//    os_win32.cpp
//    os_win32_gdi.cpp
//    os_win32_gdi.h
//    os_win32_ui.cpp
//    os_win32_ui.h
// hale_atoi.h referenced in:
//    hale.h
//    hale_atoi.cpp
//    hale_string.h
// hale_config.h referenced in:
//    hale.h
// hale_document.h referenced in:
//    hale_document.cpp
//    hale_document_arena.cpp
//    hale_document_parse.cpp
//    hale_document_view.cpp
//    hale_ui.h
//    os_win32_dx.cpp
// hale_document_parser.h referenced in:
//    hale_document.h
//    hale_parser_c.h
// hale_document_types.h referenced in:
//    hale_document.h
//    hale_document_view.h
// hale_document_view.h referenced in:
//    hale_document.h
// hale_encoding.h referenced in:
//    hale_document_arena.cpp
//    hale_document_view.h
//    hale_encoding.cpp
// hale_encoding_mib.h referenced in:
//    _hale_encoding_hale.cpp
//    _hale_encoding_utf16.cpp
//    _hale_encoding_utf8.cpp
//    hale_encoding.h
// hale_fixed_gap_buffer.h referenced in:
//    hale_document.h
//    hale_fixed_gap_buffer.cpp
// hale_gap_buffer.h referenced in:
//    hale_document.cpp
//    hale_document.h
//    hale_gap_buffer.cpp
// hale_macros.h referenced in:
//    hale.h
// hale_math.h referenced in:
//    hale.h
// hale_math_interpolation.h referenced in:
//    hale.h
// hale_memory.h referenced in:
//    hale.h
// hale_os.h referenced in:
//    hale.h
//    hale_os.cpp
//    os_win32.cpp
// hale_os_ui.h referenced in:
//    hale_os_ui.cpp
//    hale_ui.h
// hale_parser_c.h referenced in:
//    hale_parser_c.cpp
//    hale_ui.cpp
// hale_perf.h referenced in:
//    hale_ui.cpp
// hale_print.h referenced in:
//    hale.h
//    hale_fixed_gap_buffer.cpp
//    hale_print.cpp
// hale_stream.h referenced in:
//    hale_document_arena.cpp
// hale_string.h referenced in:
//    hale.h
// hale_test.h referenced in:
//    hale_fixed_gap_buffer.h
//    hale_gap_buffer.cpp
//    hale_gap_buffer.h
//    hale_test.cpp
// hale_types.h referenced in:
//    hale.h
//    hale_gap_buffer.h
// hale_ui.h referenced in:
//    hale_document_view.cpp
//    hale_ui.cpp
//    os_win32_dx.cpp
//    os_win32_ui.cpp
// hale_util.h referenced in:
//    hale.h
// hale_vector.h referenced in:
//    hale.h
//    hale_print.cpp
// os_win32.cpp referenced in:
//    hale_os.cpp
// os_win32.h referenced in:
//    hale_os.h
// os_win32_dx.cpp referenced in:
//    os_win32_ui.cpp
// os_win32_dx.h referenced in:
//    os_win32_ui.h
// os_win32_gdi.cpp referenced in:
//    os_win32_ui.cpp
// os_win32_gdi.h referenced in:
//    os_win32_ui.h
// os_win32_ui.cpp referenced in:
//    hale_os_ui.cpp
// os_win32_ui.h referenced in:
//    hale_os_ui.h
