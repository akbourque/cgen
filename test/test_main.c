#define _GNU_SOURCE /// Exposes POSIX extensions
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "mock_struct.h"


// Include the compile-time generated fixtures
#include "vec_int.h"
#include "vec_custom.h"


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

int main(void) {
    printf("Executing primitive container assertions...\n");
    test_vector_primitive();

    printf("Executing dynamic element callback deep-free assertions...\n");
    test_vector_deep_free();

    return 0;
}
