#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/caslib.h"
#include "../lib/printlib.h"
#include "../lib/cmdlib.h"

// Helper function to export a single file
static int export_single_file(const cas_File *file, int index, const char *output_dir, 
                               bool force, bool verbose, bool disk_format) {
    char *filename = generateFilename(file, index);
    if (!filename) {
        fprintf(stderr, "Error: Failed to generate filename for file %d\n", index);
        return 1;
    }
    
    char *filepath = buildFilePath(output_dir, filename);
    if (!filepath) {
        fprintf(stderr, "Error: Failed to build output path for '%s'\n", filename);
        free(filename);
        return 1;
    }
    
    // Check if file exists and force flag
    if (fileExists(filepath) && !force) {
        fprintf(stderr, "Error: File '%s' already exists (use -f to overwrite)\n", filepath);
        free(filepath);
        free(filename);
        return 1;
    }
    
    int result = writeFileData(filepath, file, verbose, disk_format) ? 0 : 1;
    
    free(filepath);
    free(filename);
    return result;
}

int execute_export(const char *input_file, int filter_index, const char *output_dir, bool force, bool verbose, bool disk_format) {
    if (verbose) {
        printf("Reading file: %s\n", input_file);
    }
    
    // Create output directory if specified
    if (output_dir && !createDirectory(output_dir)) {
        return 1;
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
    
    int result = 0;
    
    // If filtering by index, export only that file
    if (filter_index > 0) {
        if ((size_t)filter_index > container.file_count) {
            fprintf(stderr, "Error: Index %d out of range (1-%zu)\n", filter_index, container.file_count);
            result = 1;
        } else {
            result = export_single_file(&container.files[filter_index - 1], filter_index, 
                                        output_dir, force, verbose, disk_format);
        }
    } else {
        // Export all files
        for (size_t i = 0; i < container.file_count && result == 0; i++) {
            result = export_single_file(&container.files[i], i + 1, 
                                        output_dir, force, verbose, disk_format);
        }
    }
    
    // Clean up
    free(container.files);
    free(file_data);
    
    return result;
}
