#if HALE_INCLUDES
#include "hale.h"
#endif

namespace hale {

LineEnding line_endings[] = {
    { LineEnding_Unknown,   0, (ch*)L""    , (ch*)L"Unknown"},
    { LineEnding_CRLF,      2, (ch*)L"\r\n", (ch*)L"CR+LF" },
    { LineEnding_LF,        1, (ch*)L"\n",   (ch*)L"LF"},
    { LineEnding_CR,        1, (ch*)L"\r",   (ch*)L"CR"},
};

LineEnding &string_find_next_line_ending(ch **begin, ch *end)
{
    for (; (*begin) != end; (*begin)++)
    {

        switch (**begin)
        {
        case '\r':
            if (((*begin)+1) != end && *((*begin)+1) == '\n') {
                (*begin) += 2;
                return line_endings[LineEnding_CRLF];
            } else {
                (*begin)++;
                return line_endings[LineEnding_CR];
            }
        case '\n':
            (*begin)++;
            return line_endings[LineEnding_LF];
        }
    }

    return line_endings[LineEnding_Unknown];
}

} // namespace hale
