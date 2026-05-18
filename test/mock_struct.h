#pragma once
#ifndef MOCK_STRUCT_H
#define MOCK_STRUCT_H

typedef struct {
    int id;
    char *heap_string; // Allocates memory that MUST be freed via callback
} custom_t;

#endif // MOCK_STRUCT_H
