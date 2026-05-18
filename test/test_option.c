#include "option_int.h"
#include "option_custom.h"

void test_option_primitive(void) {
    option_int_t o1 = option_int_none();
    assert(option_int_is_none(&o1) == true);
    assert(option_int_is_some(&o1) == false);

    option_int_t o2 = option_int_some(123);
    assert(option_int_is_some(&o2) == true);
    assert(option_int_is_none(&o2) == false);
    assert(option_int_unwrap(o2) == 123);
}

void test_option_complex(void) {
    custom_t payload = { .id = 456, .heap_string = strdup("Option Dynamic Memory context") };
    
    option_custom_t o = option_custom_some(payload);
    assert(option_custom_is_some(&o) == true);

    custom_t extracted = option_custom_unwrap(o);
    assert(extracted.id == 456);
    
    // Safely drop internal allocation from the unwrapped option payload
    free_custom_element(&extracted);
}

void run_option_tests(void) {
    printf("  [option] Executing type-safe Option monadic assertions...\n");
    test_option_primitive();
    test_option_complex();
}
