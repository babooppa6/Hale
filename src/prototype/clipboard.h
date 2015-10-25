#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QObject>
#include <QClipboard>
#include <QStringList>

class Clipboard : public QObject
{
    Q_OBJECT
public:
    static Clipboard *instance;

    explicit Clipboard(QObject *parent = 0);

    QClipboard *clipboard();

    void copy(const QString &text);
    QString paste();

    QStringList ring;

signals:

public slots:
};

#endif // CLIPBOARD_H
