#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_CHOICES 64
#define MAX_NAME_LEN 128

typedef struct {
    char field_name[MAX_NAME_LEN];      // e.g., "high"
    char underlying_type[MAX_NAME_LEN];  // e.g., "int"
    char upper_name[MAX_NAME_LEN];      // e.g., "HIGH"
} variant_choice_t;

// Helper to convert a string to uppercase safely
void make_uppercase(const char* src, char* dest, size_t dest_len) {
    size_t i = 0;
    while (src[i] && i < dest_len - 1) {
        dest[i] = toupper((unsigned char)src[i]);
        i++;
    }
    dest[i] = '\0';
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <variant_name> <field:type> [field2:type2 ...]\n", argv[0]);
        fprintf(stderr, "Example: %s choice_t high:int low:int pstr_sb_t\n", argv[0]);
        return 1;
    }

    const char* variant_name = argv[1];
    char variant_upper[MAX_NAME_LEN];
    make_uppercase(variant_name, variant_upper, MAX_NAME_LEN);
    
    // Drop the trailing '_t' for clean enum/function prefix naming conventions if present
    char prefix[MAX_NAME_LEN];
    strncpy(prefix, variant_name, MAX_NAME_LEN - 1);
    prefix[MAX_NAME_LEN - 1] = '\0';
    size_t len = strlen(prefix);
    if (len > 2 && strcmp(&prefix[len - 2], "_t") == 0) {
        prefix[len - 2] = '\0';
    }
    char prefix_upper[MAX_NAME_LEN];
    make_uppercase(prefix, prefix_upper, MAX_NAME_LEN);

    variant_choice_t choices[MAX_CHOICES];
    int choice_count = 0;

