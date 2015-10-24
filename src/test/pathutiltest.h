#ifndef PATHUTILTEST_H
#define PATHUTILTEST_H

#include <QtTest/QtTest>
#include <QObject>

class PathUtilTest : public QObject
{
    Q_OBJECT
public:
    explicit PathUtilTest(QObject *parent = 0);

private slots:
    void parseRequest();
    void findFirstNonExisting();
};

#endif // PATHUTILTEST_H
