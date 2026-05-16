#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "vendor/libpstr.h"

// --- The Documented SwissTable SWAR Header Template ---
const char *MAP_TEMPLATE_H =
    "#pragma once\n"
    "#ifndef MAP_{{KEY_U}}_{{VAL_U}}_H\n"
    "#define MAP_{{KEY_U}}_{{VAL_U}}_H\n\n"
    "#include <stddef.h>\n"
    "#include <stdint.h>\n"
    "#include <stdbool.h>\n\n"
    "/**\n"
    " * @brief Individual data slot containing the key-value pair payload.\n"
    " */\n"
    "typedef struct {\n"
    "    {{KEY}} key; /**< The unique identifier key associated with this slot. */\n"
    "    {{VAL}} val; /**< The value mapped to the corresponding key. */\n"
    "} map_entry_{{KEY}}_{{VAL}}_t;\n\n"
    "/**\n"
    " * @brief Cache-conscious open-addressed hash map inspired by Google SwissTable.\n"
    " * @details Uses an 8-bucket parallel control byte array to achieve SIMD-like lookup performance.\n"
    " */\n"
    "typedef struct {\n"
    "    uint8_t *ctrl;                      /**< 1 byte per bucket tracking metadata state (Empty, Deleted, or Hash Signature). */\n"
    "    map_entry_{{KEY}}_{{VAL}}_t *slots; /**< Contiguous payload data storage array matching the control byte layout. */\n"
    "    size_t len;                         /**< Total number of active elements currently inside the map. */\n"
    "    size_t cap;                         /**< Total bucket capacity boundary (Must always be a power of 2). */\n"
    "    size_t growth_limit;                /**< Maximum load factor threshold (75%) before triggers a realloc resize. */\n"
    "} map_{{KEY}}_{{VAL}}_t;\n\n"
    "/**\n"
    " * @brief Initializes an empty hash map instance.\n"
    " * @param map Pointer to the target hash map structure.\n"
    " */\n"
    "void map_{{KEY}}_{{VAL}}_init(map_{{KEY}}_{{VAL}}_t *map);\n\n"
    "/**\n"
    " * @brief Frees all dynamically allocated memory held by the hash map (metadata and slots).\n"
    " * @param map Pointer to the target hash map structure.\n"
    " */\n"
    "void map_{{KEY}}_{{VAL}}_free(map_{{KEY}}_{{VAL}}_t *map);\n\n"
    "/**\n"
    " * @brief Inserts a key-value pair into the hash map or updates an existing key's value.\n"
    " * @param map Pointer to the target hash map structure.\n"
    " * @param key The unique key to serve as the lookup query target.\n"
    " * @param val The value to associate with the corresponding key parameter.\n"
    " * @return true if a brand new key entry was created, false if an existing entry was updated in-place.\n"
    " */\n"
    "bool map_{{KEY}}_{{VAL}}_insert(map_{{KEY}}_{{VAL}}_t *map, {{KEY}} key, {{VAL}} val);\n\n"
    "/**\n"
    " * @brief Searches for a key inside the hash map and obtains a pointer to its mapped value.\n"
    " * @note Returns a pointer instead of a value to allow in-place value mutations by reference.\n"
    " * @param map Pointer to the target hash map structure.\n"
    " * @param key The unique key structure to search for inside the container arrays.\n"
    " * @return A direct mutable pointer to the value payload location, or NULL if the key was not found.\n"
    " */\n"
    "{{VAL}} *map_{{KEY}}_{{VAL}}_get(map_{{KEY}}_{{VAL}}_t *map, {{KEY}} key);\n\n"
    "/**\n"
    " * @brief Removes a key-value pair entry from the hash map container.\n"
    " * @details Leaves behind a specialized deletion tombstone flag to prevent breaking lookup search paths.\n"
    " * @param map Pointer to the target hash map structure.\n"
    " * @param key The unique identifier key to eliminate from the active slots.\n"
    " * @return true if the entry was successfully found and removed, false if the key did not exist inside the map.\n"
    " */\n"
    "bool map_{{KEY}}_{{VAL}}_remove(map_{{KEY}}_{{VAL}}_t *map, {{KEY}} key);\n\n"
    "#endif\n";

