#include "util.cpp"
#include "stringdistancetest.h"

StringDistanceTest::StringDistanceTest(QObject *parent)
    : QObject(parent)
{

}

void StringDistanceTest::basicDistance()
{
    QVERIFY(StringDistance::distance("wr", "world") == 1);
    QVERIFY(StringDistance::distance("wrld", "world") == 1);
    QVERIFY(StringDistance::distance("wo", "world") == 0);

    QVERIFY(StringDistance::distance("or", "world") == 1);
    QVERIFY(StringDistance::distance("rl", "world") == 2);
}


void StringDistanceTest::findPrevious()
{
    const QString path("/how/are/you");
    const QChar *b = path.constData();
    const QChar *i = b + path.length();
    i = StringDistance::findPrevious(i, b, "/");
    QVERIFY(QString(i) == "/you");
    i = StringDistance::findPrevious(i, b, "/");
    QVERIFY(QString(i) == "/are/you");
    i = StringDistance::findPrevious(i, b, "/");
    QVERIFY(QString(i) == "/how/are/you");
    i = StringDistance::findPrevious(i, b, "/");
    QVERIFY(i == NULL);
}

static void matchf(int offset) {
    // qDebug() << offset;
}

void StringDistanceTest::pathDistance()
{
    // int d;

    QVERIFY(StringDistance::pathDistance("h/yu", "how/are/you", "/", matchf) == 0+(7)+1+(0));
    QVERIFY(StringDistance::pathDistance("h/r/yu", "how/are/you", "/", matchf) == 0+(4)+1+(2)+1+(0));
}
