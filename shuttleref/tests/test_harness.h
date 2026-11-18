#ifndef TEST_HARNESS_H
#define TEST_HARNESS_H
#include <stdio.h>
static int g_fail = 0, g_run = 0;
#define TEST(name) static void name(void)
#define RUN(name) do { g_run++; printf("  %s ... ", #name); name(); printf("ok\n"); } while (0)
#define ASSERT(cond) do { if (!(cond)) { g_fail++; \
  printf("\nFAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); return; } } while (0)
#define ASSERT_EQ(a,b) do { long _a=(long)(a), _b=(long)(b); if (_a!=_b) { g_fail++; \
  printf("\nFAIL %s:%d: %ld != %ld\n", __FILE__, __LINE__, _a, _b); return; } } while (0)
#define TEST_MAIN_END() do { printf("%d run, %d failed\n", g_run, g_fail); \
  return g_fail ? 1 : 0; } while (0)
#endif
