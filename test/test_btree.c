#include <stdio.h>
#include <assert.h>
#include <string.h>

// Forward declarations to satisfy independent C11 module compilation
char *strdup(const char *s);
void free_custom_element(custom_t *element);

#include <assert.h>
#include "btree_int_int.h"
#include "btree_int_custom.h"
#include "btree_iter_int_custom.h"

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

// Standalone comparison utility to match your collection specifications
static int dummy_int_cmp(const int *a, const int *b) {
    return (*a > *b) - (*a < *b);
}

void test_btree_iterator_basic(void) {
    btree_int_custom_t t;
    btree_int_custom_init(&t, dummy_int_cmp);

    // 1. Insert scrambled keys to trigger splits and child balancing adjustments
    custom_t c1 = { .id = 10, .heap_string = NULL };
    custom_t c2 = { .id = 20, .heap_string = NULL };
    custom_t c3 = { .id = 30, .heap_string = NULL };
    custom_t c4 = { .id = 40, .heap_string = NULL };

    btree_int_custom_insert(&t, 30, c3);
    btree_int_custom_insert(&t, 10, c1);
    btree_int_custom_insert(&t, 40, c4);
    btree_int_custom_insert(&t, 20, c2);

    // 2. Execute tracking assertions using the unpacked iterator
    btree_iter_int_custom_t iter = btree_iter_int_custom_new(&t);
    int key;
    custom_t *val;
    int last_key = -1;
    size_t count = 0;

    while (btree_iter_int_custom_next(&iter, &key, &val)) {
        assert(val != NULL);
        // Verify strict in-order sort sequence sorting: 10 -> 20 -> 30 -> 40
        assert(key > last_key);
        last_key = key;
        count++;
    }
    assert(count == 4);
    assert(last_key == 40);

    // 3. Test packaged structure pair modifications
    btree_iter_int_custom_t iter2 = btree_iter_int_custom_new(&t);
    btree_iter_int_custom_pair_t pair;
    while (btree_iter_int_custom_next_pair(&iter2, &pair)) {
        pair.value->id += 100; // Modify values in-place inside the tree slots
    }

    // 4. Confirm modification updates stuck
    custom_t check;
    bool found = btree_int_custom_get(&t, 30, &check);
    assert(found);
    assert(check.id == 130); // 30 + 100

    btree_int_custom_free(&t);
    printf("  ✅ B-Tree explicit stack iterators verified flawlessly.\n");
}

void run_btree_tests(void) {
    printf("  [btree] Executing primitive B-Tree assertions...\n");
    test_btree_primitive();
    test_btree_iterator_basic();
}
