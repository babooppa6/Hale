#ifndef STRINGDISTANCETEST_H
#define STRINGDISTANCETEST_H

#include <QtTest/QtTest>
#include <QObject>

class StringDistanceTest : public QObject
{
    Q_OBJECT
public:
    explicit StringDistanceTest(QObject *parent = 0);

private slots:
    void basicDistance();
    void findPrevious();
    void pathDistance();
};

#endif // STRINGDISTANCETEST_H
