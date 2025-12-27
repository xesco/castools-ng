#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "caslib.h"

int main() {
    FILE *fp = fopen("game.cas", "rb");
    if (!fp) {
        perror("Error opening game.cas");
        return 1;
    }

    // Get file size
    fseek(fp, 0, SEEK_END);
    long file_size_long = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    if (file_size_long < 0) {
        perror("Error getting file size");
        fclose(fp);
        return 1;
    }
    
    size_t file_size = (size_t)file_size_long;

    // Read entire file into memory
    uint8_t *data = malloc(file_size);
    if (!data) {
        perror("Memory allocation failed");
        fclose(fp);
        return 1;
    }

    size_t bytes_read = fread(data, 1, file_size, fp);
    fclose(fp);

    if (bytes_read != file_size) {
        fprintf(stderr, "Error reading file\n");
        free(data);
        return 1;
    }

    // Parse the CAS container
    cas_Container container;
    if (!parseCasContainer(data, &container, file_size)) {
        fprintf(stderr, "Failed to parse CAS file\n");
        free(data);
        return 1;
    }

    for (size_t i = 0; i < container.file_count; i++) {
        if (container.files[i].is_custom) {
            printf("custom |        | %5zu bytes | %zu blocks\n", 
                   container.files[i].data_size,
                   container.files[i].data_block_count);
        } else if (isAsciiFile(container.files[i].file_header.file_type)) {
            printf("ascii  | %.6s | %5zu bytes | %zu blocks\n", 
                   container.files[i].file_header.file_name,
                   container.files[i].data_size,
                   container.files[i].data_block_count);
        } else if (isBinaryFile(container.files[i].file_header.file_type)) {
            printf("bin    | %.6s | %5zu bytes | %zu blocks | [0x%04x,0x%04x]:0x%04x\n",
                   container.files[i].file_header.file_name,
                   container.files[i].data_size,
                   container.files[i].data_block_count,
                   container.files[i].data_block_header.load_address,
                   container.files[i].data_block_header.end_address,
                   container.files[i].data_block_header.exec_address);
        } else if (isBasicFile(container.files[i].file_header.file_type)) {
            printf("basic  | %.6s | %5zu bytes | %zu blocks | [0x%04x,0x%04x]:0x%04x\n",
                   container.files[i].file_header.file_name,
                   container.files[i].data_size,
                   container.files[i].data_block_count,
                   container.files[i].data_block_header.load_address,
                   container.files[i].data_block_header.end_address,
                   container.files[i].data_block_header.exec_address);
        }
    }

    free(container.files);
    free(data);
    return 0;
}