#include <QFile>
#include <QJsonObject>
#include <QDateTime>
#include <QTextStream>

#include "util.h"

bool Util::saveStringToFile(const QString &string, const QString &path)
{
    QFile file(path);
    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        stream << string;
        stream.flush();
        file.close();
        return true;
    }
    return false;
}

bool Util::saveStringToFile(const QByteArray &string, const QString &path)
{
    QFile file(path);
    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        stream << string;
        stream.flush();
        file.close();
        return true;
    }
    return false;
}

bool Util::loadStringFromFile(QString *string, const QString &path)
{
    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        stream.setAutoDetectUnicode(true);
        stream.setCodec("UTF-8");
        *string = stream.readAll();
        file.close();
        return true;
    }
    return false;
}

static const QString JSON_DATETIME_FORMAT("yyyy-MM-dd HH:mm:ss.zzz");

QJsonValue Util::fromDateTime(const QDateTime &date_time)
{
    return QJsonValue(date_time.toUTC().toString(JSON_DATETIME_FORMAT));
}

QDateTime Util::toDateTime(const QJsonValue &value)
{
    QDateTime dt(QDateTime::fromString(value.toString(), JSON_DATETIME_FORMAT));
    dt.setTimeSpec(Qt::UTC);
    return dt;
}

// 1. Must match all characters in needle.
// 2. The closer the first character is to the begin of the haystack, the better.
//    - This can also be done by closeness to a word.
//    - Essentially add distance from the start to first match (same as with 3)
// 3. The closer the characters are together, the better.

int StringDistance::distance(const QString &needle, const QString &haystack, MatchFunction f)
{
    return distance(needle.constData(), needle.length(), haystack.constData(), haystack.length(), f);
}

int StringDistance::distance(const QChar *needle, int needle_length, const QChar *haystack, int haystack_length, MatchFunction f)
{
    return distance(needle, needle_length, haystack, haystack_length, NULL, NULL, f);
}

//int StringDistance::distance(const QChar *needle, int needle_length, const QChar *haystack, int haystack_length, State *cached_state)
//{
//    Q_ASSERT(cached_state != NULL);

//    distanceInit(cached_state, haystack);
//    if (distanceAdd(needle, needle_length, haystack, haystack_length, cached_state)) {
//        return cached_state->d;
//    }
//    return -1;
//}

//void StringDistance::distanceInit(State *state, const QChar *haystack_begin, int haystack_length, bool path)
//{
//    state->haystack_begin = haystack_begin;
//    if (path) {
//        state->match = &StringDistance::matchBackward;
//        state->match_end = haystack_begin + haystack_length;
//    } else {
//        state->match = &StringDistance::matchForward;
//        state->match_end = 0;
//    }
//    state->d = 0;
//}

int StringDistance::distance(const QChar *needle, int needle_length, const QChar *haystack, int haystack_length, int *first_match, int *last_match, MatchFunction f)
{
    if (needle_length > haystack_length) {
        return -1;
    }

    int first = -1;
    int d = 0;
    int match_end = 0;
    int ni = 0;
    int hi = 0; // haystack - state->haystack_begin;
    haystack_length += hi;
    while (ni != needle_length && hi != haystack_length) {
        if (needle[ni] == haystack[hi]) {
            d += hi - match_end;
            // d += state->match(hi);
            if (first == -1) {
                first = hi;
            }
            if (f) {
                f(hi);
            }
            hi++;
            match_end = hi;
            ni++;
        } else {
            hi++;
        }
    }

    if (ni == needle_length) {
        if (first_match) {
            *first_match = first;
        }
        if (last_match) {
            *last_match = match_end;
        }
        return d;
    }
    return -1;
}

namespace {

    struct segment_t {
        // Iterator (starts at separator)
        const QChar *it;
        // Begin of the string.
        const QChar *sentry;
        // Match begin and end.
        const QChar *begin;
        const QChar *end;

        int index;
    };

    void segment_init(segment_t *seg, const QChar *string, int length) {
        seg->end = seg->it = seg->begin = string + length;
        seg->sentry = string;
        seg->index = 0;
    }

    bool segment_next(segment_t *seg, const QString &separator) {
        if (seg->it == seg->sentry) {
            return false;
        }

        seg->end = seg->it;
        seg->it = StringDistance::findPrevious(seg->end, seg->sentry, separator);
        if (seg->it == NULL) {
            seg->it = seg->sentry;
            seg->begin = seg->it;
        } else {
            seg->begin = seg->it + separator.length();
        }
        seg->index--;
        return true;
    }

    int segment_distance(segment_t *a, segment_t *b, int *f, int *l, StringDistance::MatchFunction func) {
        return StringDistance::distance(a->begin, a->end - a->begin, b->begin, b->end - b->begin, f, l, [&](int offset) {
            if (func) {
                func((b->begin - b->sentry) + offset);
            }
        });
    }
}

int StringDistance::pathDistance(const QString &needle, const QString &haystack, const QString &separator, MatchFunction f)
{
    return pathDistance(needle.constData(), needle.length(), haystack.constData(), haystack.length(), separator, f);
}

int StringDistance::pathDistance(const QChar *needle, int needle_length, const QChar *haystack, int haystack_length, const QString &separator, MatchFunction f)
{
    Q_ASSERT(separator.length() > 0);

    segment_t n, h;
    segment_init(&n, needle, needle_length);
    segment_init(&h, haystack, haystack_length);
    if (!segment_next(&n, separator)) {
        return -1;
    }

    // /--[match]--/--[match]--
    //                b      e last
    int b;
    int e;
    int last = haystack_length;
    int segment_begin;
    int d = 0;
    int total_d = 0;
    while (segment_next(&h, separator)) {
        d = segment_distance(&n, &h, &b, &e, f);
        if (d != -1) {
            segment_begin = h.begin - haystack;
            total_d += d + last - (segment_begin + e);
            last = segment_begin + b;
            if (segment_next(&n, separator) == false) {
                return total_d;
            }
        }
    }

    return -1;
}

const QChar *StringDistance::findNext(const QChar *it, const QChar *end, const QString &needle)
{
    end = end - needle.length();
    if (it > end) {
        return NULL;
    }
    while (it != end) {
        if (wcsncmp((const wchar_t *)it, (const wchar_t *)needle.constData(), needle.length()) == 0) {
            return it;
        }
        it++;
    }
    return end;
}

const QChar *StringDistance::findPrevious(const QChar *it, const QChar *begin, const QString &needle)
{
    it = it - needle.length();
    if (begin > it) {
        return NULL;
    }
    while (it >= begin) {
        if (wcsncmp((const wchar_t *)it, (const wchar_t *)needle.constData(), needle.length()) == 0) {
            return it;
        }
        it--;
    }
    return NULL;
}
