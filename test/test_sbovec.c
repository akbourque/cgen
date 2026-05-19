#include "sbovec_int.h"
#include "sbovec_custom.h"

void test_sbovec_primitive(void) {
    sbovec_int_t v;
    int local_scratchpad[4];
    sbovec_int_init_with_buf(&v, local_scratchpad, 4);
    
    assert(v.data == local_scratchpad);
    assert(v.len == 0);
    assert(v.cap == 4);
    assert(v.is_heap == false);

    for (int i = 0; i < 4; i++) {
        sbovec_int_push(&v, i * 100);
    }
    assert(v.len == 4);
    assert(v.is_heap == false);

    sbovec_int_push(&v, 9999);
    assert(v.len == 5);
    assert(v.cap == 8);
    assert(v.is_heap == true);
    assert(v.data != local_scratchpad);

    sbovec_int_free(&v);
}

void test_sbovec_deep_free(void) {
    sbovec_custom_t v;
    custom_t stack_scratch[2];
    sbovec_custom_init_with_buf(&v, stack_scratch, 2);

    custom_t item1 = { .id = 101, .heap_string = strdup("SBO External Allocation A") };
    custom_t item2 = { .id = 202, .heap_string = strdup("SBO External Allocation B") };

    sbovec_custom_push(&v, item1);
    sbovec_custom_push(&v, item2);

    sbovec_custom_deep_free(&v, free_custom_element);
    assert(v.len == 0);
}

// Master execution entrypoint called by the driver
void run_sbovec_tests(void) {
    printf("  [sbovec] Executing small-buffer optimized primitive assertions...\n");
    test_sbovec_primitive();

    printf("  [sbovec] Executing small-buffer optimized callback deep-free assertions...\n");
    test_sbovec_deep_free();
}
