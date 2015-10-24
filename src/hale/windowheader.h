#ifndef WINDOWHEADER_H
#define WINDOWHEADER_H

#include <QWidget>

class WindowHeader : public QWidget
{
    Q_OBJECT
public:
    explicit WindowHeader(QWidget *parent = 0);

    QSize sizeHint() const;

signals:

public slots:
};

#endif // WINDOWHEADER_H
