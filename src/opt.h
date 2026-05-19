#pragma once
#ifndef CGEN_OPT_H
#define CGEN_OPT_H

#include "libpstr.h"

#define CGEN_NO_ARG '-'
#define CGEN_OPTIONAL_ARG '?'
#define CGEN_REQUIRED_ARG '='

typedef struct {
    pstr_slice_t raw; // Points directly to the specification string in the header slice
} cgen_opt_t;

// Porting the core logic from opt.rs
cgen_opt_t    cgen_opt_new(pstr_slice_t attr);
char          cgen_opt_arg_type(cgen_opt_t opt);
char          cgen_opt_short_name(cgen_opt_t opt);
pstr_slice_t  cgen_opt_long_name(cgen_opt_t opt);
bool          cgen_opt_check(pstr_slice_t attr, pstr_t **out_err_msg);

#endif // CGEN_OPT_H
