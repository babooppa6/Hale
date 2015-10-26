#if HALE_INCLUDES
#include "hale_test.h"
#endif

namespace hale {

void
test_coverage_init(TestCoverage *coverage)
{
    hale_assert_debug(coverage);
    vector_init(&coverage->branches);
    coverage->branches_total = 0;
}

void
test_coverage_branch_hit(TestCoverage *coverage,
                         char *name,
                         char *file,
                         memi line,
                         memi id)
{
    for (memi i = 0; i < vector_count(&coverage->branches); i++) {
        if (coverage->branches[i].id == id) {
            coverage->branches[i].hit++;
            return;
        }
    }

    TestCoverage::Branch branch;
    branch.name = name;
    branch.file = file;
    branch.line = line;
    branch.id = id;
    branch.hit = 1;

    vector_push(&coverage->branches, branch);
}

} // namespace hale
