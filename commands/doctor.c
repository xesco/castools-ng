#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/caslib.h"
#include "../lib/printlib.h"
#include "../lib/cmdlib.h"

// Check a single file for disk markers
static int check_file_markers(const cas_File *file, int index) {
    int issues_found = 0;
    
    // Check BINARY files for 0xFE start marker (disk BSAVE format)
    if (isBinaryFile(file->file_header.file_type)) {
        // Defensive check - should not happen with valid parsing
        if (file->data_block_count == 0) {
            return 0;
        }
        
        // BINARY files only have one data block
        const cas_DataBlock *block = &file->data_blocks[0];
        
        if (!block->data || block->data_size == 0) {
            return 0;
        }
        
        // Check for 0xFE marker at the start (disk BSAVE format indicator)
        if (block->data[0] == 0xFE) {
            printf("Warning: File %d (BINARY) contains 0xFE disk marker at start (offset 0x%08zx):\n", 
                   index, block->data_offset);
            size_t show_len = (block->data_size < 16) ? block->data_size : 16;
            printHexDump(block->data, show_len, block->data_offset);
            issues_found++;
        }
    }
    
    // Check BASIC files for 0xFF start marker (disk tokenized BASIC format)
    if (isBasicFile(file->file_header.file_type)) {
        // Defensive check
        if (file->data_block_count == 0) {
            return 0;
        }
        
        // BASIC files only have one data block
        const cas_DataBlock *block = &file->data_blocks[0];
        
        // Check for 0xFF marker at the start of data
        if (block->data && block->data_size > 0) {
            if (block->data[0] == 0xFF) {
                printf("Warning: File %d (BASIC) contains 0xFF disk marker at start (offset 0x%08zx):\n", 
                       index, block->data_offset);
                size_t show_len = (block->data_size < 16) ? block->data_size : 16;
                printHexDump(block->data, show_len, block->data_offset);
                issues_found++;
            }
        }
    }
    
    return issues_found;
}

int execute_doctor(const char *input_file, bool check_disk_markers, bool verbose) {
    if (verbose) {
        printf("Reading file: %s\n", input_file);
        if (check_disk_markers) {
            printf("Checking for disk format markers\n");
        }
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
        printf("Successfully parsed %zu file(s)\n", container.file_count);
    }
    
    int total_issues = 0;
    
    // Perform enabled checks
    if (check_disk_markers) {
        if (verbose) {
            printf("Checking for disk format markers...\n\n");
        }
        
        // Check each file
        for (size_t i = 0; i < container.file_count; i++) {
            int issues = check_file_markers(&container.files[i], i + 1);
            total_issues += issues;
        }
        
        if (total_issues == 0) {
            printf("✓ No disk format markers found\n");
        } else {
            printf("\nFound %d issue(s) with disk format markers\n", total_issues);
        }
    }
    
    // Clean up
    free(container.files);
    free(file_data);
    
    return total_issues > 0 ? 1 : 0;
}
