// A minimal test harness: CHECK records a failure and keeps going, and main()
// returns the failure count, so ctest reports every broken expectation at once.
#pragma once

#include <cstdio>

namespace atman_test {
inline int failures = 0;
inline int checks = 0;
}  // namespace atman_test

#define CHECK(cond)                                                                 \
    do {                                                                            \
        ++atman_test::checks;                                                    \
        if (!(cond)) {                                                              \
            ++atman_test::failures;                                              \
            std::fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #cond); \
        }                                                                           \
    } while (0)

#define TEST_MAIN_END()                                                                       \
    std::printf("%d checks, %d failed\n", atman_test::checks, atman_test::failures);  \
    return atman_test::failures == 0 ? 0 : 1
