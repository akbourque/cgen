#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "libpstr.h"
#include "cgen_framework.h"

// --- SwissTable Map Iterator Header Template ---
const char *MAP_ITER_TEMPLATE_H =
    "#pragma once\n"
    "#ifndef MAP_ITER_{{KEY_BU}}_{{VAL_BU}}_H\n"
    "#define MAP_ITER_{{KEY_BU}}_{{VAL_BU}}_H\n\n"
    "#include \"map_{{KEY_B}}_{{VAL_B}}.h\"\n"
    "#include <stdbool.h>\n"
    "#include <stddef.h>\n\n"
    "/**\n"
    " * @brief Ergonomic bundle containing an unpacked entry from an active slot.\n"
    " * @note value is a mutable pointer to allow in-place value modifications.\n"
    " */\n"
    "typedef struct {\n"
    "    {{KEY}} key;\n"
    "    {{VAL}} *value;\n"
    "} map_iter_{{KEY_B}}_{{VAL_B}}_pair_t;\n\n"
    "/**\n"
    " * @brief Iterator state tracking mechanical engine.\n"
    " */\n"
    "typedef struct {\n"
    "    const map_{{KEY_B}}_{{VAL_B}}_t *map; /**< Read-only reference to the source map. */\n"
    "    size_t idx;                  /**< Current slot bucket position counter. */\n"
    "} map_iter_{{KEY_B}}_{{VAL_B}}_t;\n\n"
    "/**\n"
    " * @brief Allocates and initializes a fresh map iterator instance.\n"
    " */\n"
    "map_iter_{{KEY_B}}_{{VAL_B}}_t map_iter_{{KEY_B}}_{{VAL_B}}_new(const map_{{KEY_B}}_{{VAL_B}}_t *map);\n\n"
    "/**\n"
    " * @brief Multiplexed pointer unpacking iterator. Advances state to next valid element.\n"
    " * @return true if an active element was populated into out parameters, false if complete.\n"
    " */\n"
    "bool map_iter_{{KEY_B}}_{{VAL_B}}_next(map_iter_{{KEY_B}}_{{VAL_B}}_t *iter, {{KEY}} *out_key, {{VAL}} **out_val);\n\n"
    "/**\n"
    " * @brief Packaged pair structure iterator. Advances state to next valid element.\n"
    " * @return true if an active element was populated into out_pair, false if complete.\n"
    " */\n"
    "bool map_iter_{{KEY_B}}_{{VAL_B}}_next_pair(map_iter_{{KEY_B}}_{{VAL_B}}_t *iter, map_iter_{{KEY_B}}_{{VAL_B}}_pair_t *out_pair);\n\n"
    "#endif\n";

// --- SwissTable Map Iterator Implementation Template ---
const char *MAP_ITER_TEMPLATE_C =
    "#include \"map_iter_{{KEY_B}}_{{VAL_B}}.h\"\n"
    "#include <stdlib.h>\n\n"
    "map_iter_{{KEY_B}}_{{VAL_B}}_t map_iter_{{KEY_B}}_{{VAL_B}}_new(const map_{{KEY_B}}_{{VAL_B}}_t *map) {\n"
    "    map_iter_{{KEY_B}}_{{VAL_B}}_t iter;\n"
    "    iter.map = map;\n"
    "    iter.idx = 0;\n"
    "    return iter;\n"
    "}\n\n"
    "bool map_iter_{{KEY_B}}_{{VAL_B}}_next(map_iter_{{KEY_B}}_{{VAL_B}}_t *iter, {{KEY}} *out_key, {{VAL}} **out_val) {\n"
    "    if (iter->map == NULL || iter->map->ctrl == NULL || iter->map->cap == 0) {\n"
    "        return false;\n"
    "    }\n\n"
    "    while (iter->idx < iter->map->cap) {\n"
    "        uint8_t ctrl = iter->map->ctrl[iter->idx];\n"
    "        if (ctrl != 0xFF && ctrl != 0xFE) {\n"
    "            if (out_key) *out_key = iter->map->slots[iter->idx].key;\n"
    "            if (out_val) *out_val = &iter->map->slots[iter->idx].val;\n"
    "            iter->idx++;\n"
    "            return true;\n"
    "        }\n"
    "        iter->idx++;\n"
    "    }\n"
    "    return false;\n"
    "}\n\n"
    "bool map_iter_{{KEY_B}}_{{VAL_B}}_next_pair(map_iter_{{KEY_B}}_{{VAL_B}}_t *iter, map_iter_{{KEY_B}}_{{VAL_B}}_pair_t *out_pair) {\n"
    "    if (iter->map == NULL || iter->map->ctrl == NULL || iter->map->cap == 0) {\n"
    "        return false;\n"
    "    }\n\n"
    "    while (iter->idx < iter->map->cap) {\n"
    "        uint8_t ctrl = iter->map->ctrl[iter->idx];\n"
    "        if (ctrl != 0xFF && ctrl != 0xFE) {\n"
    "            if (out_pair) {\n"
    "                out_pair->key = iter->map->slots[iter->idx].key;\n"
    "                out_pair->value = &iter->map->slots[iter->idx].val;\n"
    "            }\n"
    "            iter->idx++;\n"
    "            return true;\n"
    "        }\n"
    "        iter->idx++;\n"
    "    }\n"
    "    return false;\n"
    "}\n";

int main(int argc, char **argv) {
    cgen_app_dual_def_t app = {
        .subcommand_name = "map_iter",
        .template_h      = MAP_ITER_TEMPLATE_H,
        .template_c      = MAP_ITER_TEMPLATE_C
    };
    return cgen_app_run_dual(&app, argc - 1, argv + 1);
}
