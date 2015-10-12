#include "scopeselectortest.h"

#include "scopepath.cpp"

ScopeSelectorTest::ScopeSelectorTest(QObject *parent) : QObject(parent)
{}

void ScopeSelectorTest::score()
{
    ScopeScore s1, s2;

    s1.reset(); s1.plus(0, 3);
    s2.reset(); s2.plus(1, 3);
    Q_ASSERT(s2 > s1);

    s1.reset(); s1.plus(0, 3);
    s2.reset(); s2.plus(1, 3);
    Q_ASSERT(s2 > s1);

    s1.reset(); s1.plus(0, 3);
    s2.reset(); s2.plus(0, 5);
    Q_ASSERT(s2 > s1);
}

void ScopeSelectorTest::highestMatchWins()
{
    ScopePath path;
    path.push("app");
    path.push("document");

    ScopeScore s1, s2;

    Q_ASSERT(path.match(ScopeSelector("app"), &s1));
    Q_ASSERT(path.match(ScopeSelector("document"), &s2));
    Q_ASSERT(s2 > s1);

    // Path length is 2, so the score for matching would be:
    // `app` - 1
    // `document` - 2
    // end - 4
    // That means that partial matches of "document" should start at 2, and end before 4.
    Q_ASSERT(path.match(ScopeSelector("app"), &s1));
    Q_ASSERT(path.match(ScopeSelector("app document"), &s2));
    Q_ASSERT(s2 > s1);
}

