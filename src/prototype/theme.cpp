#include "theme.h"

Theme::Theme()
{
}

void Theme::apply(Parser::Tokens &tokens, QList<QTextLayout::FormatRange> *formats)
{
    for (Parser::Token &token : tokens)
    {
        QTextLayout::FormatRange r;
        QColor color = QColor(foregroundColor());
        if (token.name.startsWith("number")) {
            color = QColor(0xd08770);
        } else if (token.name.startsWith("keyword")){
            color = QColor(0xb48ead);
        } else if (token.name.startsWith("string")) {
            color = QColor(0xa3be8c);
        } else if (token.name.startsWith("comment")) {
            color = QColor(0x65737e);
        }

        r.format.setForeground(color);
        r.start = token.begin;
        r.length = token.end - token.begin;

        formats->append(r);
    }
}


QColor Theme::foregroundColor()
{
    return QColor(0xe0e0dc);
}

QColor Theme::foregroundDimColor()
{
    return QColor(0x6d7480);
}

QColor Theme::foregroundBrightColor()
{
    return QColor(0xFFFFFF);
}

QColor Theme::backgroundColor()
{
    return QColor(0x151C24) ; // 0x1D262B);
}

QColor Theme::selectionColor()
{
    return QColor(0x4f5b66);
}

QColor Theme::caretColor()
{
    return QColor(0xc0c5ce);
}

QColor Theme::lineNumbersColor()
{
    return QColor(0x6d7480);
}
