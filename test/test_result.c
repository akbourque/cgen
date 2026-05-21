#include <stdio.h>
#include <assert.h>
#include <string.h>

// Forward declarations to satisfy strict C11 compiler checks
char *strdup(const char *s);
void free_custom_element(custom_t *element);
#include "result_int_int.h"
#include "result_int_custom_t.h"

char* strdup(const char*);
/**
 * @brief Verifies standard primitive monadic success and failure wrappers.
 */
void test_result_primitive(void) {
    // 1. Test success path (OK)
    result_int_int_t r1 = result_int_int_ok(42);
    assert(result_int_int_is_ok(&r1) == true);
    assert(result_int_int_is_err(&r1) == false);
    assert(result_int_int_unwrap(r1) == 42);

    // 2. Test failure path (ERR)
    result_int_int_t r2 = result_int_int_err(504);
    assert(result_int_int_is_ok(&r2) == false);
    assert(result_int_int_is_err(&r2) == true);
    assert(result_int_int_unwrap_err(r2) == 504);
}

/**
 * @brief Verifies result payloads containing dynamic dynamic allocations.
 */
void test_result_complex(void) {
    custom_t payload = { .id = 99, .heap_string = strdup("Result Error Context String") };
    
    // Wrap the allocation inside an error container variant
    result_int_custom_t r = result_int_custom_err(payload);
    assert(result_int_custom_is_err(&r) == true);

    // Unwrap the dynamic error context and release its inner allocations cleanly
    custom_t extracted_err = result_int_custom_unwrap_err(r);
    assert(extracted_err.id == 99);
    
    free_custom_element(&extracted_err);
}

// Master execution entrypoint called by the driver
void run_result_tests(void) {
    printf("  [result] Executing primitive option/result monadic assertions...\n");
    test_result_primitive();

    printf("  [result] Executing complex heap error unwrap assertions...\n");
    test_result_complex();
}
