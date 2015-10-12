#include "scopestore.h"

ScopeStore::ScopeStore()
{

}

static void
merge(QJsonObject *base, QJsonObject *other)
{
    QJsonObject::iterator it = other->begin();
    while (it != other->end()) {
        if (base->contains(it.key())) {
            QJsonValue base_value = base->value(it.key());
            if (base_value.type() == QJsonValue::Object) {
                QJsonObject base_object(base_value.toObject());
                merge(&base_object, &it.value().toObject());
                base->insert(it.key(), base_object);
                it++;
                continue;
            }
        }
        base->insert(it.key(), it.value());
        it++;
    }
}

static void
inherit(QJsonObject *base, QJsonObject *other)
{
    QJsonObject::iterator it = other->begin();
    while (it != other->end()) {
        if (it.key() == "background" ||
            it.key() == "foreground" ||
            it.key() == "font")
        {
            base->insert(it.key(), it.value());
        }
        it++;
    }
}

static bool
entry_less_than(const ScopeStore::Entry *a, const ScopeStore::Entry *b)
{
    return a->score_ < b->score_;
}


QJsonObject ScopeStore::get(const ScopePath &scope)
{
//    qDebug() << __FUNCTION__;
//    qDebug() << __FUNCTION__ << *scope;
    QList<Entry*> matched;
    QJsonObject o;
    for (auto &entry : options) {
        if (scope.match(entry.selector, &entry.score_)) {
            qDebug() << __FUNCTION__ << "-" << entry.selector << entry.score_;
            matched.append(&entry);
        }
    }

    if (matched.empty()) {
        return o;
    }

    std::sort(matched.begin(), matched.end(), &entry_less_than);

    for (auto entry : matched) {
        if (entry->score_.topMatched(&scope)) {
            merge(&o, &entry->options);
        } else {
            inherit(&o, &entry->options);
        }
    }

    return o;
}

ScopeStore::CacheEntry *ScopeStore::find(const ScopePath &scope)
{
    for (CacheEntry &e : cache) {
        if (e.scope.hasEqualNames(scope)) {
            return &e;
        }
    }
    return NULL;
}
