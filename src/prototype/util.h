#ifndef UTIL_H
#define UTIL_H

#include <QElapsedTimer>
#include <QDebug>
#include <QString>
#include <functional>

class Util
{
public:
    static bool saveStringToFile(const QString &string, const QString &path);
    static bool saveStringToFile(const QByteArray &string, const QString &path);
    static bool loadStringFromFile(QString *string, const QString &path);

    static QJsonValue fromDateTime(const QDateTime &date_time);
    static QDateTime toDateTime(const QJsonValue &value);
};

class PerformanceTimer
{
public:
    QElapsedTimer timer;
    char *name;
#if defined(Q_OS_WIN)
    PerformanceTimer(const char *name): name(_strdup(name)) { timer.start(); }
#else
    PerformanceTimer(const char *name): name(strdup(name)) { timer.start(); }
#endif
    ~PerformanceTimer() {
        qDebug() << name << "time" << (double)timer.nsecsElapsed() / 1000000.0 << "ms";
        delete [] name;
    }
};

#define PERFORMANCE_TIMER_RELEASE(t, n) PerformanceTimer t(n)
#define PERFORMANCE_TIMER(t, n) PerformanceTimer t(n)

class StringDistance
{
public:
    struct State;

    static const int NO_MATCH = -1;

    typedef std::function<void (int match)> MatchFunction;

    static int distance(const QString &needle, const QString &haystack, MatchFunction f = MatchFunction());
    static int distance(const QChar *needle, int needle_length, const QChar *haystack, int haystack_length, MatchFunction f = MatchFunction());
    static int distance(const QChar *needle, int needle_length, const QChar *haystack, int haystack_length, int *first_match, int *last_match, MatchFunction f = MatchFunction());

    // static void distanceInit(State *state, const QChar *haystack_begin, int haystack_length, bool path = false);
    // static bool distanceAdd(const QChar *needle, int needle_length, const QChar *haystack, int haystack_length, State *state);

    static int pathDistance(const QString &needle, const QString &haystack, const QString &separator, MatchFunction f = MatchFunction());
    static int pathDistance(const QChar *needle, int needle_length, const QChar *haystack, int haystack_length, const QString &separator, MatchFunction f = MatchFunction());

    static const QChar *findNext(const QChar *it, const QChar *end, const QString &needle);
    static const QChar *findPrevious(const QChar *it, const QChar *begin, const QString &needle);
};

#endif // UTIL_H
