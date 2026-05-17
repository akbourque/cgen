#include <string.h>
#include "cgen_framework.h"

// ============================================================================
// 1. HEADER ASSET TEMPLATE (WITH FULL DOXYGEN DOCUMENTATION)
// ============================================================================
const char *RING_TEMPLATE_H =
    "#pragma once\n"
    "#ifndef RING_{{00BU}}_H\n"
    "#define RING_{{00BU}}_H\n\n"
    "#include <stddef.h>\n"
    "#include <stdbool.h>\n\n"
    "/**\n"
    " * @brief A fixed-allocation, run-time bounded circular FIFO queue.\n"
    " */\n"
    "typedef struct {\n"
    "    {{00}} *data;       /**< Contiguous internal memory block. */\n"
    "    size_t capacity;   /**< Total maximum capacity of the queue. */\n"
    "    size_t head;       /**< Index of the oldest active element (Read). */\n"
    "    size_t tail;       /**< Index of the next storage slot (Write). */\n"
    "    size_t len;        /**< Active count tracking to differentiate Empty vs Full. */\n"
    "} ring_{{00B}}_t;\n\n"
    
    "/**\n"
    " * @brief Initializes the ring buffer with a rigid capacity footprint.\n"
    " *\n"
    " * Allocates memory on the heap exactly once. This allocation size remains fixed\n"
    " * throughout the lifespan of the container to ensure predictable performance.\n"
    " *\n"
    " * @param r Pointer to the ring buffer instance to initialize.\n"
    " * @param capacity The maximum number of elements the buffer can physically hold.\n"
    " */\n"
    "void ring_{{00B}}_init(ring_{{00B}}_t *r, size_t capacity);\n\n"
    
    "/**\n"
    " * @brief Deep cleans all memory footprints and zeroes internal tracking states.\n"
    " *\n"
    " * Frees the underlying allocated heap data block safely and zero-initializes members\n"
    " * to protect against dangling references.\n"
    " *\n"
    " * @param r Pointer to the active ring buffer instance to clean up.\n"
    " */\n"
    "void ring_{{00B}}_free(ring_{{00B}}_t *r);\n\n"
    
    "/**\n"
    " * @brief Lossless safe push. Appends an element to the tail of the buffer.\n"
    " *\n"
    " * If the buffer has reached maximum capacity, the operation is explicitly rejected\n"
    " * to protect against data loss.\n"
    " *\n"
    " * @param r Pointer to the active ring buffer instance.\n"
    " * @param item The element payload to push into the queue.\n"
    " * @return true if the element was successfully appended; false if the queue is entirely full.\n"
    " */\n"
    "bool ring_{{00B}}_push(ring_{{00B}}_t *r, {{00}} item);\n\n"
    
    "/**\n"
    " * @brief Lossy circulating push. Forces an item into the queue, overwriting data if full.\n"
    " *\n"
    " * If the buffer is completely full, this operation silently evicts the oldest stored element\n"
    " * by automatically advancing the head pointer forward before tracking the new element.\n"
    " *\n"
    " * @param r Pointer to the active ring buffer instance.\n"
    " * @param item The element payload to push into the queue.\n"
    " */\n"
    "void ring_{{00B}}_push_overwrite(ring_{{00B}}_t *r, {{00}} item);\n\n"
    
    "/**\n"
    " * @brief Extracts the oldest element from the head of the buffer.\n"
    " *\n"
    " * Cleans up elements and removes them in strict First-In, First-Out (FIFO) ordering.\n"
    " *\n"
    " * @param r Pointer to the active ring buffer instance.\n"
    " * @param out_item Pointer to a storage destination where the popped value will be written.\n"
    " * Can be safely set to NULL if you merely wish to discard the oldest item.\n"
    " * @return true if an item was successfully extracted; false if the buffer is entirely empty.\n"
    " */\n"
    "bool ring_{{00B}}_pop(ring_{{00B}}_t *r, {{00}} *out_item);\n\n"
    
    "#endif\n";

// ============================================================================
// 2. SOURCE ASSET TEMPLATE
// ============================================================================
const char *RING_TEMPLATE_C =
    "#include <stdlib.h>\n"
    "#include \"ring_{{00B}}.h\"\n\n"
    
    "void ring_{{00B}}_init(ring_{{00B}}_t *r, size_t capacity) {\n"
    "    if (r == NULL) return;\n"
    "    r->data = malloc(sizeof({{00}}) * capacity);\n"
    "    r->capacity = capacity;\n"
    "    r->head = 0;\n"
    "    r->tail = 0;\n"
    "    r->len = 0;\n"
    "}\n\n"
    
    "void ring_{{00B}}_free(ring_{{00B}}_t *r) {\n"
    "    if (r == NULL) return;\n"
    "    if (r->data != NULL) {\n"
    "        free(r->data);\n"
    "        r->data = NULL;\n"
    "    }\n"
    "    r->capacity = 0;\n"
    "    r->head = 0;\n"
    "    r->tail = 0;\n"
    "    r->len = 0;\n"
    "}\n\n"
    
    "bool ring_{{00B}}_push(ring_{{00B}}_t *r, {{00}} item) {\n"
    "    if (r == NULL || r->data == NULL) return false;\n"
    "    if (r->len == r->capacity) {\n"
    "        return false; // Queue is full, push rejected\n"
    "    }\n"
    "    r->data[r->tail] = item;\n"
    "    r->tail = (r->tail + 1) % r->capacity;\n"
    "    r->len++;\n"
    "    return true;\n"
    "}\n\n"
    
    "void ring_{{00B}}_push_overwrite(ring_{{00B}}_t *r, {{00}} item) {\n"
    "    if (r == NULL || r->data == NULL) return;\n"
    "    r->data[r->tail] = item;\n"
    "    r->tail = (r->tail + 1) % r->capacity;\n"
    "    if (r->len == r->capacity) {\n"
    "        // Drop oldest record by pushing head pointer forward\n"
    "        r->head = (r->head + 1) % r->capacity;\n"
    "    } else {\n"
    "        r->len++;\n"
    "    }\n"
    "}\n\n"
    
    "bool ring_{{00B}}_pop(ring_{{00B}}_t *r, {{00}} *out_item) {\n"
    "    if (r == NULL || r->data == NULL || r->len == 0) {\n"
    "        return false;\n"
    "    }\n"
    "    if (out_item != NULL) {\n"
    "        *out_item = r->data[r->head];\n"
    "    }\n"
    "    r->head = (r->head + 1) % r->capacity;\n"
    "    r->len--;\n"
    "    return true;\n"
    "}\n";

// ============================================================================
// 3. APPLICATION ENTRYPOINT
// ============================================================================
int main(int argc, char **argv) {
    cgen_app_def_t app = {
        .subcommand_name = "ring",
        .opt_spec        = "=ttypename",
        .template_h      = RING_TEMPLATE_H,
        .template_c      = RING_TEMPLATE_C
    };

    // Forward parsing configurations down to the shared engine framework
    return cgen_app_run(&app, argc - 1, argv + 1);
}
