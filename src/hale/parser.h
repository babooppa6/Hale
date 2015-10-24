#ifndef PARSER_H
#define PARSER_H

#include <QObject>
#include <QSharedPointer>
#include <QVector>

#include <stack>

#include "grammar.h"

class Grammar;
class GrammarRule;

class OnigResult;

class Parser : public QObject
{
    Q_OBJECT
public:
    struct Statistics
    {
        int numberOfRegExpSearches;
        int numberOfCachesMatch;
        int numberOfCachesMatchBest;
        int numberOfCachesNoMatch;
        int numberOfCachesMisses;
        int numberOfCachesMissesEmpty;
        int numberOfTokens;
    };

    Parser();

    Statistics &statistics() {
        return m_statistics;
    }

    void setGrammar(QSharedPointer<Grammar> grammar);

    struct Token
    {
        QString name;
        int begin;
        int end;
    };
    typedef QVector<Token> Tokens;

    struct StackEntry
    {
        GrammarRule *rule;
        bool operator ==(const StackEntry &other) const {
            return rule == other.rule;
        }
    };
    typedef QVector<StackEntry> Stack;


    void parse(const QString &block, Stack *stack, Tokens *tokens);

    // TODO: These are private, need to move them.

    struct Input
    {
        Input(const QString &block) : block(block), offset(0) {}
        const QString &block;
        int offset;
        Tokens *tokens;
        Stack *stack;
    };

    struct Result
    {
        OnigResult result;
        OnigRegExp *regex;
        GrammarRule *rule;

        void set(OnigResult *_result, GrammarRule *_rule, OnigRegExp *_regex) {
            result = *_result;
            regex = _regex;
            rule = _rule;
        }

        void clear() {
            // Not clearing result due to optimization.
            result.clear();
            regex = NULL;
            rule = NULL;
        }

        bool matched() {
            return rule != NULL;
        }

        bool end() {
            return regex == rule->end;
        }
    };


    struct Cache
    {
        OnigResult result;
        std::vector<OnigResult> search_cache;
    };

private:
    Statistics m_statistics;
    QSharedPointer<Grammar> m_grammar;

    void addToken(const QString &name, const QString &type, int begin, int end, Input *input);

    bool match(GrammarRule *rule, Input *input, Cache *cache, Result *o_best_result);
    bool match(GrammarRule *rule, OnigRegExp *regex, Input *input, Cache *cache, int cache_index, Result *o_best_result);
};

#endif // PARSER_H