// --- The SwissTable SWAR Implementation Template ---
const char *MAP_TEMPLATE_C =
    "#include \"map_{{KEY}}_{{VAL}}.h\"\n"
    "#include <stdlib.h>\n"
    "#include <string.h>\n"
    "#include <assert.h>\n\n"
    "// Metadata status bit definitions\n"
    "#define CTRL_EMPTY   0xFF\n"
    "#define CTRL_DELETED 0xFE\n\n"
    "static inline uint64_t map_{{KEY}}_{{VAL}}_hash(const {{KEY}} *key) {\n"
    "    uint64_t hash = 14695981039346656037ULL;\n"
    "    const uint8_t *bytes = (const uint8_t *)key;\n"
    "    for (size_t i = 0; i < sizeof({{KEY}}); i++) {\n"
    "        hash ^= bytes[i];\n"
    "        hash *= 1099511628211ULL;\n"
    "    }\n"
    "    return hash;\n"
    "}\n\n"
    "static void map_{{KEY}}_{{VAL}}_realloc(map_{{KEY}}_{{VAL}}_t *map, size_t new_cap) {\n"
    "    size_t old_cap = map->cap;\n"
    "    uint8_t *old_ctrl = map->ctrl;\n"
    "    map_entry_{{KEY}}_{{VAL}}_t *old_slots = map->slots;\n\n"
    "    map->cap = new_cap;\n"
    "    map->growth_limit = (size_t)(new_cap * 0.75);\n"
    "    map->len = 0;\n"
    "    map->ctrl = (uint8_t *)malloc(new_cap);\n"
    "    map->slots = (map_entry_{{KEY}}_{{VAL}}_t *)malloc(new_cap * sizeof(map_entry_{{KEY}}_{{VAL}}_t));\n"
    "    if (map->ctrl == NULL || map->slots == NULL) abort();\n"
    "    memset(map->ctrl, CTRL_EMPTY, new_cap);\n\n"
    "    for (size_t i = 0; i < old_cap; i++) {\n"
    "        if (old_ctrl[i] != CTRL_EMPTY && old_ctrl[i] != CTRL_DELETED) {\n"
    "            map_{{KEY}}_{{VAL}}_insert(map, old_slots[i].key, old_slots[i].val);\n"
    "        }\n"
    "    }\n"
    "    if (old_ctrl != NULL) free(old_ctrl);\n"
    "    if (old_slots != NULL) free(old_slots);\n"
    "}\n\n"
    "void map_{{KEY}}_{{VAL}}_init(map_{{KEY}}_{{VAL}}_t *map) {\n"
    "    map->ctrl = NULL;\n"
    "    map->slots = NULL;\n"
    "    map->len = 0;\n"
    "    map->cap = 0;\n"
    "    map->growth_limit = 0;\n"
    "}\n\n"
    "void map_{{KEY}}_{{VAL}}_free(map_{{KEY}}_{{VAL}}_t *map) {\n"
    "    if (map->ctrl != NULL) free(map->ctrl);\n"
    "    if (map->slots != NULL) free(map->slots);\n"
    "    map->ctrl = NULL;\n"
    "    map->slots = NULL;\n"
    "    map->len = 0;\n"
    "    map->cap = 0;\n"
    "    map->growth_limit = 0;\n"
    "}\n\n"
    "// SWAR Bit-Scanner: Evaluates 8 control bytes simultaneously via standard 64-bit integer registers\n"
    "static inline uint64_t next_match_mask(const uint8_t *ctrl, size_t offset, uint8_t h2) {\n"
    "    uint64_t block;\n"
    "    memcpy(&block, ctrl + offset, 8);\n"
    "    uint64_t target = h2 * 0x0101010101010101ULL;\n"
    "    uint64_t xor_res = block ^ target;\n"
    "    return (xor_res - 0x0101010101010101ULL) & ~xor_res & 0x8080808080808080ULL;\n"
    "}\n\n"
    "{{VAL}} *map_{{KEY}}_{{VAL}}_get(map_{{KEY}}_{{VAL}}_t *map, {{KEY}} key) {\n"
    "    if (map->len == 0) return NULL;\n"
    "    uint64_t hash = map_{{KEY}}_{{VAL}}_hash(&key);\n"
    "    size_t hash_idx = (size_t)(hash & (map->cap - 1));\n"
    "    uint8_t h2 = (uint8_t)(hash & 0x7F);\n"
    "    size_t mask_mod = map->cap - 1;\n\n"
    "    for (size_t step = 0; step < map->cap; step += 8) {\n"
    "        size_t base = (hash_idx + step) & mask_mod;\n"
    "        if (base + 8 > map->cap) {\n"
    "            // Handle wrap-around boundary elements flatly\n"
    "            for (size_t i = 0; i < 8; i++) {\n"
    "                size_t idx = (base + i) & mask_mod;\n"
    "                if (map->ctrl[idx] == CTRL_EMPTY) return NULL;\n"
    "                if (map->ctrl[idx] == h2) {\n"
    "                    if (memcmp(&map->slots[idx].key, &key, sizeof({{KEY}})) == 0) return &map->slots[idx].val;\n"
    "                }\n"
    "            }\n"
    "            continue;\n"
    "        }\n"
    "        uint64_t match_mask = next_match_mask(map->ctrl, base, h2);\n"
    "        while (match_mask != 0) {\n"
    "            int bit_pos = __builtin_ctzll(match_mask);\n"
    "            size_t idx = base + (bit_pos / 8);\n"
    "            if (memcmp(&map->slots[idx].key, &key, sizeof({{KEY}})) == 0) return &map->slots[idx].val;\n"
    "            match_mask &= match_mask - 1;\n"
    "        }\n"
    "        // Break early if an empty slot is embedded inside the 8-byte block configuration\n"
    "        uint64_t empty_mask = next_match_mask(map->ctrl, base, CTRL_EMPTY);\n"
    "        if (empty_mask != 0) return NULL;\n"
    "    }\n"
    "    return NULL;\n"
    "}\n\n"
    "bool map_{{KEY}}_{{VAL}}_insert(map_{{KEY}}_{{VAL}}_t *map, {{KEY}} key, {{VAL}} val) {\n"
    "    if (map->cap == 0 || map->len >= map->growth_limit) {\n"
    "        size_t new_cap = map->cap == 0 ? 16 : map->cap * 2;\n"
    "        map_{{KEY}}_{{VAL}}_realloc(map, new_cap);\n"
    "    }\n\n"
    "    uint64_t hash = map_{{KEY}}_{{VAL}}_hash(&key);\n"
    "    size_t hash_idx = (size_t)(hash & (map->cap - 1));\n"
    "    uint8_t h2 = (uint8_t)(hash & 0x7F);\n"
    "    size_t mask_mod = map->cap - 1;\n"
    "    size_t target_idx = SIZE_MAX;\n\n"
    "    for (size_t step = 0; step < map->cap; step++) {\n"
    "        size_t idx = (hash_idx + step) & mask_mod;\n"
    "        if (map->ctrl[idx] == h2) {\n"
    "            if (memcmp(&map->slots[idx].key, &key, sizeof({{KEY}})) == 0) {\n"
    "                map->slots[idx].val = val;\n"
    "                return false;\n"
    "            }\n"
    "        }\n"
    "        if (map->ctrl[idx] == CTRL_EMPTY) {\n"
    "            if (target_idx == SIZE_MAX) target_idx = idx;\n"
    "            break;\n"
    "        }\n"
    "        if (map->ctrl[idx] == CTRL_DELETED && target_idx == SIZE_MAX) {\n"
    "            target_idx = idx;\n"
    "        }\n"
    "    }\n\n"
    "    assert(target_idx != SIZE_MAX);\n"
    "    map->ctrl[target_idx] = h2;\n"
    "    map->slots[target_idx].key = key;\n"
    "    map->slots[target_idx].val = val;\n"
    "    map->len++;\n"
    "    return true;\n"
    "}\n\n"
    "bool map_{{KEY}}_{{VAL}}_remove(map_{{KEY}}_{{VAL}}_t *map, {{KEY}} key) {\n"
    "    if (map->len == 0) return false;\n"
    "    uint64_t hash = map_{{KEY}}_{{VAL}}_hash(&key);\n"
    "    size_t hash_idx = (size_t)(hash & (map->cap - 1));\n"
    "    uint8_t h2 = (uint8_t)(hash & 0x7F);\n"
    "    size_t mask_mod = map->cap - 1;\n\n"
    "    for (size_t step = 0; step < map->cap; step++) {\n"
    "        size_t idx = (hash_idx + step) & mask_mod;\n"
    "        if (map->ctrl[idx] == CTRL_EMPTY) return false;\n"
    "        if (map->ctrl[idx] == h2) {\n"
    "            if (memcmp(&map->slots[idx].key, &key, sizeof({{KEY}})) == 0) {\n"
    "                map->ctrl[idx] = CTRL_DELETED;\n"
    "                map->len--;\n"
    "                return true;\n"
    "            }\n"
    "        }\n"
    "    }\n"
    "    return false;\n"
    "}\n";

