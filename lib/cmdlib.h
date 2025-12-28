#ifndef CMDLIB_H
#define CMDLIB_H

#include <stddef.h>
#include <stdint.h>

// Read an entire file into memory
// Returns pointer to allocated buffer, or NULL on error
// Caller must free the returned buffer
uint8_t* readFileIntoMemory(const char *filename, size_t *out_size);

#endif // CMDLIB_H
