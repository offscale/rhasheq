#include <greatest.h>

#define RHASHEQ_IMPLEMENTATION
#include <rhasheq.h>
#include <config_for_tests.h>

TEST x_rhasheq_crc32c_should_succeed(void) {
    ASSERT_EQ(crc32c(GREATEST_FILE, GREATEST_CRC32C), 1);
    PASS();
}

SUITE(rhasheq_suite) {
    RUN_TEST(x_rhasheq_crc32c_should_succeed);
}
