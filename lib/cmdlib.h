#ifndef CMDLIB_H
#define CMDLIB_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "caslib.h"

// Read an entire file into memory
// Returns pointer to allocated buffer, or NULL on error
// Caller must free the returned buffer
uint8_t* readFileIntoMemory(const char *filename, size_t *out_size);

// Check if a file exists
bool fileExists(const char *filename);

// Create a directory (and verify it's a directory if it exists)
bool createDirectory(const char *path);

// Build a file path from directory and filename
// Returns allocated string that must be freed by caller
char* buildFilePath(const char *dir, const char *filename);

// Write CAS file data to disk
// disk_format: if true, add MSX-DOS prefix (0xFE/0xFF) for BINARY/BASIC files
bool writeFileData(const char *filename, const cas_File *file, bool verbose, bool disk_format);

#endif // CMDLIB_H
