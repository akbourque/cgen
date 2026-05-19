#pragma once
#ifndef CGEN_PARSER_H
#define CGEN_PARSER_H

#include "libpstr.h"
#include "opt.h"

typedef enum {
    CGEN_PARSE_NONE = 0, // End of arguments reached
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
    pstr_t *msg; // Allocated descriptive error string
} cgen_parse_error_t;

typedef struct {
    cgen_parse_kind_t kind;
    union {
        pstr_t *non_option_arg; // Owned string if kind == CGEN_PARSE_NON_OPTION_ARG
        struct {
            cgen_opt_t opt;
            pstr_t *arg;        // Owned argument string (or NULL if none provided)
        } option;
        cgen_parse_error_t error;
    } as;
} cgen_parse_result_t;

typedef struct {
    pstr_list_t args;    // Slice list containing command line argument inputs
    size_t arg_idx;      // Iteration state tracking next unconsumed argument
    size_t short_idx;    // Byte offset inside an active clustered short-flag string
    pstr_t *current_arg; // Active short-flag string being systematically drained
    cgen_opt_t *opts;    // System configuration options pointer array
    size_t opts_len;     // Array configuration count boundaries
} cgen_parser_t;

void                cgen_parser_init(cgen_parser_t *p, int argc, char **argv, cgen_opt_t *opts, size_t opts_len);
void                cgen_parser_cleanup(cgen_parser_t *p);
cgen_parse_result_t cgen_parser_next(cgen_parser_t *p);
void                cgen_parse_result_free(cgen_parse_result_t res);

#endif // CGEN_PARSER_H
