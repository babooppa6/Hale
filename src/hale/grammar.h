#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <QObject>
#include <QSharedPointer>
#include <QFileInfo>
#include "onigregexp.h"

class Document;
class OnigRegExp;

class GrammarRule
{
public:
    GrammarRule() :
        is_block(false),
        begin(NULL),
        end(NULL)
    {}

    GrammarRule(const GrammarRule &other) :
        is_block(other.is_block),
        name(other.name),
        begin(other.begin),
        end(other.end)
    {}

    GrammarRule &operator =(const GrammarRule &other);

    bool is_block;
    QString name;
    // TODO(cohen) Can we optimize these allocations?
    // - We cannot use by-value members, because std::list is copying entire structure.
    // - Possible option would be to use std::list with emplace.
    // - Other option would be first insert the rule to std::list and the use it's reference.
    // - Other option would be to use internal deque in GrammarReader and later build a static array.
    // - Other option would be to create a different allocator (possibly with all stored in Grammar?)
    QSharedPointer<OnigRegExp> begin;
    QSharedPointer<OnigRegExp> end;

    std::list<GrammarRule> children;

private:
};

class Grammar
{
public:
    Grammar() :
        rule(NULL)
    {}

    ~Grammar() {
        delete rule;
    }

    QString description() {
        if (!title.isEmpty()) {
            return title;
        }
        if (rule && !rule->name.isEmpty()) {
            return rule->name;
        }
        if (!path.isEmpty()) {
            return QFileInfo(path).fileName();
        }
        return QStringLiteral("Untitled");
    }

    // TODO: int indentationForBlock(Document *document, int block);

    QString path;
    QString title;
    QStringList extensions;

    // TODO: `name` has to be defined on Grammar, as the rule might be NULL.

    // Rule is NULL in case the grammar is not loaded, but we know about it.
    GrammarRule *rule;
};

class GrammarReader
{
public:
    GrammarReader();

    QSharedPointer<Grammar> loadFromJson(const QString &path);

private:
    bool readPatterns(GrammarRule *rule, const QJsonArray &patterns);
    bool readPattern(GrammarRule *rule, const QJsonObject &object);

    void error(const QString &message);

    QString m_path;
};

//
//
//

class GrammarManager : public QObject
{
    Q_OBJECT
public:
    explicit GrammarManager(QObject *parent = 0);

    void loadState();
    void saveState();

    // TODO: Change to only load index.
    void load(const QString &path);

    QSharedPointer<Grammar> loadGrammar(const QString &path);
    void reloadGrammar(const QString &path);
    void reloadGrammar(QSharedPointer<Grammar> grammar);

    QSharedPointer<Grammar> findGrammar(const QString &name);
    QSharedPointer<Grammar> findGrammarForDocument(const Document *document);

    void observeDocument(Document *document);

    static GrammarManager *instance;

private slots:
    void documentPathChanged(const QString &path);
    void documentGrammarNameChanged(const QString &name);
    void documentDestroyed(QObject *object);

private:
    typedef QList<QSharedPointer<Grammar> > Grammars;
    Grammars m_grammars;
    QSharedPointer<Grammar> m_grammar_default;
    QList<Document*> m_documents;
};

#endif // GRAMMAR_H
