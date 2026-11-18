#include "test_harness.h"
TEST(sanity) { ASSERT_EQ(1, 1); }
int main(void) { RUN(sanity); TEST_MAIN_END(); }
