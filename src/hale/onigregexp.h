#ifndef ONIGREGEXP_H
#define ONIGREGEXP_H

#include <QString>
#include <QList>
#include <QSharedPointer>

#include <oniguruma.h>

class OnigResult;

class OnigRegExp
{
public:
    explicit OnigRegExp(const QString &source, QString *o_error = NULL);
    ~OnigRegExp();

    bool isValid() {
        return m_regex != NULL;
    }

    bool search(const QString &source, size_t offset, OnigResult *result);

    QString source()
    {
#if !defined(QT_NO_DEBUG)
        return m_source;
#else
        return QString();
#endif
    }

private:
    OnigRegExp(const OnigRegExp&);
    OnigRegExp &operator=(const OnigRegExp&);

    friend class OnigScanner;

#if !defined(QT_NO_DEBUG)
    QString m_source;
#endif
    regex_t* m_regex;
};

//
//
//

class OnigResult
{
public:
    OnigResult();
    ~OnigResult();

    OnigResult(const OnigResult &other);
    OnigResult &operator=(const OnigResult&);

    void clear();

    int count();
    int begin(int group);
    int end(int group);
    int length(int group);

private:
    friend class OnigRegExp;

    OnigRegion *m_region;
};

//
//
//

#if 0

class OnigScannerCache
{
public:
};

//
//
//

class OnigScanner
{
public:
    OnigScanner();

    /// Scanner takes ownership.
    void add(OnigRegExp *regex);

    bool search(const QString& string, size_t offset, OnigScannerCache *cache, OnigResult* result);

private:
    QList<QSharedPointer<OnigRegExp>> m_regex;
};

#endif

#endif // ONIGREGEXP_H
