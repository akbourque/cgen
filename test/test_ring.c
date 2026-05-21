#include <stdio.h>
#include <assert.h>
#include <string.h>

// Forward declarations to satisfy strict C11 compiler checks
char *strdup(const char *s);
void free_custom_element(custom_t *element);
#include "ring_int.h"
#include "ring_custom_t.h"

void test_ring_primitive(void) {
    ring_int_t r;
    ring_int_init(&r, 3);

    // 1. Verify standard lossless push restrictions
    assert(ring_int_push(&r, 10) == true);
    assert(ring_int_push(&r, 20) == true);
    assert(ring_int_push(&r, 30) == true);
    assert(ring_int_push(&r, 40) == false); // Explicitly rejected (Full!)

    int val = 0;
    assert(ring_int_pop(&r, &val) == true);
    assert(val == 10); // FIFO order validation
    assert(ring_int_pop(&r, &val) == true);
    assert(val == 20);

    // 2. Verify lossy circulating overwrite mechanics
    ring_int_push_overwrite(&r, 50); // Overwrites slot and advances tracking
    
    assert(ring_int_pop(&r, &val) == true);
    assert(val == 30);
    assert(ring_int_pop(&r, &val) == true);
    assert(val == 50);
    assert(ring_int_pop(&r, &val) == false); // Empty!

    ring_int_free(&r);
}

void test_ring_complex(void) {
    ring_custom_t r;
    ring_custom_init(&r, 2);

    custom_t item1 = { .id = 1, .heap_string = strdup("FIFO Record A") };
    custom_t item2 = { .id = 2, .heap_string = strdup("FIFO Record B") };

    assert(ring_custom_push(&r, item1) == true);
    assert(ring_custom_push(&r, item2) == true);

    custom_t popped;
    assert(ring_custom_pop(&r, &popped) == true);
    assert(popped.id == 1);
    free_custom_element(&popped);

    assert(ring_custom_pop(&r, &popped) == true);
    assert(popped.id == 2);
    free_custom_element(&popped);

    ring_custom_free(&r);
}

void run_ring_tests(void) {
    printf("  [ring] Executing bounded circular ring buffer assertions...\n");
    test_ring_primitive();
    test_ring_complex();
}
