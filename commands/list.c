#include <stdio.h>
#include <stdlib.h>

#include "../lib/caslib.h"
#include "../lib/printlib.h"
#include "../lib/cmdlib.h"

int execute_list(const char *input_file, bool extended, bool verbose) {
    if (verbose) {
        printf("Reading file: %s\n", input_file);
    }
    
    // Read the file into memory
    size_t file_size;
    uint8_t *file_data = readFileIntoMemory(input_file, &file_size);
    if (!file_data) {
        fprintf(stderr, "Error: Failed to read file '%s'\n", input_file);
        return 1;
    }
    
    if (verbose) {
        printf("File size: %zu bytes\n", file_size);
        printf("Parsing CAS container...\n");
    }
    
    // Parse the CAS container
    cas_Container container;
    if (!parseCasContainer(file_data, &container, file_size)) {
        fprintf(stderr, "Error: Failed to parse CAS container\n");
        free(file_data);
        return 1;
    }
    
    if (verbose) {
        printf("Successfully parsed %zu file(s)\n\n", container.file_count);
    }
    
    // Print the list
    if (extended) {
        // Print full details with hexdumps
        printDetailedContainer(&container);
    } else {
        // Print compact list
        printCompactContainer(&container);
    }
    
    // Cleanup
    free(container.files);
    free(file_data);
    
    return 0;
}