#define _GNU_SOURCE /// Exposes POSIX extensions
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "mock_struct.h"


// Include the compile-time generated fixtures
#include "vec_int.h"
#include "vec_custom.h"
#include "sbovec_int.h"
#include "sbovec_custom.h"


// Implementation of the cgen-compliant deep free callback function pointer
void free_custom_element(custom_t *item) {
    if (item->heap_string != NULL) {
        free(item->heap_string);
        item->heap_string = NULL;
    }
}

/**
 * @brief Rigorous verification suite for the generated Vector container code.
 */
void test_vector_primitive(void) {
    vec_int_t v;
    vec_int_init(&v);
    
    // Assert structural invariants on initialization
    assert(v.len == 0);
    assert(v.cap == 0);
    assert(v.data == NULL);

    // Test growth and appending boundaries
    vec_int_push(&v, 100);
    vec_int_push(&v, 200);
    vec_int_push(&v, 300);
    
    assert(v.len == 3);
    assert(v.cap >= 3);
    assert(v.data[0] == 100);
    assert(v.data[1] == 200);
    assert(v.data[2] == 300);

    // Test lookups and pop bounds
    int *back = vec_int_back(&v);
    assert(back != NULL && *back == 300);

    bool popped = vec_int_pop(&v);
    assert(popped == true);
    assert(v.len == 2);
    assert(*vec_int_back(&v) == 200);

    vec_int_free(&v);
}

/**
 * @brief Verification suite for deep-free callback lifecycles inside collections.
 */
void test_vector_deep_free(void) {
    vec_custom_t v;
    vec_custom_init(&v);

    custom_t item1 = { .id = 1, .heap_string = strdup("Deep Allocation A") };
    custom_t item2 = { .id = 2, .heap_string = strdup("Deep Allocation B") };

    vec_custom_push(&v, item1);
    vec_custom_push(&v, item2);

    assert(v.len == 2);

    // Run your brand-new template deep destruction function loop!
    vec_custom_deep_free(&v, free_custom_element);
    
    // Invariants post-freeing
    assert(v.len == 0);
    assert(v.data == NULL);
}

/**
 * @brief Rigorous verification suite for External-Buffer Borrowing SBO Vectors.
 */
void test_sbovec_primitive(void) {
    sbovec_int_t v;
    
    // Provision a local stack array of 4 elements to borrow
    int local_scratchpad[4];
    sbovec_int_init_with_buf(&v, local_scratchpad, 4);
    
    // 1. Assert initial invariants matching your buffer insertion rules
    assert(v.data == local_scratchpad); // Pointer must point to stack
    assert(v.len == 0);
    assert(v.cap == 4);
    assert(v.is_heap == false); // Should not have touched the allocator yet

    // 2. Fill the container up to the exact threshold of your borrowed array
    for (int i = 0; i < 4; i++) {
        sbovec_int_push(&v, i * 100);
    }
    assert(v.len == 4);
    assert(v.cap == 4);
    assert(v.is_heap == false); // Still safely tracking on the local stack frame

    // 3. Trigger a boundary breach! This must force allocation escalation
    sbovec_int_push(&v, 9999);
    
    // Invariants post-escalation
    assert(v.len == 5);
    assert(v.cap == 8);          // Capacity doubled cleanly from 4
    assert(v.is_heap == true);   // Heap flag flipped successfully
    assert(v.data != local_scratchpad); // Pointer migrated safely to the dynamic heap
    assert(*sbovec_int_back(&v) == 9999);

    // 4. Teardown should safely free the heap allocation without touching the stack frame memory
    sbovec_int_free(&v);
    assert(v.data == NULL);
    assert(v.is_heap == false);
}

/**
 * @brief Verification suite for SBO elements using dynamic callback destruction.
 */
void test_sbovec_deep_free(void) {
    sbovec_custom_t v;
    
    // Allocate space for 2 elements locally on the stack
    custom_t stack_scratch[2];
    sbovec_custom_init_with_buf(&v, stack_scratch, 2);

    custom_t item1 = { .id = 101, .heap_string = strdup("SBO External Allocation A") };
    custom_t item2 = { .id = 202, .heap_string = strdup("SBO External Allocation B") };

    sbovec_custom_push(&v, item1);
    sbovec_custom_push(&v, item2);

    assert(v.len == 2);
    assert(v.is_heap == false);

    // Run your customized template deep destruction loop!
    sbovec_custom_deep_free(&v, free_custom_element);
    
    // Invariants post-freeing
    assert(v.len == 0);
}

int main(void) {
    printf("Executing primitive container assertions...\n");
    test_vector_primitive();

    printf("Executing dynamic element callback deep-free assertions...\n");
    test_vector_deep_free();

    printf("Executing small-buffer optimized assertions...\n");
    test_sbovec_primitive();

    printf("Executing small-buffer optimized callback deep-free assertions...\n");
    test_sbovec_deep_free();
    return 0;
}
 
