#include "cmdlib.h"
#include <stdio.h>
#include <stdlib.h>

uint8_t* readFileIntoMemory(const char *filename, size_t *out_size) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Error opening file");
        return NULL;
    }

    // Get file size
    if (fseek(fp, 0, SEEK_END) != 0) {
        perror("Error seeking to end of file");
        fclose(fp);
        return NULL;
    }

    long file_size_long = ftell(fp);
    if (file_size_long < 0) {
        perror("Error getting file size");
        fclose(fp);
        return NULL;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        perror("Error seeking to start of file");
        fclose(fp);
        return NULL;
    }

    size_t file_size = (size_t)file_size_long;

    // Allocate buffer
    uint8_t *data = malloc(file_size);
    if (!data) {
        perror("Memory allocation failed");
        fclose(fp);
        return NULL;
    }

    // Read entire file
    size_t bytes_read = fread(data, 1, file_size, fp);
    fclose(fp);

    if (bytes_read != file_size) {
        fprintf(stderr, "Error: Could not read entire file (expected %zu bytes, got %zu)\n", 
                file_size, bytes_read);
        free(data);
        return NULL;
    }

    *out_size = file_size;
    return data;
}
