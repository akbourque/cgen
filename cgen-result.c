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
    
    "result_{{KEY_B}}_{{VAL_B}}_t result_{{KEY_B}}_{{VAL_B}}_ok({{KEY}} val);\n"
    "result_{{KEY_B}}_{{VAL_B}}_t result_{{KEY_B}}_{{VAL_B}}_err({{VAL}} err);\n"
    "bool result_{{KEY_B}}_{{VAL_B}}_is_ok(const result_{{KEY_B}}_{{VAL_B}}_t *res);\n"
    "bool result_{{KEY_B}}_{{VAL_B}}_is_err(const result_{{KEY_B}}_{{VAL_B}}_t *res);\n"
    "{{KEY}} result_{{KEY_B}}_{{VAL_B}}_unwrap(result_{{KEY_B}}_{{VAL_B}}_t res);\n"
    "{{VAL}} result_{{KEY_B}}_{{VAL_B}}_unwrap_err(result_{{KEY_B}}_{{VAL_B}}_t res);\n\n"
    "#endif\n";

const char *RESULT_TEMPLATE_C =
    "#include <stdio.h>\n"
    "#include <stdlib.h>\n"
    "#include \"result_{{KEY_B}}_{{VAL_B}}.h\"\n\n"
    
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
    return cgen_app_run_dual(&app, argc, argv);
}
