#pragma once
#include <stddef.h>
#include <stdbool.h>
#include "libpstr.h"
#include "panic.h"

/* --- Auto-Generated Typedef Aliases --- */
typedef int sample_high_t;
typedef int sample_low_t;
typedef pstr_builder_t sample_text_t;

typedef enum {
    SAMPLE_EMPTY,
    SAMPLE_HIGH,
    SAMPLE_LOW,
    SAMPLE_TEXT,
} sample_tag_t;

typedef struct {
    sample_tag_t tag;
    union {
        sample_high_t as_high;
        sample_low_t as_low;
        sample_text_t as_text;
    } data;
} sample_t;

/* --- Safe Constructors --- */
sample_t sample_new_high(sample_high_t val);
sample_t sample_new_low(sample_low_t val);
sample_t sample_new_text(sample_text_t val);

/* --- Type Inspectors --- */
bool sample_is_high(const sample_t* v);
bool sample_is_low(const sample_t* v);
bool sample_is_text(const sample_t* v);

/* --- Value Extractors --- */
sample_high_t sample_unwrap_high(const sample_t* v);
sample_low_t sample_unwrap_low(const sample_t* v);
sample_text_t sample_unwrap_text(const sample_t* v);
