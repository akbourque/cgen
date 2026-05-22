#include "cgen_framework.h"

// ============================================================================
// TEMPLATE STRINGS
// ============================================================================
const char *RESULT_TEMPLATE_H =
    "#pragma once\n"
    "#ifndef RESULT_{{KEY_BU}}_{{VAL_BU}}_H\n"
    "#define RESULT_{{KEY_BU}}_{{VAL_BU}}_H\n\n"
    "#include <stdbool.h>\n"
    "#include <stddef.h>\n\n"
    "/**\n"
    " * @brief Discriminator tag indicating the success or failure state of a Result.\n"
    " */\n"
    "typedef enum {\n"
    "    RESULT_{{KEY_BU}}_{{VAL_BU}}_OK = 0,  /**< Indicates operation success. */\n"
    "    RESULT_{{KEY_BU}}_{{VAL_BU}}_ERR = 1  /**< Indicates operation failure. */\n"
    "} result_{{KEY_B}}_{{VAL_B}}_tag_t;\n\n"
    
    "/**\n"
    " * @brief A type-safe variant containing either a successful value or an error payload.\n"
    " */\n"
    "typedef struct {\n"
    "    result_{{KEY_B}}_{{VAL_B}}_tag_t tag; /**< Active state discriminator. */\n"
    "    union {\n"
    "        {{KEY}} ok;                       /**< Successful value payload. */\n"
    "        {{VAL}} err;                      /**< Error variant payload. */\n"
    "    } as;\n"
    "} result_{{KEY_B}}_{{VAL_B}}_t;\n\n"
    
    "/* --- Constructors --- */\n\n"
    "/**\n"
    " * @brief Constructs a successful result containing a {{KEY}} value.\n"
    " * @param val The {{KEY}} value to wrap.\n"
    " * @return A result_{{KEY_B}}_{{VAL_B}}_t in the OK state.\n"
    " */\n"
    "result_{{KEY_B}}_{{VAL_B}}_t result_{{KEY_B}}_{{VAL_B}}_ok({{KEY}} val);\n\n"
    
    "/**\n"
    " * @brief Constructs an error result containing a {{VAL}} payload.\n"
    " * @param err The {{VAL}} error payload to wrap.\n"
    " * @return A result_{{KEY_B}}_{{VAL_B}}_t in the ERR state.\n"
    " */\n"
    "result_{{KEY_B}}_{{VAL_B}}_t result_{{KEY_B}}_{{VAL_B}}_err({{VAL}} err);\n\n"
    
    "/* --- Inspectors --- */\n\n"
    "/**\n"
    " * @brief Checks if the result is in the OK (success) state.\n"
    " * @param res Pointer to the result to inspect.\n"
    " * @return true if the result contains an OK value, false otherwise.\n"
    " */\n"
    "bool result_{{KEY_B}}_{{VAL_B}}_is_ok(const result_{{KEY_B}}_{{VAL_B}}_t *res);\n\n"
    
    "/**\n"
    " * @brief Checks if the result is in the ERR (failure) state.\n"
    " * @param res Pointer to the result to inspect.\n"
    " * @return true if the result contains an ERR value, false otherwise.\n"
    " */\n"
    "bool result_{{KEY_B}}_{{VAL_B}}_is_err(const result_{{KEY_B}}_{{VAL_B}}_t *res);\n\n"
    
    "/* --- Extractors --- */\n\n"
    "/**\n"
    " * @brief Unwraps the result, extracting the successful {{KEY}} value.\n"
    " * @note This function will trigger a PANIC if the result is in the ERR state.\n"
    " * @param res The result to unwrap.\n"
    " * @return The underlying {{KEY}} value.\n"
    " */\n"
    "{{KEY}} result_{{KEY_B}}_{{VAL_B}}_unwrap(result_{{KEY_B}}_{{VAL_B}}_t res);\n\n"
    
    "/**\n"
    " * @brief Unwraps the result, extracting the error {{VAL}} value.\n"
    " * @note This function will trigger a PANIC if the result is in the OK state.\n"
    " * @param res The result to unwrap.\n"
    " * @return The underlying {{VAL}} error payload.\n"
    " */\n"
    "{{VAL}} result_{{KEY_B}}_{{VAL_B}}_unwrap_err(result_{{KEY_B}}_{{VAL_B}}_t res);\n\n"
    
    "#endif\n";

const char *RESULT_TEMPLATE_C =
    "#include <stdio.h>\n"
    "#include <stdlib.h>\n"
    "#include \"result_{{KEY}}_{{VAL}}.h\"\n\n"
    
    "result_{{KEY_B}}_{{VAL_B}}_t result_{{KEY_B}}_{{VAL_B}}_ok({{KEY}} val) {\n"
    "    result_{{KEY_B}}_{{VAL_B}}_t res;\n"
    "    res.tag = RESULT_{{KEY_BU}}_{{VAL_BU}}_OK;\n"
    "    res.as.ok = val;\n"
    "    return res;\n"
    "}\n\n"
    
    "result_{{KEY_B}}_{{VAL_B}}_t result_{{KEY_B}}_{{VAL_B}}_err({{VAL}} err) {\n"
    "    result_{{KEY_B}}_{{VAL_B}}_t res;\n"
    "    res.tag = RESULT_{{KEY_BU}}_{{VAL_BU}}_ERR;\n"
    "    res.as.err = err;\n"
    "    return res;\n"
    "}\n\n"
    
    "bool result_{{KEY_B}}_{{VAL_B}}_is_ok(const result_{{KEY_B}}_{{VAL_B}}_t *res) {\n"
    "    if (res == NULL) return false;\n"
    "    return res->tag == RESULT_{{KEY_BU}}_{{VAL_BU}}_OK;\n"
    "}\n\n"
    
    "bool result_{{KEY_B}}_{{VAL_B}}_is_err(const result_{{KEY_B}}_{{VAL_B}}_t *res) {\n"
    "    if (res == NULL) return true;\n"
    "    return res->tag == RESULT_{{KEY_BU}}_{{VAL_BU}}_ERR;\n"
    "}\n\n"
    
    "{{KEY}} result_{{KEY_B}}_{{VAL_B}}_unwrap(result_{{KEY_B}}_{{VAL_B}}_t res) {\n"
    "    if (res.tag == RESULT_{{KEY_BU}}_{{VAL_BU}}_ERR) {\n"
    "        fprintf(stderr, \"panic: called result_{{KEY_B}}_{{VAL_B}}_unwrap() on an ERR value\\n\");\n"
    "        abort();\n"
    "    }\n"
    "    return res.as.ok;\n"
    "}\n\n"
    
    "{{VAL}} result_{{KEY_B}}_{{VAL_B}}_unwrap_err(result_{{KEY_B}}_{{VAL_B}}_t res) {\n"
    "    if (res.tag == RESULT_{{KEY_BU}}_{{VAL_BU}}_OK) {\n"
    "        fprintf(stderr, \"panic: called result_{{KEY_B}}_{{VAL_B}}_unwrap_err() on an OK value\\n\");\n"
    "        abort();\n"
    "    }\n"
    "    return res.as.err;\n"
    "}\n";

int main(int argc, char **argv) {
    cgen_app_dual_def_t app = {
        .subcommand_name = "result",
        .template_h      = RESULT_TEMPLATE_H,
        .template_c      = RESULT_TEMPLATE_C
    };
    // Direct link straight into the shared double-token engine
    return cgen_app_run_dual(&app, argc - 1, argv + 1);
}
