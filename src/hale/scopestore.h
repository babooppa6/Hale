#ifndef SCOPESTORE_H
#define SCOPESTORE_H

#include "scopepath.h"
#include <QJsonObject>

struct ScopeStore
{
    ScopeStore();

    struct Entry {
        ScopeSelector selector;
        QJsonObject options;
        ScopeScore score_;
    };
    typedef QList<Entry> Options;
    Options options;

    struct CacheEntry {
        ScopePath scope;
        QJsonObject options;
    };
    typedef QList<CacheEntry> Cache;
    Cache cache;

    void add(const ScopeSelector &scope, const QJsonObject &options);

    QJsonObject get(const ScopePath &scope);
    CacheEntry *find(const ScopePath &scope);
};

#endif // SCOPESTORE_H
