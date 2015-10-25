#ifndef DOCUMENTPARSER_H
#define DOCUMENTPARSER_H

#include <QObject>
#include <QTextLayout>
#include <QSharedPointer>
#include <QList>
#include <QTimer>

#include "parser.h"
#include "grammar.h"
#include "theme.h"

class Document;

class DocumentParser : public QObject
{
    Q_OBJECT

    static const int MAX_MS_PER_PARTIAL_PARSE = 5;

public:
    DocumentParser(Document *document);

    void resume();
    void suspend();

    void setGrammar(QSharedPointer<Grammar> grammar);
    QSharedPointer<Grammar> grammar()
    { return m_grammar; }

    void setTheme(QSharedPointer<Theme> theme);
    QSharedPointer<Theme> theme()
    { return m_theme; }

    void invalidate();
    void invalidate(int block_first, int block_last);

    /// Invalidates the given range.
    /// Tries to parse immediately. Any formatChange signal for lines in given range is skipped.
    /// This is designed to be called from Document::insertText/removeText because they
    /// will notify the textChange themselves.
    void parseImmediate(int block_first, int block_last);

    QList<QTextLayout::FormatRange> *blockFormats(int block_index);

private:
    Document *m_d;
    bool m_active;
    Parser m_parser;
    QSharedPointer<Grammar> m_grammar;
    QSharedPointer<Theme> m_theme;
    int m_block_first;

    QTimer m_partial_parse_timer;

    void startPartialParse(const char *reason);
    void stopPartialParse();

    // @block_last determines where to end in current frame.
    // - This is used only for parseImmediate().
    void parse(int block_last);

    void invalidateBlockFormats();

private slots:
    void parsePartial();

    bool canParse();
};

#endif // DOCUMENTPARSER_H
