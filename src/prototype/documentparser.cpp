#include "util.h"

#include "document.h"
#include "documentparser.h"

// I cannot just stop at first stack match. The stacks might be not even used by the grammar (no begin/end).

// TODO(cohen): Do I need to parse the entire document all the time?
// - Can we actually use a hint that will say what part of the document is most important?
//   - Typically the edited line and the visible segments (there might be multitude of those)

// TODO(cohen): Can the m_block_first actually change without DocumentParser knowing it?
// - It can only change by inserting or removing lines.
// - Any insert or removal will update the m_block_first anyway.

// TODO(cohen): If the grammar has no children, we must ensure that it's root scope is always returned.

// Enable to print debug information and to perf count the parsing.
// #define DOCUMENT_PARSER_DEBUG

DocumentParser::DocumentParser(Document *document) :
    m_d(document),
    m_block_first(INT_MAX),
    m_active(false)
{
    m_partial_parse_timer.setInterval(0);
    connect(&m_partial_parse_timer, SIGNAL(timeout()), this, SLOT(parsePartial()));
}

void DocumentParser::setTheme(QSharedPointer<Theme> theme)
{
    m_theme = theme;
    invalidateBlockFormats();
}

void DocumentParser::setGrammar(QSharedPointer<Grammar> grammar)
{
    m_grammar = grammar;
    invalidate();
}

void DocumentParser::resume()
{
    if (m_active == false) {
        m_active = true;
        startPartialParse("resume");
    }
}

void DocumentParser::suspend()
{
    if (m_active != false) {
        m_active = false;
        stopPartialParse();
    }
}

void DocumentParser::invalidate()
{
#if defined(DOCUMENT_PARSER_DEBUG)
    qDebug() << __FUNCTION__;
#endif
    invalidate(0, m_d->blockCount() - 1);
    startPartialParse("total invalidate");
}

void DocumentParser::invalidate(int block_first, int block_last)
{
#if defined(DOCUMENT_PARSER_DEBUG)
    qDebug() << __FUNCTION__ << block_first << block_last;
#endif
    if (m_block_first > block_first) {
        m_block_first = block_first;
    }

    for (; block_first != block_last; block_first++)
    {
        m_d->blockData(block_first)->flags |= Document::Block::F_TokensInvalidated;
    }

    startPartialParse("range invalidate");
}

void DocumentParser::invalidateBlockFormats()
{
    for (int i = 0; i < m_d->blockCount(); i++) {
        emit m_d->beforeFormatChanged();
        Document::Block *block = m_d->blockData(i);
        block->formats.clear();
        block->flags |= Document::Block::F_FormatsInvalidated;
        emit m_d->formatChanged(i);
    }
}

QList<QTextLayout::FormatRange> *DocumentParser::blockFormats(int block_index)
{
    Document::Block *block = m_d->blockData(block_index);
    if (!(block->flags & Document::Block::F_FormatsInvalidated)) {
        return &block->formats;
    }

    block->formats.clear();
    m_theme->apply(block->tokens, &block->formats);
    block->flags &= ~Document::Block::F_FormatsInvalidated;

    return &block->formats;
}

void DocumentParser::parseImmediate(int block_first, int block_last)
{
    // NOTE(cohen): We cannot start immediate parsing if the lines above the
    // requested range are waiting to be parsed.
    // This can be detected simply by checking if the block_first is lower than m_block_first.

    // TODO(cohen): We can possibly let the parser parse all the lines above the block_first
    // and parse whole region resulting in longer wait, but consistent behavior:
    // -> Guarantee that the document is always parsed on changed line.
    // Do we even need that?

    // Invalidate will change the m_block_first, so we better store it.
    int block_first_old = m_block_first;
    invalidate(block_first, block_last);
    if (block_first <= block_first_old) {
        // If we can parse, then we'll only pass block_last.
        if (canParse()) {
            parse(block_last);
        }
    }
}

void DocumentParser::parse(int block_last)
{
    Q_ASSERT(canParse());
    if (!canParse()) {
        return;
    }

#if defined(DOCUMENT_PARSER_DEBUG)
    PERFORMANCE_TIMER(t, __FUNCTION__);
#endif

    bool is_immediate = block_last != -1;

    if (!is_immediate) {
        block_last = m_d->blockCount() - 1;
    }

    QString block_text;

    Document::Block *block_previous;
    Document::Block *block = m_d->blockData(m_block_first);

#if defined(DOCUMENT_PARSER_DEBUG)
    int debug_range_block_first = m_block_first;
#endif

    if (m_block_first == 0) {
        block->stack.clear();
        block->stack.push_back({m_grammar->rule});
    } else {
        block->stack = m_d->blockData(m_block_first - 1)->stack;
        Q_ASSERT(!block->stack.empty());
    }
#if defined(DOCUMENT_PARSER_DEBUG)
    qDebug() << __FUNCTION__ << "{";
#endif

    QElapsedTimer timer;
    timer.start();

    for (;;)
    {
        // TODO(cohen): We must parse the line if:
        // - F_TokensInvalidated is set -or-
        // - The end stack of the previous line is different with beginning stack of this line.
        //   - This can be possibly check after we parse the previous line.
        //   - We can simply check if it's ending stack has changed and then force the parse of the next line.
        //   - Be careful with the first line parsed.


        block_text = m_d->blockText(m_block_first);

        if (!is_immediate)
        {
            emit m_d->beforeFormatChanged();
        }

        // qDebug() << "Parsing block" << m_block_first;
        block->tokens.clear();
        m_parser.parse(block_text, &block->stack, &block->tokens);
        block->flags |= Document::Block::F_FormatsInvalidated;
        block->flags &= ~Document::Block::F_TokensInvalidated;

        if (!is_immediate)
        {
            // It we're not parsing in immediate mode, we won't send any formatChanged signals.
            emit m_d->formatChanged(m_block_first);
        }

        if (m_block_first == block_last) {
            break;
        }
        m_block_first++;

        if (timer.elapsed() > MAX_MS_PER_PARTIAL_PARSE) {
            break;
        }

        block_previous = block;

        block = m_d->blockData(m_block_first);
        block->stack = block_previous->stack;
    }
#if defined(DOCUMENT_PARSER_DEBUG)
    qDebug() << "}" << __FUNCTION__ << debug_range_block_first << m_block_first << "in" << timer.elapsed() << "ms";
#endif

    if (m_block_first == m_d->blockCount() - 1) {
        // All is parsed.
        m_block_first = INT_MAX;
        stopPartialParse();
    } else {
        startPartialParse("deferred parsing");
    }
}

void DocumentParser::startPartialParse(const char *reason)
{
#if defined(DOCUMENT_PARSER_DEBUG)
    qDebug() << __FUNCTION__ << reason << (m_active ? "active" : "suspended");
#else
    Q_UNUSED(reason);
#endif
    if (canParse()) {
#if defined(DOCUMENT_PARSER_DEBUG)
        qDebug() << "- started";
#endif
        m_partial_parse_timer.start();
    }
}

void DocumentParser::stopPartialParse()
{
#if defined(DOCUMENT_PARSER_DEBUG)
    qDebug() << __FUNCTION__;
#endif
    m_partial_parse_timer.stop();
}

void DocumentParser::parsePartial()
{
#if defined(DOCUMENT_PARSER_DEBUG)
    qDebug() << __FUNCTION__;
#endif
    parse(-1);
}

bool DocumentParser::canParse()
{
    return m_active && m_grammar && m_grammar->rule && !m_grammar->rule->children.empty() && m_block_first != INT_MAX;
}
