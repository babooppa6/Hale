#include <QDebug>

#include "onigregexp.h"

OnigRegExp::OnigRegExp(const QString &source, QString *o_error) :
    m_regex(NULL)
#if !defined(QT_NO_DEBUG)
    , m_source(source)
#endif
{
    if (o_error) {
        o_error->clear();
    }
    OnigErrorInfo error;
    const UChar* sourceData = (const UChar*)source.data();
    int status = onig_new(&m_regex,
                          sourceData,
                          sourceData + (source.length() << 1),
                          ONIG_OPTION_CAPTURE_GROUP,
                          ONIG_ENCODING_UTF16_LE,
                          ONIG_SYNTAX_DEFAULT, &error);

    if (status != ONIG_NORMAL) {
        UChar error_string[ONIG_MAX_ERROR_MESSAGE_LEN];
        onig_error_code_to_str(error_string, status, &error);
        if (o_error) {
            *o_error = (char*)error_string;
        } else {
            qWarning() << "Error compiling OnigRegEx" << source << ":" << (char*)error_string;
        }
        if (m_regex) {
            onig_free(m_regex);
            m_regex = NULL;
        }
    }
}

OnigRegExp::~OnigRegExp()
{
    if (m_regex) {
        onig_free(m_regex);
    }
}

// TODO(cohen): ONIG_OPTION_NOTBOL when it != it_begin
// TODO(cohen): ONIG_OPTION_NOTEOL when it != it_end

#if 0
static OnigOptionType anchor_options (bool isFirstLine, bool isGPos, char const* first, char const* last)
{
    OnigOptionType res = ONIG_OPTION_NONE;
    if(!isFirstLine)
        res |= ONIG_OPTION_NOTBOS;
    if(!isGPos)
        res |= ONIG_OPTION_NOTGPOS;
    if(first != last && last[-1] == '\n')
        res |= ONIG_OPTION_NOTEOS;
    return res;
}
#endif

bool OnigRegExp::search(const QString &string, size_t offset, OnigResult *result)
{
    if (!m_regex)
    {
        qWarning() << "Cannot search in NULL regexp.";
        return false;
    }

    offset = offset << 1;
    int end = string.size() << 1;
    auto string_begin = (const UChar*)string.data();
    auto status = onig_search(m_regex,
                             string_begin,
                             string_begin + end,
                             string_begin + offset,
                             string_begin + end,
                             result->m_region,
                             ONIG_OPTION_NONE
                             );

    if (status != ONIG_MISMATCH) {
        return true;
    }

    return false;
}

//
//
//

OnigResult::OnigResult()
{
    m_region = onig_region_new();
}

OnigResult::OnigResult(const OnigResult &other)
{
    m_region = onig_region_new();
    onig_region_copy(m_region, other.m_region);
}

OnigResult &OnigResult::operator =(const OnigResult &other)
{
    onig_region_copy(m_region, other.m_region);
    return *this;
}

OnigResult::~OnigResult()
{
    onig_region_free(m_region ,1);
}

void OnigResult::clear()
{
    onig_region_clear(m_region);
}

int OnigResult::count()
{
    return m_region->num_regs;
}

int OnigResult::begin(int group)
{
    Q_ASSERT(group < count());
    Q_ASSERT(m_region->beg[group] >= -1);
    if (m_region->beg[group] > 0) {
        return m_region->beg[group] >> 1;
    }
    // NOTE(cohen) Returns 0 or -1
    return m_region->beg[group];
}

int OnigResult::end(int group)
{
    Q_ASSERT(group < count());
    Q_ASSERT(m_region->end[group] >= -1);
    if (m_region->end[group] > 0) {
        return m_region->end[group] >> 1;
    }
    // NOTE(cohen) Returns 0 or -1
    return m_region->end[group];
}

int OnigResult::length(int group)
{
    Q_ASSERT(group < count());
    if (begin(group) == -1) {
        return 0;
    }
    return (m_region->end[group] >> 1) - (m_region->beg[group] >> 1);
}

//
//
//

#if 0

OnigScanner::OnigScanner()
{
}

void OnigScanner::add(OnigRegExp *regex)
{
    Q_CHECK_PTR(regex);
    regex->m_index = m_regex.size();
    m_regex.append(QSharedPointer<OnigRegExp>(regex));
}

bool OnigScanner::search(const QString &string, size_t offset, OnigScannerCache *cache, OnigResult* result)
{
    QSharedPointer<OnigResult> best_result;
    int best_begin = 0;

    for (auto regex : m_regex)
    {
        QSharedPointer<OnigResult> result = regex->search(string, offset);
        if (result && result->count() > 0) {
            int begin = result->begin(0);
            if (best_result == NULL || begin < best_begin) {
                best_begin = begin;
                best_result = result;
            }

            if (begin == offset) {
                // We have found the best result at given offset.
                break;
            }
        }
    }

    return best_result;
}

#endif
