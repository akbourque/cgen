#include <stdio.h>
#include <assert.h>
#include <string.h>

// Forward declarations to satisfy independent C11 module compilation
char *strdup(const char *s);
void free_custom_element(custom_t *element);
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "map_int_int.h"
#include "map_int_custom.h"
#include "map_iter_int_custom.h"

char* strdup(const char*);
/**
 * @brief Verifies primitive key-value functionality on SwissTable maps.
 */
void test_map_primitive(void) {
    map_int_int_t m;
    map_int_int_init(&m);

    // Assert your map's standard insert and lookup API mechanics
    map_int_int_insert(&m, 42, 999);
    map_int_int_insert(&m, 100, 555);

    int *val1 = map_int_int_get(&m, 42);
    int *val2 = map_int_int_get(&m, 100);
    int *val_missing = map_int_int_get(&m, 9999);

    assert(val1 != NULL && *val1 == 999);
    assert(val2 != NULL && *val2 == 555);
    assert(val_missing == NULL);

    map_int_int_free(&m);
}

/**
 * @brief Verifies deep destruction cleanup of complex map values.
 */
void test_map_deep_free(void) {
    map_int_custom_t m;
    map_int_custom_init(&m);

    custom_t item1 = { .id = 7, .heap_string = strdup("Map Value Allocation A") };
    
    map_int_custom_insert(&m, 1, item1);

    // If your maps don't have a deep_free lifecycle utility built into their 
    // templates yet, you can skip this test or implement standard shallow free.
    map_int_custom_deep_free(&m, free_custom_element);
}

void test_map_iterator_basic(void) {
    map_int_custom_t m;
    map_int_custom_init(&m);

    custom_t c1 = { .id = 100, .heap_string = NULL };
    custom_t c2 = { .id = 200, .heap_string = NULL };
    custom_t c3 = { .id = 300, .heap_string = NULL };

    map_int_custom_insert(&m, 10, c1);
    map_int_custom_insert(&m, 20, c2);
    map_int_custom_insert(&m, 30, c3);

    // 1. Test Multi-Pointer Unpacking Iterator
    map_iter_int_custom_t iter1 = map_iter_int_custom_new(&m); // ✨ Fixed type
    int key;
    custom_t *val;
    size_t count1 = 0;

    while (map_iter_int_custom_next(&iter1, &key, &val)) {
        assert(val != NULL);
        assert(key == 10 || key == 20 || key == 30);
        count1++;
    }
    assert(count1 == 3);

    // 2. Test Packaged Pair Structure Iterator
    map_iter_int_custom_t iter2 = map_iter_int_custom_new(&m);      // ✨ Fixed type
    map_iter_int_custom_pair_t pair;                               // ✨ Fixed type
    size_t count2 = 0;

    while (map_iter_int_custom_next_pair(&iter2, &pair)) {
        assert(pair.value != NULL);
        pair.value->id += 5; 
        count2++;
    }
    assert(count2 == 3);

    custom_t *checked_c1 = map_int_custom_get(&m, 10);
    assert(checked_c1 != NULL);
    assert(checked_c1->id == 105);

    map_int_custom_free(&m);
    printf("  ✅ SwissTable dual-style iterators verified flawlessly.\n");
}

// Master execution entrypoint called by the driver
void run_map_tests(void) {
    printf("  [map] Executing primitive SwissTable map assertions...\n");
    test_map_primitive();

    printf("  [map] Executing SwissTable callback deep-free assertions...\n");
    test_map_deep_free();

    test_map_iterator_basic(); 
}
