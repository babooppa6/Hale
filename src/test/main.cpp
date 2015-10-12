#include <QString>
#include <QtTest>

#include "scopeselectortest.h"
#include "stringdistancetest.h"
#include "pathutiltest.h"

#define TEST(Cl) { Cl t; QTest::qExec(&t); }

int main(int argv, char *args[])
{
    TEST(ScopeSelectorTest);
    TEST(PathUtilTest);
    TEST(StringDistanceTest);

    return 1;
}
