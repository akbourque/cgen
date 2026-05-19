#include "sample_t.h"
#include <string.h>

/* --- Constructors Implementation --- */
sample_t sample_new_high(sample_high_t val) {
    sample_t v;
    v.tag = SAMPLE_HIGH;
    v.data.as_high = val;
    return v;
}

sample_t sample_new_low(sample_low_t val) {
    sample_t v;
    v.tag = SAMPLE_LOW;
    v.data.as_low = val;
    return v;
}

sample_t sample_new_text(sample_text_t val) {
    sample_t v;
    v.tag = SAMPLE_TEXT;
    v.data.as_text = val;
    return v;
}

/* --- Inspectors Implementation --- */
bool sample_is_high(const sample_t* v) {
    return v->tag == SAMPLE_HIGH;
}

bool sample_is_low(const sample_t* v) {
    return v->tag == SAMPLE_LOW;
}

bool sample_is_text(const sample_t* v) {
    return v->tag == SAMPLE_TEXT;
}

/* --- Extractors Implementation --- */
sample_high_t sample_unwrap_high(const sample_t* v) {
    if (v->tag != SAMPLE_HIGH) {
        PANIC("Catastrophic: Variant unwrapping type mismatch violation!");
    }
    return v->data.as_high;
}

sample_low_t sample_unwrap_low(const sample_t* v) {
    if (v->tag != SAMPLE_LOW) {
        PANIC("Catastrophic: Variant unwrapping type mismatch violation!");
    }
    return v->data.as_low;
}

sample_text_t sample_unwrap_text(const sample_t* v) {
    if (v->tag != SAMPLE_TEXT) {
        PANIC("Catastrophic: Variant unwrapping type mismatch violation!");
    }
    return v->data.as_text;
}

