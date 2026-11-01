#include "utils/utils.h"

#include "test_helpers.h"
#include <assert.h>
#include <stdio.h>

static void test_sign_extend(void) {
    // Positive number (highest bit is 0)
    // Should return the original number
    assert(sign_extend(0x7, 5) == 0x7);

    // Negative number (highest bit is 1)
    // Should set the higher bits to 1
    assert(sign_extend(0x9, 4) == 0xfff9);

    PRINT_PASS;
}

static void test_swap16(void) {
    assert(swap16(0x0000) == 0x0000);
    assert(swap16(0xff00) == 0x00ff);
    assert(swap16(0x1234) == 0x3412);
    assert(swap16(0xffff) == 0xffff);

    PRINT_PASS;
}

void test_utils_main(void) {
    puts("Running unit tests for utils");

    test_sign_extend();
    test_swap16();

    puts("All tests for utils passed successfully");
}