static pstr_t *replace_all(const char *src, const char *k, const char *ku, const char *v, const char *vu) {
    pstr_builder_t sb;
    pstr.builder.init(&sb);
    pstr.builder.append_cstr(&sb, src);

    while (1) {
        pstr_slice_t m = pstr.builder.find_cstr(&sb, "{{KEY_U}}");
        if (m.ptr == NULL) break;
        pstr.builder.replace_range(&sb, (size_t)(m.ptr - (char *)sb.vec.data), m.len, ku, strlen(ku));
    }
    while (1) {
        pstr_slice_t m = pstr.builder.find_cstr(&sb, "{{KEY}}");
        if (m.ptr == NULL) break;
        pstr.builder.replace_range(&sb, (size_t)(m.ptr - (char *)sb.vec.data), m.len, k, strlen(k));
    }
    while (1) {
        pstr_slice_t m = pstr.builder.find_cstr(&sb, "{{VAL_U}}");
        if (m.ptr == NULL) break;
        pstr.builder.replace_range(&sb, (size_t)(m.ptr - (char *)sb.vec.data), m.len, vu, strlen(vu));
    }
    while (1) {
        pstr_slice_t m = pstr.builder.find_cstr(&sb, "{{VAL}}");
        if (m.ptr == NULL) break;
        pstr.builder.replace_range(&sb, (size_t)(m.ptr - (char *)sb.vec.data), m.len, v, strlen(v));
    }

    pstr_t *res = pstr_from_cstr((char *)sb.vec.data);
    pstr.builder.cleanup(&sb);
    return res;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: ./cgen-map <key_type> <val_type>\n");
        return 1;
    }
    const char *k = argv[1];
    const char *v = argv[2];

    // Generate screaming uppercase equivalents with clean formatting blocks
    char ku[128], vu[128];
    size_t i;
    
    for (i = 0; k[i]; i++) {
        ku[i] = toupper((unsigned char)k[i]);
    }
    ku[i] = '\0';

    for (i = 0; v[i]; i++) {
        vu[i] = toupper((unsigned char)v[i]);
    }
    vu[i] = '\0';

    pstr_t *out_h = replace_all(MAP_TEMPLATE_H, k, ku, v, vu);
    pstr_t *out_c = replace_all(MAP_TEMPLATE_C, k, ku, v, vu);

    pstr_t *path_h = pstr_format("map_%s_%s.h", k, v);
    pstr_t *path_c = pstr_format("map_%s_%s.c", k, v);

    // Clobber files "the Linux way"
    FILE *fh = fopen(path_h->buf, "w");
    if (fh) { fwrite(out_h->buf, 1, out_h->len, fh); fclose(fh); }
    FILE *fc = fopen(path_c->buf, "w");
    if (fc) { fwrite(out_c->buf, 1, out_c->len, fc); fclose(fc); }

    pstr.free(out_h); pstr.free(out_c);
    pstr.free(path_h); pstr.free(path_c);
    return 0;
}
