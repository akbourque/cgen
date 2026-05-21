#pragma once
#ifndef CGEN_PARSER_H
#define CGEN_PARSER_H

#include "libpstr.h"
#include "opt.h"

typedef enum {
    CGEN_PARSE_NONE = 0, ///< End of argument stream reached.
    CGEN_PARSE_NON_OPTION_ARG,
    CGEN_PARSE_OPTION,
    CGEN_PARSE_ERR
} cgen_parse_kind_t;

typedef enum {
    CGEN_ERR_UNKNOWN_SHORT,
    CGEN_ERR_UNKNOWN_LONG,
    CGEN_ERR_MISSING_ARG,
    CGEN_ERR_INVALID_ARG
} cgen_err_kind_t;

typedef struct {
    cgen_err_kind_t kind;
    libpstr_pstr_t *msg; ///< Allocated context descriptive error string.
} cgen_parse_error_t;

typedef struct {
    cgen_parse_kind_t kind;
    union {
        libpstr_pstr_t *non_option_arg;
        struct {
            cgen_opt_t opt;
            libpstr_pstr_t *arg;        ///< Owned argument payload value block (or NULL).
        } option;
        cgen_parse_error_t error;
    } as;
} cgen_parse_result_t;

typedef struct {
    libpstr_list_t args;    ///< Reusable stack array of incoming system command arguments.
    size_t arg_idx;         ///< Pointer offset tracking next unconsumed segment.
    size_t short_idx;       ///< Local character counter within clustered short flag streams.
    libpstr_pstr_t *current_arg; ///< Active clustered string currently being drained.
    cgen_opt_t *opts;       ///< Registered system schema option constraints pointer array.
    size_t opts_len;        ///< Sizing bounds check threshold for constraint array.
} cgen_parser_t;

void                cgen_parser_init(cgen_parser_t *p, int argc, char **argv, cgen_opt_t *opts, size_t opts_len);
void                cgen_parser_cleanup(cgen_parser_t *p);
cgen_parse_result_t cgen_parser_next(cgen_parser_t *p);
void                cgen_parse_result_free(cgen_parse_result_t res);
bool                cgen_parse_template_macro(libpstr_pstr_t *raw_macro, libpstr_pstr_t **out_key, libpstr_pstr_t **out_val);

#endif // CGEN_PARSER_H
