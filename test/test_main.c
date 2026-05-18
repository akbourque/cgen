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
#include "test_vec.c"
#include "test_sbovec.c"
#include "test_map.c"


int main(void) {
    printf("Executing automated validation suites...\n");

    // Route execution down to modular testing files cleanly
    run_vec_tests();
    run_sbovec_tests();
    run_map_tests();
    return 0;
}
 
