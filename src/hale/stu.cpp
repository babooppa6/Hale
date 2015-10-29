// #define HALE_TEST 1

#ifdef HALE_INCLUDES
#undef HALE_INCLUDES
#endif

#define depends(f)

#include "hale_config.h"
#include "hale_macros.h"
#include "hale_types.h"
#include "hale_itoa.h"
#include "hale_ftoa.h"
#include "hale_math.h"
#include "hale_math_interpolation.h"
#include "hale_util.h"
#include "hale_os.h"
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
#include "hale_os_ui.h"
#include "hale_ui.h"
#include "hale_parser_c.h"
#include "hale_perf.h"

#ifdef HALE_TEST
#include "test.h"
#endif

#include "hale.cpp"
#include "hale_itoa.cpp"
#include "hale_document.cpp"
#include "hale_document_arena.cpp"
#include "hale_document_parser.cpp"
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


#ifdef HALE_TEST
#include "test.cpp"
#endif
