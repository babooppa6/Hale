#ifndef SCOPESELECTORTEST_H
#define SCOPESELECTORTEST_H

#include <QtTest/QtTest>
#include <QObject>

class ScopeSelectorTest : public QObject
{
    Q_OBJECT
public:
    explicit ScopeSelectorTest(QObject *parent = 0);

private slots:
    void score();
    void highestMatchWins();
};

#endif // SCOPESELECTORTEST_H
