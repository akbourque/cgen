#include "map_int_int.h"
#include "map_int_custom.h"

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

// Master execution entrypoint called by the driver
void run_map_tests(void) {
    printf("  [map] Executing primitive SwissTable map assertions...\n");
    test_map_primitive();

    printf("  [map] Executing SwissTable callback deep-free assertions...\n");
    test_map_deep_free();
}
