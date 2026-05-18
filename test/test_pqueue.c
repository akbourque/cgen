#include "pqueue_int.h"
#include "pqueue_custom.h"

// Max-Heap Comparator: Returns positive if m > n
int pqueue_cmp_int(const int *m, const int *n) {
    return *m - *n;
}

int pqueue_cmp_custom(const custom_t *m, const custom_t *n) {
    return m->id - n->id;
}

void test_pqueue_primitive(void) {
    pqueue_int_t pq;
    pqueue_int_init(&pq, pqueue_cmp_int);

    // Push values unsorted
    assert(pqueue_int_push(&pq, 20) == true);
    assert(pqueue_int_push(&pq, 50) == true);
    assert(pqueue_int_push(&pq, 10) == true);

    int out = 0;
    // Extract items—must emerge strictly sorted by maximum value priority
    assert(pqueue_int_pop(&pq, &out) == true);
    assert(out == 50); 
    assert(pqueue_int_pop(&pq, &out) == true);
    assert(out == 20);
    assert(pqueue_int_pop(&pq, &out) == true);
    assert(out == 10);
    assert(pqueue_int_pop(&pq, &out) == false);

    pqueue_int_free(&pq);
}

void test_pqueue_complex(void) {
    pqueue_custom_t pq;
    pqueue_custom_init(&pq, pqueue_cmp_custom);

    custom_t item1 = { .id = 30, .heap_string = strdup("Low Priority Heap Allocation") };
    custom_t item2 = { .id = 90, .heap_string = strdup("High Priority Heap Allocation") };
    custom_t item3 = { .id = 10, .heap_string = strdup("Lowest Priority Heap Allocation") };

    pqueue_custom_push(&pq, item1);
    pqueue_custom_push(&pq, item2);
    pqueue_custom_push(&pq, item3);

    custom_t out;
    assert(pqueue_custom_pop(&pq, &out) == true);
    assert(out.id == 90);
    free_custom_element(&out);

    assert(pqueue_custom_pop(&pq, &out) == true);
    assert(out.id == 30);
    free_custom_element(&out);

    assert(pqueue_custom_pop(&pq, &out) == true);
    assert(out.id == 10);
    free_custom_element(&out);

    pqueue_custom_free(&pq);
}

void run_pqueue_tests(void) {
    printf("  [pqueue] Executing binary heap priority queue assertions...\n");
    test_pqueue_primitive();
    test_pqueue_complex();
}
