#ifndef HALE_TEST_H
#define HALE_TEST_H

#if HALE_INCLUDES
#include "hale.h"
#endif

namespace hale {

struct TestCoverage {
    struct Branch {
        char *name;
        char *file;
        memi id;
        memi line;
        memi hit;
    };
    Vector<Branch> branches;
    memi branches_total;
};

void test_coverage_init(TestCoverage *coverage);
void test_coverage_branch_hit(TestCoverage *coverage, char *name, char *file, memi line, memi id);

#ifdef HALE_TEST
#define HALE_TEST_COVERAGE_BRANCH(coverage, name) test_coverage_branch_hit(coverage, (char*)name, (char*)__FILE__, __LINE__, __COUNTER__)
#else
#define HALE_TEST_COVERAGE_BRANCH(coverage, name)
#endif

#define hale_test hale_assert

} // namespace hale

#endif // HALE_TEST_H

