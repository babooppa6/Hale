#ifndef SCOPEPATH_H
#define SCOPEPATH_H

#include <QObject>
#include <QDebug>
#include <QString>
#include <QStringList>

class ConfigurationObserver;

class ScopePath;
class ScopeSelector;

struct ScopeScore
{
    static const int MAX_ELEMENT_LENGTH = 256;
    static const int MAX_ELEMENT_COUNT  = 64;
    static const int MAX_ELEMENT        = MAX_ELEMENT_COUNT - 1;

    typedef qint8 Value[MAX_ELEMENT_COUNT];

    ScopeScore() {
    }

    ScopeScore(const ScopeScore &other) {
        *this = other;
    }

    ScopeScore &operator =(const ScopeScore &other) {
        set(other);
        return *this;
    }

    inline bool operator >(const ScopeScore &other) const {
        return compare(other) > 0;
    }

    inline bool operator <(const ScopeScore &other) const {
        return compare(other) < 0;
    }

    inline bool operator >=(const ScopeScore &other) const {
        return compare(other) >= 0;
    }

    inline bool operator <=(const ScopeScore &other) const {
        return compare(other) <= 0;
    }

    bool topMatched(const ScopePath *scope);

    inline int compare(const ScopeScore &other) const {
        // 10000 > 10
        if (top > other.top) {
            return 1;
        }
        if (top < other.top) {
            return -1;
        }

        // 3001 > 3001
        // 5000 > 4000
        return memcmp((&value[MAX_ELEMENT] - top),
                      (&other.value[MAX_ELEMENT] - top),
                      top + 1);
    }

    inline void set(const ScopeScore &other) {
        memcpy(&value, &other.value, sizeof(Value));
        top = other.top;
    }

    inline ScopeScore &plus(int index, int part) {
        Q_ASSERT(index < MAX_ELEMENT_COUNT);
        Q_ASSERT(part < MAX_ELEMENT_LENGTH);
        value[MAX_ELEMENT - index] = part;
        top = qMax(index, top);
        return *this;
    }

    inline void reset() {
        memset(&value, 0, sizeof(Value));
        top = 0;
    }

    int top;
    Value value;
};

QDebug operator<<(QDebug debug, const ScopeScore &s);

class ScopePath
{
public:
    ScopePath();
    ScopePath(const ScopePath &other);
    ScopePath &operator =(const ScopePath &other);

    bool hasEqualNames(const ScopePath &other) const;

    /// Clears the scope path.
    void clear();

    /// Creates a scope path by the objects.
    void make(ConfigurationObserver *focus_object);

    /// Pushes an entry to a scope path.
    void push(QString string, ConfigurationObserver *object = NULL);

    /// Matches the path against a selector.
    bool match(ScopeSelector &selector, ScopeScore *score) const;
    static int matchName(const QString &path_name, const QString &selector_name);

    ConfigurationObserver *top(const QString &name) const;


    void dump();
    QString toString();

    struct Element
    {
        Element(const QString &string, ConfigurationObserver *object) :
            string(string),
            object(object)
        {}
        QString string;
        ConfigurationObserver *object;
    };
    typedef QList<Element> Path;
    Path m_path;
};

QDebug operator<<(QDebug debug, const ScopePath &s);

class ScopeSelector
{
public:
    ScopeSelector();
    ScopeSelector(const QString &string);

    ScopeSelector &operator =(const QString &string);

    QStringList string_list;
};

QDebug operator<<(QDebug debug, const ScopeSelector &s);

#endif // SCOPEPATH_H
