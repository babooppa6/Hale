#ifndef DOCUMENTBLOCKLAYOUT_H
#define DOCUMENTBLOCKLAYOUT_H

#include <QTextLayout>

class DocumentBlockLayout
{
public:
    DocumentBlockLayout();

    QTextLayout *layout() {
        return m_layout;
    }

    void setLayout(QTextLayout *layout);

    QRectF layoutRect() const {
        return m_layout_rect;
    }

private:
    QTextLayout *m_layout;
    QRectF m_layout_rect;
};

#endif // DOCUMENTBLOCKLAYOUT_H
