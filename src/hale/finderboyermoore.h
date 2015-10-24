#ifndef FINDERBOYERMOORE_H
#define FINDERBOYERMOORE_H

#include <functional>
#include <vector>
#include <QString>

class FinderBoyerMoore
{
public:
    typedef std::function<QChar (int pos)> CharFunc;
    FinderBoyerMoore(const QString &pattern, CharFunc char_func, bool ignore_case = false, bool whole_word = false);
    virtual ~FinderBoyerMoore();

    struct Result
    {
        int begin;
        int end;
    };

    /// Scans for a match in the string
    virtual bool find(int begin,
                      int end,
                      int first,
                      int last,
                      bool reverse,
                      Result *result);

private:
    /// Char function.
    CharFunc m_char_func;
    /// Pattern used
    QString m_pattern;
    /// End of pattern
    size_t m_pattern_end;
    /// True if case is ignored
    bool m_ignore_case;
    /// True if whole word match
    bool m_whole_word;

    struct Tables
    {
        Tables( unsigned int skip_init, unsigned int suffix_init )
        :	skip(skip_init, 0),
            suffix(suffix_init, 0)
        {}

        typedef std::vector<int> Table;
        Table skip;
        Table suffix;
    };

    /// Tables for forward search
    Tables * m_tables_forward;
    /// Tables for backward search
    Tables * m_tables_backward;

    Tables * makeTables(bool reverse);
    void makeSkipTable(Tables::Table & io_table, bool reverse);
    void makeSuffixTable(Tables::Table & io_table, bool reverse);

    /// Scans for a match in the string
    bool __find(Tables *tables,
                int begin,
                int end,
                int first,
                int last,
                bool reverse,
                Result *result);
};

#endif // __IT_FINDER_BOYER_MOORE_H__
