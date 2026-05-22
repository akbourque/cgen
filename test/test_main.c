#define _GNU_SOURCE /// Exposes POSIX extensions
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "mock_struct.h"

// Implementation of the cgen-compliant deep free callback function pointer
void free_custom_element(custom_t *item) {
    if (item->heap_string != NULL) {
        free(item->heap_string);
        item->heap_string = NULL;
    }
}

// Unity Include Architecture: Pull modules directly into this translation unit
void run_vec_tests();
void run_sbovec_tests();
void run_map_tests();
void run_btree_tests();
void run_result_tests();
void run_option_tests();
void run_ring_tests();
void run_pqueue_tests();
void run_variant_tests();
void run_parser_tests();

int main(void) {
    printf("Executing automated validation suites...\n");

    // Route execution down to modular testing files cleanly
    run_parser_tests();
    run_vec_tests();
    run_sbovec_tests();
    run_map_tests();
    run_btree_tests();
    run_result_tests();
    run_option_tests();
    run_ring_tests();
    run_pqueue_tests();
    run_variant_tests();

    return 0;
}

