#include <QDebug>
#include <QElapsedTimer>

#include "onigregexp.h"

#include "grammar.h"
#include "parser.h"

#define ONIG_REGION_GROUP_BEGIN(region, group)  (region->beg[group] >> 1)
#define ONIG_REGION_GROUP_END(region, group)    (region->end[group] >> 1)

// TODO(cohen) Handle empty match. (begin: "\*")
//  - Empty matches are allowed for block's begin or end regexes.
//  - Here is expected that child rules will advance.

Parser::Parser()
{
}

void Parser::setGrammar(QSharedPointer<Grammar> grammar)
{
    m_grammar = grammar;
}

void Parser::addToken(const QString &name, const QString &type, int begin, int end, Input *input)
{
    Q_UNUSED(type);

    m_statistics.numberOfTokens++;

    Token token;
    token.name = name;
    token.begin = begin;
    token.end = end;

    input->tokens->append(token);
}


namespace {
#if 0
    class Stack
    {
    public:
        struct Entry
        {
            GrammarRule *rule;
            Parser::Cache cache;
        };

        std::stack<Entry> stack;

        void push(GrammarRule *rule)
        {
            Entry entry;
            entry.rule = rule;
            stack.push(rule);
        }

        void pop()
        {
            stack.pop();
        }
    };
#endif
}

void Parser::parse(const QString &block, Stack *stack, Tokens *tokens)
{
    Q_ASSERT(!stack->empty());
    m_statistics = {};

    Input input(block);
    input.tokens = tokens;
    input.stack = stack;

    Cache cache;
    Result result;

    for (;;)
    {
        if (match(stack->back().rule, &input, &cache, &result))
        {
            if (result.result.begin(0) > input.offset) {
                // Create a token for the text.
                addToken(stack->back().rule->name, "text", (int)input.offset, result.result.begin(0), &input);
            }

            input.offset = result.result.end(0);

            if (result.rule->is_block) {
                if (result.end()) {
                    addToken(result.rule->name, "end", result.result.begin(0), result.result.end(0), &input);
                    stack->pop_back();
                } else {
                    addToken(result.rule->name, "begin", result.result.begin(0), result.result.end(0), &input);
                    stack->push_back({result.rule});
                }
                // TODO(cohen): Store the cache on the stack. Create a new one for the new stack top.
                // TODO(cohen): Possibly initialize the capacity of the new cache to something reasonable.
                //   - Try to run few complete grammars to see how many children they tend to have.
                //   - The caches will not be stored in the final stack.
                //   - Consider storing a pool of caches in the parser.
                cache.search_cache.clear();
            } else {
                addToken(result.rule->name, "match", result.result.begin(0), result.result.end(0), &input);
            }

        }
        else
        {
            break;
        }
    }

    if (input.offset < input.block.length() - 1) {
        addToken(stack->back().rule->name, "text", (int)input.offset, input.block.length() - 1, &input);
    }
}

bool Parser::match(GrammarRule *rule, Input *input, Cache *cache, Result *o_best_result)
{
    o_best_result->clear();

    int cache_index = 0;
    //
    // (1) Try to match end if available.
    //

    if (rule->end != NULL && match(rule, rule->end.data(), input, cache, cache_index, o_best_result)) {
        // We have matched the end rule as the best match (occuring at input->offset).
        return true;
    }

    //
    // (2) Try to match child rules.
    //

    for (auto &child_rule : rule->children)
    {
        cache_index++;
        if (match(&child_rule, child_rule.begin.data(), input, cache, cache_index, o_best_result)) {
            // We have found the best match in the children.
            return true;
        }
    }

    //
    // (3) If o_result did not match anything, return false.
    //

    return o_best_result->matched();
}

namespace {

    inline bool check_result(GrammarRule *rule, OnigRegExp *regex, OnigResult *result, Parser::Input *input, Parser::Result *o_best_result)
    {
        if (!o_best_result->matched() || result->begin(0) < o_best_result->result.begin(0))
        {
            o_best_result->set(result, rule, regex);

            if (result->begin(0) == input->offset) {
                // This is surely the best match.
                return true;
            }
        }
        return false;
    }

    inline void search_cache_insert(std::vector<OnigResult> *cache, int cache_index, bool matched, OnigResult *result)
    {
        if (cache->size() <= cache_index) {
            if (cache->capacity() <= cache_index) {
                cache->reserve(cache_index + 10);
            }
            cache->resize(cache_index + 1);
        }

        if (matched == false) {
            cache->at(cache_index).clear();
        } else {
            cache->at(cache_index) = *result;
        }
    }

}

bool Parser::match(GrammarRule *rule, OnigRegExp *regex, Input *input, Cache *cache, int cache_index, Result *o_best_result)
{
    bool use_cache = true;

    OnigResult *result;

    if (use_cache && cache->search_cache.size() > cache_index) {
        result = &cache->search_cache[cache_index];
        if (result->count() == 0) {
            // We know we didn't matched anything before with this here, so don't try it again.
            // qDebug() << "Cached (!)";
            m_statistics.numberOfCachesNoMatch++;
            return false;
        }
        if (result->begin(0) >= (int)input->offset) {
            // The match result in cache is still valid.
            if (check_result(rule, regex, result, input, o_best_result)) {
                // We have the best result in cache, no need to run the regex.
                // qDebug() << "Cached (~)";
                m_statistics.numberOfCachesMatchBest++;
                return true;
            }
            else
            {
                m_statistics.numberOfCachesMatch++;
            }
        }
        else
        {
            m_statistics.numberOfCachesMisses++;
        }
    }
    else
    {
        m_statistics.numberOfCachesMissesEmpty++;
    }

    result = &cache->result;
    bool matched = regex->search(input->block, input->offset, result);
    m_statistics.numberOfRegExpSearches++;
    search_cache_insert(&cache->search_cache, cache_index, matched, result);
    if (matched && result->count() > 0)
    {
        if (check_result(rule, regex, result, input, o_best_result)) {
            return true;
        }
    }

    // Result was not the best match.
    // The top match, if any, is returned in o_result.
    return false;
}