// 1. Parse Command Line Arguments using Pair Syntax Loop
    for (int i = 2; i < argc && choice_count < MAX_CHOICES; i++) {
        char arg_copy[MAX_NAME_LEN];
        // Safely copy the argument into our workspace with guaranteed null-termination
        snprintf(arg_copy, sizeof(arg_copy), "%s", argv[i]);

        char* colon = strchr(arg_copy, ':');
        if (colon) {
            *colon = '\0';
            snprintf(choices[choice_count].field_name, MAX_NAME_LEN, "%s", arg_copy);
            snprintf(choices[choice_count].underlying_type, MAX_NAME_LEN, "%s", colon + 1);
        } else {
            // Fallback Backwards Compatibility: Use type name as field name if no colon
            snprintf(choices[choice_count].field_name, MAX_NAME_LEN, "%s", arg_copy);
            snprintf(choices[choice_count].underlying_type, MAX_NAME_LEN, "%s", arg_copy);
        }
        make_uppercase(choices[choice_count].field_name, choices[choice_count].upper_name, MAX_NAME_LEN);
        choice_count++;
    }

    // 2. Open Files for Streaming
    char h_filename[MAX_NAME_LEN + 8];
    char c_filename[MAX_NAME_LEN + 8];
    snprintf(h_filename, sizeof(h_filename), "%s.h", variant_name);
    snprintf(c_filename, sizeof(c_filename), "%s.c", variant_name);

    FILE* h_file = fopen(h_filename, "w");
    FILE* c_file = fopen(c_filename, "w");

    if (!h_file || !c_file) {
        fprintf(stderr, "Error: Could not open output streams for generation.\n");
        if (h_file) fclose(h_file);
        if (c_file) fclose(c_file);
        return 1;
    }

    // ========================================================================
    // GENERATE THE HEADER FILE (.h)
    // ========================================================================
    fprintf(h_file, "#pragma once\n");
    fprintf(h_file, "#include <stddef.h>\n");
    fprintf(h_file, "#include <stdbool.h>\n");
    fprintf(h_file, "#include \"libpstr.h\"\n");
    fprintf(h_file, "#include \"panic.h\"\n\n");

    fprintf(h_file, "/* --- Auto-Generated Typedef Aliases --- */\n");
    for (int i = 0; i < choice_count; i++) {
        fprintf(h_file, "typedef %s %s_%s_t;\n", 
                choices[i].underlying_type, prefix, choices[i].field_name);
    }
    fprintf(h_file, "\n");

    fprintf(h_file, "typedef enum {\n");
    fprintf(h_file, "    %s_EMPTY,\n", prefix_upper);
    for (int i = 0; i < choice_count; i++) {
        fprintf(h_file, "    %s_%s,\n", prefix_upper, choices[i].upper_name);
    }
    fprintf(h_file, "} %s_tag_t;\n\n", prefix);

    fprintf(h_file, "typedef struct {\n");
    fprintf(h_file, "    %s_tag_t tag;\n", prefix);
    fprintf(h_file, "    union {\n");
    for (int i = 0; i < choice_count; i++) {
        fprintf(h_file, "        %s_%s_t as_%s;\n", 
                prefix, choices[i].field_name, choices[i].field_name);
    }
    fprintf(h_file, "    } data;\n");
    fprintf(h_file, "} %s;\n\n", variant_name);

    fprintf(h_file, "/* --- Safe Constructors --- */\n");
    for (int i = 0; i < choice_count; i++) {
        fprintf(h_file, "%s %s_new_%s(%s_%s_t val);\n", 
                variant_name, prefix, choices[i].field_name, prefix, choices[i].field_name);
    }
    fprintf(h_file, "\n");

    fprintf(h_file, "/* --- Type Inspectors --- */\n");
    for (int i = 0; i < choice_count; i++) {
        fprintf(h_file, "bool %s_is_%s(const %s* v);\n", 
                prefix, choices[i].field_name, variant_name);
    }
    fprintf(h_file, "\n");

    fprintf(h_file, "/* --- Value Extractors --- */\n");
    for (int i = 0; i < choice_count; i++) {
        fprintf(h_file, "%s_%s_t %s_unwrap_%s(const %s* v);\n", 
                prefix, choices[i].field_name, prefix, choices[i].field_name, variant_name);
    }

    // ========================================================================
    // GENERATE THE IMPLEMENTATION FILE (.c)
    // ========================================================================
    fprintf(c_file, "#include \"%s\"\n", h_filename);
    fprintf(c_file, "#include <string.h>\n\n");

    fprintf(c_file, "/* --- Constructors Implementation --- */\n");
    for (int i = 0; i < choice_count; i++) {
        fprintf(c_file, "%s %s_new_%s(%s_%s_t val) {\n", 
                variant_name, prefix, choices[i].field_name, prefix, choices[i].field_name);
        fprintf(c_file, "    %s v;\n", variant_name);
        fprintf(c_file, "    v.tag = %s_%s;\n", prefix_upper, choices[i].upper_name);
        fprintf(c_file, "    v.data.as_%s = val;\n", choices[i].field_name);
        fprintf(c_file, "    return v;\n");
        fprintf(c_file, "}\n\n");
    }

    fprintf(c_file, "/* --- Inspectors Implementation --- */\n");
    for (int i = 0; i < choice_count; i++) {
        fprintf(c_file, "bool %s_is_%s(const %s* v) {\n", 
                prefix, choices[i].field_name, variant_name);
        fprintf(c_file, "    return v->tag == %s_%s;\n", prefix_upper, choices[i].upper_name);
        fprintf(c_file, "}\n\n");
    }

    fprintf(c_file, "/* --- Extractors Implementation --- */\n");
    for (int i = 0; i < choice_count; i++) {
        fprintf(c_file, "%s_%s_t %s_unwrap_%s(const %s* v) {\n", 
                prefix, choices[i].field_name, prefix, choices[i].field_name, variant_name);
        fprintf(c_file, "    if (v->tag != %s_%s) {\n", prefix_upper, choices[i].upper_name);
        fprintf(c_file, "        PANIC(\"Catastrophic: Variant unwrapping type mismatch violation!\");\n");
        fprintf(c_file, "    }\n");
        fprintf(c_file, "    return v->data.as_%s;\n", choices[i].field_name);
        fprintf(c_file, "}\n\n");
    }

    fclose(h_file);
    fclose(c_file);
    
    printf("✨ Success! Auto-generated files: %s and %s\n", h_filename, c_filename);
    return 0;
}
