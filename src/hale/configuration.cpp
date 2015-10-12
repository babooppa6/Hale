#include "precompiled.h"

#include "application.h"
#include "configurationobserver.h"
#include "scopepath.h"

#include "tlua.h"
#include "luaengine.h"
#include "configuration.h"

Configuration *Configuration::instance = NULL;

bool Configuration::Observer::operator ==(const Observer &other) {
    return scope.hasEqualNames(other.scope);
}

Configuration::Configuration(QObject *parent) :
    QObject(parent)
{
    Q_ASSERT(instance == NULL);
    instance = this;
}

namespace {
static void
table_to_json(tlua::table &table, QJsonObject *o_object)
{
    table.forEach([&] (const char *key, int type) {
        switch (type) {
        case LUA_TTABLE: {
            QJsonObject o;
            table_to_json(table.get<tlua::table>(key), &o);
            (*o_object)[key] = o;
        } break;
        case LUA_TSTRING: {
            (*o_object)[key] = table.get<QString>(key);
        } break;
        case LUA_TNUMBER: {
            (*o_object)[key] = table.get<double>(key);
        } break;
        case LUA_TBOOLEAN: {
            (*o_object)[key] = table.get<bool>(key);
        } break;
        }
    });
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
}

void Configuration::load()
{
    m_path = Application::instance()->resourcePath() + "/config.lua";
    options.clear();

    auto L = LuaEngine::instance->L;
    LuaEngine::instance->execute(m_path, 1);
    auto T(tlua::pop<tlua::table>(L));
    if (T.empty()) {
        qWarning() << __FUNCTION__ << "Configuration was empty.";
        return;
    }

    T.forEach([&] (const char *key, int type) {
        if (type == LUA_TTABLE) {
            Entry entry;
            entry.selector = key;
            table_to_json(T.get<tlua::table>(key), &entry.options);
            options << entry;
        }
    });

    for (Observer &observer : observers) {
        observer.options = get(&observer.scope);
        for (ConfigurationObserver *target : observer.targets) {
            target->configure(observer.options);
        }
    }
}


bool Configuration::update(const QString &path)
{
    if (path == m_path) {
        load();
        return true;
    }
    return false;
}

namespace {
    bool entry_less_than(const Configuration::Entry *a, const Configuration::Entry *b)
    {
        return a->score < b->score;
    }
}

QJsonObject Configuration::get(const ScopePath *scope)
{
    qDebug() << __FUNCTION__;
    qDebug() << __FUNCTION__ << *scope;
    QList<Entry*> matched;
    QJsonObject o;
    for (auto &entry : options) {
        if (scope->match(entry.selector, &entry.score)) {
            qDebug() << __FUNCTION__ << "-" << entry.selector << entry.score;
            matched.append(&entry);
        }
    }

    if (matched.empty()) {
        return o;
    }

    std::sort(matched.begin(), matched.end(), &entry_less_than);

    for (auto entry : matched) {
        if (entry->score.topMatched(scope)) {
            merge(&o, &entry->options);
        } else {
            inherit(&o, &entry->options);
        }
    }

    return o;
}

void Configuration::setTheme(QSharedPointer<Theme> theme)
{
    if (theme != m_theme) {
        m_theme = theme;
        emit themeChanged(m_theme);
    }
}

Configuration::Observer *Configuration::find(const ScopePath &scope)
{
    for (Observer &observer : observers) {
        if (observer.scope.hasEqualNames(scope)) {
            return &observer;
        }
    }
    return NULL;
}

Configuration::Observer *Configuration::find(const ScopePath &scope, ConfigurationObserver* target)
{
    for (Observer &observer : observers) {
        if (observer.scope.hasEqualNames(scope) && observer.targets.contains(target)) {
            return &observer;
        }
    }
    return NULL;
}

void Configuration::update(ConfigurationObserver *target)
{
    if (forget(target)) {
        observe(target);
    }
}

void Configuration::observe(ConfigurationObserver* target)
{
    ScopePath scope;
    scope.make(target);
    Observer *observer = find(scope);
    if (observer == NULL) {
        Observer n;
        n.scope = scope;
        n.options = get(&scope);
        observers << n;
        observer = &observers.last();
    } else {
        if (observer->targets.contains(target)) {
            qWarning() << __FUNCTION__ << "Already registered.";
            return;
        }
    }

    observer->targets << target;
    target->observeEvent();
    target->configure(observer->options);
}

//void Configuration::forget(const ScopePath &scope, ConfigurationObserver* target)
//{
//    Observer *observer = find(scope, target);
//    if (!observer) {
//        qWarning() << __FUNCTION__ << "Observer was never registered.";
//        return;
//    }

//    observer->targets.removeOne(target);
//    if (observer->targets.isEmpty()) {
//        observers.removeOne(*observer);
//    }
//}

bool Configuration::forget(ConfigurationObserver *target)
{
    bool ret = false;

    QList<ConfigurationObserver*>::iterator target_it;
    Observers::iterator it = observers.begin();
    while (it != observers.end()) {
        target_it = std::find(it->targets.begin(), it->targets.end(), target);
        if (target_it != it->targets.end()) {
            target->forgetEvent();
            it->targets.erase(target_it);
            ret = true;
            continue;
        }
        // COMMENTED: Keep the observers cached and configured.
//        if (it->targets.isEmpty()) {
//            it = observers.erase(it);
//            ret = true;
//            continue;
//        }
        it++;
    }
    return ret;
}

//void Configuration::setUiFont(QFont font)
//{
//    if (font != m_font_ui) {
//        m_font_ui = font;
//        m_font_ui_metrics = QFontMetrics(m_font_ui);
//        emit uiFontChanged(m_font_ui, m_font_ui_metrics);
//    }
//}

//void Configuration::setEditorFont(QFont font)
//{
//    if (font != m_font_editor) {
//        m_font_editor = font;
//        m_font_editor_metrics = QFontMetrics(m_font_editor);
//        emit editorFontChanged(m_font_editor, m_font_editor_metrics);
//    }
//}
