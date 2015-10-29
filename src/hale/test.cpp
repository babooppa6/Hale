#if HALE_INCLUDES
#include "hale_types.h"
#endif

#include "test_parser_c.cpp"

namespace hale {

s32 test_main(s32, ch **)
{
    test_parser_c();
    return 1;
}

}
