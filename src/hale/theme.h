#ifndef THEME_H
#define THEME_H

#include <QTextLayout>
#include "parser.h"

class Theme
{
public:
    Theme();

    void apply(Parser::Tokens &tokens, QList<QTextLayout::FormatRange> *formats);

    QColor foregroundColor();
    QColor foregroundDimColor();
    QColor foregroundBrightColor();
    QColor backgroundColor();
    QColor selectionColor();
    QColor lineNumbersColor();
    QColor caretColor();

signals:

public slots:

};

#endif // THEME_H
