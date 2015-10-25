#include <QDebug>
#include <QRegularExpression>

// #include "application.h"

#include "configurationobserver.h"
#include "scopepath.h"

bool ScopeScore::topMatched(const ScopePath *scope)
{
   if (scope->m_path.count() == 0) {
       return false;
   }
   return value[MAX_ELEMENT_COUNT - scope->m_path.count()] != 0;
}
//
//
//

ScopePath::ScopePath()
{
}

ScopePath::ScopePath(const ScopePath &other)
{
    *this = other;
}

ScopePath &ScopePath::operator =(const ScopePath &other)
{
    m_path = other.m_path;
    return *this;
}

bool ScopePath::hasEqualNames(const ScopePath &other) const
{
    if (other.m_path.count() != m_path.count()) {
        return false;
    }

    for (int i = 0; i < m_path.count(); i++) {
        if (m_path[i].string != other.m_path[i].string) {
            return false;
        }
    }

    return true;
}

ConfigurationObserver *ScopePath::top(const QString &name) const
{
    if (m_path.isEmpty()) {
        return NULL;
    }

    Path::const_iterator it = m_path.end();
    do
    {
        it--;
        if (matchName(it->string, name)) {
            return it->object;
        }
    } while(it != m_path.begin());
    return NULL;
}

void ScopePath::clear()
{
    m_path.clear();
}

void ScopePath::make(ConfigurationObserver *s)
{
    ConfigurationObserver *proxy;
    m_path.clear();
    while (s != NULL) {
        // TODO: Oh fuck, how I hate this piece.
        // This is used to put the status line in the scope of the
        // document (with status line belonging to the panel in the code).
        // This can be probably remedied by making the path from parent to
        // child and having the parent have the first say (as it has the
        // most information).
        proxy = s->proxy();
        if (proxy) {
            if (!proxy->state.isEmpty()) {
                push(proxy->state, proxy->state_object);
            }
            proxy->scope(this);
            s = proxy->scopeParent();
        } else {
            if (!s->state.isEmpty()) {
                push(s->state, s->state_object);
            }
            s->scope(this);
            s = s->scopeParent();
        }
    }
}

void ScopePath::push(QString string, ConfigurationObserver *object)
{
    Q_ASSERT(string.length() < ScopeScore::MAX_ELEMENT_LENGTH);
    Q_ASSERT(m_path.size() <= ScopeScore::MAX_ELEMENT_COUNT);
    m_path.prepend(Element(string, object));
}

// Variant A
// Negative score = matched some, but not the top.
// Positive score = matched some, and also the top.
// - That wouldn't differentiate between close-to-top matches.

// Maximum score is defined by how long path is.
// So we can add extra score on top of that.
// Count how good the selector matched the path (up to path.length points)
//

bool ScopePath::match(ScopeSelector &selector, ScopeScore *score) const
{
    score->reset();

    if (selector.string_list.length() > m_path.length()) {
        return false;
    }

    if (selector.string_list.isEmpty() || m_path.isEmpty()) {
        return true;
    }

    if (selector.string_list.count() == 1 && selector.string_list.first().isEmpty()) {
        return true;
    }

    QStringList &sel = selector.string_list;

    int p;
    int pi = m_path.size() - 1;
    int si = sel.size() - 1;
    while (pi != -1 && si != -1) {
        p = matchName(m_path[pi].string, sel[si]);
        if (p > 0) {
            score->plus(pi, p);
            si--;
            pi--;
        } else {
            pi--;
        }
    }

    // We must match entire selector.
    if (si == -1) {
        return true;
    }
    return false;
}

int ScopePath::matchName(const QString &path_name, const QString &selector_name)
{
    if (path_name.startsWith(selector_name)) {
        return qMin(path_name.length(), selector_name.length());
    }
    return 0;
}

void ScopePath::dump()
{
    qDebug() << __FUNCTION__ << toString();
}

QString ScopePath::toString()
{
    QString ret;
    for (auto &element : m_path)
    {
        ret.append(" - ");
        ret.append(element.string);
        ret.append(" ");
        if (element.object) {
            if (element.object->qObject) {
                ret.append(element.object->qObject->metaObject()->className());
                ret.append(" ");
            }
            ret.append(QString::number((qulonglong)element.object, 16));
        } else {
            ret.append("NULL");
        }
        ret.append("\n");
    }

    return ret;
}

QDebug operator<<(QDebug debug, const ScopePath &s)
{
    QDebugStateSaver saver(debug);

    debug.nospace() << '[';
    for (int i = 0; i < s.m_path.count(); i++) {
        const ScopePath::Element &e = s.m_path[i];
        debug << e.string;
    }
    debug.nospace() << ']';

    return debug;
}

//
//
//

QRegularExpression split("\\s+");

ScopeSelector::ScopeSelector()
{}

ScopeSelector::ScopeSelector(const QString &selector)
{
    *this = selector;
}

ScopeSelector &ScopeSelector::operator =(const QString &selector)
{
    string_list = selector.split(split);
    return *this;
}

QDebug operator<<(QDebug debug, const ScopeSelector &s)
{
    QDebugStateSaver saver(debug);

    debug.nospace() << "ScopeSelector(";
    debug << s.string_list;
    debug.nospace() << ')';

    return debug;
}


//
//
//

QDebug operator<<(QDebug debug, const ScopeScore &s)
{
    QDebugStateSaver saver(debug);

    debug.nospace() << '[';
    for (int i = ScopeScore::MAX_ELEMENT - s.top; i < ScopeScore::MAX_ELEMENT_COUNT; i++) {
        debug << s.value[i];
    }
    debug.nospace() << ']';

    return debug;
}
