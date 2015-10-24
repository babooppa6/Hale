#ifndef FRAMELESS_H
#define FRAMELESS_H

#include <QWidget>

class Frameless
{
public:
    static void init(QWidget *widget);
    static void setWindowFlags(QWidget *widget);
    static bool event(QWidget *widget, QEvent *event);
    static bool nativeEvent(QWidget *widget, const QByteArray &eventType, void *mptr, long *rptr);
};

#endif // FRAMELESS_H
