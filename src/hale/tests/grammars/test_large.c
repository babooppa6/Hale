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

Parser::Parser(QSharedPointer<Grammar> grammar) :
    m_grammar(grammar)
{
}

void Parser::parseBlock(const QString &block)
{
    parse(block);

#if 0

    size_t offset = 0;

    auto rules = &m_grammar->rules;

    bool matched;
    while (offset < block.length())
    {
        matched = false;
        auto it = rules->begin();
        while (it != rules->end())
        {
            // qDebug() << "Attempting match " << it->begin->source() << " " << it->name << " at " << offset;

            auto result = it->begin->match(block, offset);
            if (result)
            {
                offset += result->length(0);
                addToken(&*it, result.data());
                matched = true;
            }
            it++;
        }

        if (!matched) {
            offset++;
        }
    }
#endif
}

void Parser::addToken(const QString &name, const QString &type, int begin, int end, Input *input)
{
    QStringRef substring(&input->block, begin, end - begin);
    qDebug() << "Token" << name << type << substring << (begin+1) << "-" << (end+1);
    emit tokenAdded(name, begin, end-begin);
}

//
//
//
namespace {

    struct Context
    {

    };

} // namespace

void Parser::parse(const QString &block)
{
    QElapsedTimer timer;
    timer.start();

    Input input(block);
    Cache cache;
    Result result;

    std::stack<GrammarRule*> stack;
    stack.push(&m_grammar->rule);

    for (;;)
    {
        if (match(stack.top(), &input, &result, &cache))
        {
            if (result.result.begin(0) > input.offset) {
                // Create a token for the text.
                addToken(stack.top()->name, "text", input.offset, result.result.begin(0), &input);
            }

            input.offset = result.result.end(0);

            if (result.rule->is_block) {
                if (result.end()) {
                    addToken(result.rule->name, "end", result.result.begin(0), result.result.end(0), &input);
                    stack.pop();
                } else {
                    addToken(result.rule->name, "begin", result.result.begin(0), result.result.end(0), &input);
                    stack.push(result.rule);
                }
            } else {
                addToken(result.rule->name, "match", result.result.begin(0), result.result.end(0), &input);
            }

        }
        else
        {
            break;
        }
    }

     qDebug() << "Parsing took" << timer.elapsed() << "milliseconds.";
}

bool Parser::match(GrammarRule *rule, Input *input, Result *o_result, Cache *cache)
{
    o_result->rule = NULL;
    o_result->regex = NULL;

    //
    // (1) Try to match end if available.
    //

    if (rule->end != NULL && match(rule, rule->end.data(), input, o_result, cache)) {
        // We have matched the end rule as the best match (occuring at input->offset).
        return true;
    }

    //
    // (2) Try to match child rules.
    //

    for (auto &child_rule : rule->children)
    {
        if (match(&child_rule, child_rule.begin.data(), input, o_result, cache)) {
            // We have found the best match in the children.
            return true;
        }
    }

    //
    // (3) If o_result did not match anything, return false.
    //

    return o_result->matched();
}

bool Parser::match(GrammarRule *rule, OnigRegExp *regex, Input *input, Result *o_result, Cache *cache)
{
    OnigResult *result = &cache->result;
    bool matched = regex->search(input->block, input->offset, result);
    if (matched && cache->result.count() > 0)
    {
        if (!o_result->matched() || result->begin(0) < o_result->result.begin(0))
        {
            o_result->result = *result;
            o_result->rule = rule;
            o_result->regex = regex;

            if (result->begin(0) == input->offset) {
                // This is surely the best match.
                return true;
            }
        }
    }

    // Result was not the best match.
    // The top match, if any, is returned in o_result.
    return false;
}
