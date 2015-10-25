#include <cstdlib>
#include "finderboyermoore.h"

namespace {
    ushort getSkipIndex(QChar c) {
        return c.unicode() & 0xff;
    }
}

FinderBoyerMoore::FinderBoyerMoore(const QString &pattern, CharFunc char_func, bool ignore_case, bool whole_word) :
    m_tables_forward(NULL),
    m_tables_backward(NULL),
    m_pattern_end(pattern.length() - 1),
    m_ignore_case(ignore_case),
    m_whole_word(whole_word),
    m_char_func(char_func)
{
    if (m_ignore_case) {
        m_pattern = pattern.toUpper();
    } else {
        m_pattern = pattern;
    }
}

FinderBoyerMoore::~FinderBoyerMoore()
{
    delete m_tables_forward;
    delete m_tables_backward;
}

bool FinderBoyerMoore::find(int begin, int end, int first, int last, bool reverse, FinderBoyerMoore::Result *result)
{
    if (first != last && m_pattern.length() > 0)
    {
        // typedef std::reverse_iterator< iterator > reverse_iterator;
        if (reverse)
        {
            if (m_tables_backward == NULL) {
                m_tables_backward = makeTables( true );
            }
            return __find(m_tables_backward,
                        end,
                        begin,
                        last,
                        first,
                        reverse,
                        result);
        }
        else
        {
            if (m_tables_forward == NULL) {
                m_tables_forward = makeTables(false);
            }
            return __find(m_tables_forward,
                                    begin,
                                    end,
                                    first,
                                    last,
                                    reverse,
                                    result);
        }
    }

    return false;
}

FinderBoyerMoore::Tables *FinderBoyerMoore::makeTables(bool reverse)
{
    Tables * tables = new Tables( 256, m_pattern.length() + 1 );
    makeSkipTable( tables->skip, reverse );
    makeSuffixTable( tables->suffix, reverse );
    return tables;
}

void FinderBoyerMoore::makeSkipTable(FinderBoyerMoore::Tables::Table &io_table, bool reverse)
{
    io_table.resize( 256, 0 );

    // In case of an empty m_pattern, leave the table empty
    if( m_pattern.length() == 0 )
        return;

    int pos = 0;
    do {
        QChar c = m_pattern[(int)(reverse ? m_pattern_end - pos : pos)];
        io_table[getSkipIndex(c)] = pos;
    }
    while( ++pos < m_pattern.length() );
}

void FinderBoyerMoore::makeSuffixTable(FinderBoyerMoore::Tables::Table &io_table, bool reverse)
{
    int m = m_pattern.length();
    int j = m + 1;

    io_table.resize( j );
    Tables::Table tmp( j, 0 );
    tmp[m] = j;

    for(int i = m; i > 0; --i)
    {
        while
                (
                 j <= m &&
                 (
                     m_pattern[(int)(reverse ? m_pattern_end - i + 1 : i - 1)]
                     !=
                     m_pattern[(int)(reverse ? m_pattern_end - j + 1 : j - 1)]
                     )
                 )
        {
            if (io_table[j] == 0) {
                io_table[j] = j - i;
            }

            j = tmp[j];
        }

        tmp[ i - 1 ] = --j;
    }

    int k = tmp[0];

    for( j = 0; j <= m; j++ )
    {
        // The code above builds a 1-indexed suffix array,
        // but we shift it to be 0-indexed, ignoring the
        // original 0-th element
        if( j > 0 ) {
            io_table[j - 1] = ( io_table[j] == 0 ) ? k : io_table[j];
        }

        if( j == k ) {
            k = tmp[k];
        }
    }
}

namespace {
    int __increment(int &a, int i) {
        return a + i;
    }
    int __decrement(int &a, int i) {
        return a - i;
    }
}

typedef int (*IncrementFunc)(int &, int);

bool FinderBoyerMoore::__find(FinderBoyerMoore::Tables *tables, int begin, int end, int first, int last, bool reverse, FinderBoyerMoore::Result *result)
{
    IncrementFunc increment = reverse ? &__decrement : &__increment;

    bool ret = false;
    if (first != last && m_pattern.length() > 0)
    {
        /// Position in pattern
        int pos = 0;
        /// Anchor iterator in haystack
        int anchor = first;

        /// Heuristics #1
        int bad_char = 0;
        /// Heuristics #2
        int good_suffix = 0;

        /// Haystack character
        QChar c = 0;
SEARCH:
        while( (anchor + m_pattern_end) < last ) {
            for (pos = m_pattern_end; pos >= 0; --pos) {
                c = m_char_func(increment(anchor, pos));
                if (m_ignore_case) {
                    c = c.toUpper();
                    // c = StringTools::uppercaseCharacter( c );
                }

                if( ( reverse ? c != m_pattern[(int)m_pattern_end - pos] : c != m_pattern[(int)pos] ) )
                {
                    // Character mismatch, determine how many characters to skip
                    // Heuristic #1
                    bad_char = pos - tables->skip[getSkipIndex(c)];
                    // Heuristic #2
                    good_suffix = tables->suffix[pos];
                    // Skip the greater of the two distances provided by the
                    // heuristics
                    anchor = increment(anchor, (bad_char > good_suffix) ? bad_char : good_suffix);
                    // Go back to the while loop
                    goto SEARCH;
                }
            }

            //                if( m_whole_word )
            //                {
            //                    if( anchor == begin || afUnicodeCType::isWordBoundary( afUnicodeCType::getType((U32)*(anchor-1)), afUnicodeCType::getType((U32)*(anchor))) )
            //                    {
            //                        anchor_tmp = anchor + m_pattern_end;
            //                        if( anchor_tmp == end || (anchor_tmp+1) == end || afUnicodeCType::isWordBoundary( afUnicodeCType::getType((U32)*(anchor_tmp)), afUnicodeCType::getType((U32)*(anchor_tmp+1)) ) )
            //                        {
            //                            goto SEARCH_DONE;
            //                        }
            //                    }

            //                    anchor++;
            //                    goto SEARCH;
            //                }

SEARCH_DONE:
            // MATCH: return the position of its first character
            if (reverse) {
                int b = std::abs(end - anchor);
                result->begin = b - m_pattern.length();
                result->end = b;
                ret = true;
            } else {
                int b = std::abs(begin - anchor);
                result->begin = b;
                result->end = b + m_pattern.length();
                ret = true;
            }

            break;
        }
    }

    return ret;
}
