#include <stdio.h>
#include <stdlib.h>
#include "lib/caslib.h"
#include "lib/printlib.h"
#include "lib/cmdlib.h"

int main(int argc, char *argv[]) {
    // Check command-line arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <cas-file>\n", argv[0]);
        return 1;
    }

    // Read CAS file into memory
    size_t file_size;
    uint8_t *data = readFileIntoMemory(argv[1], &file_size);
    if (!data) {
        fprintf(stderr, "Failed to read file: %s\n", argv[1]);
        return 1;
    }

    // Parse the CAS container
    cas_Container container;
    if (!parseCasContainer(data, &container, file_size)) {
        fprintf(stderr, "Failed to parse CAS file\n");
        free(data);
        return 1;
    }

    // Print the entire container
    printContainer(&container);

    // Cleanup
    free(container.files);
    free(data);
    return 0;
}