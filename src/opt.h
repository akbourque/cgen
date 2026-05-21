#pragma once
#ifndef CGEN_OPT_H
#define CGEN_OPT_H

#include "libpstr.h"

#define CGEN_NO_ARG '-'
#define CGEN_OPTIONAL_ARG '?'
#define CGEN_REQUIRED_ARG '='

typedef struct {
    libpstr_slice_t raw; ///< Points directly to the specification string segment.
} cgen_opt_t;

cgen_opt_t      cgen_opt_new(libpstr_slice_t attr);
char            cgen_opt_arg_type(cgen_opt_t opt);
char            cgen_opt_short_name(cgen_opt_t opt);
libpstr_slice_t cgen_opt_long_name(cgen_opt_t opt);
bool            cgen_opt_check(libpstr_slice_t attr, libpstr_pstr_t **out_err_msg);

#endif // CGEN_OPT_H
