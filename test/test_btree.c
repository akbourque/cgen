#include "btree_int_int.h"
#include "btree_int_custom.h"

/**
 * @brief Evaluates strict key sorting order invariants.
 * @note Corrected to return positive when m > n, matching the B-Tree template spec.
 */
int compare_fn_int(const int* m, const int* n) {
    return *m - *n;
}

void test_btree_primitive(void) {
    btree_int_int_t t;
    btree_int_int_init(&t, compare_fn_int);

    // Populate elements into the Order 4 B-Tree container
    btree_int_int_insert(&t, 50, 500);
    btree_int_int_insert(&t, 25, 250);
    btree_int_int_insert(&t, 75, 750);

    // 1. Verify extraction of key 50 using out-parameter allocation targets
    int v1 = 0;
    bool found1 = btree_int_int_get(&t, 50, &v1);
    assert(found1 == true);
    assert(v1 == 500);

    // 2. Verify extraction of key 25
    int v2 = 0;
    bool found2 = btree_int_int_get(&t, 25, &v2);
    assert(found2 == true);
    assert(v2 == 250);

    // 3. Verify lookup boundary failure on non-existent keys
    int v_missing = 0;
    bool found_missing = btree_int_int_get(&t, 999, &v_missing);
    assert(found_missing == false);

    // Release all dynamically balanced multi-way tree nodes
    btree_int_int_free(&t);
}

void run_btree_tests(void) {
    printf("  [btree] Executing primitive B-Tree assertions...\n");
    test_btree_primitive();
}
