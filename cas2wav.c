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

    printf("=== CAS Container: %zu file(s) ===\n\n", container.file_count);

    for (size_t i = 0; i < container.file_count; i++) {
        cas_File *file = &container.files[i];

        printf("FILE %zu\n", i + 1);
        printf("======\n\n");

        // Print main file header (cas_Header)
        printf("File Header (CAS_HEADER):\n");
        printf("  Bytes: ");
        for (int j = 0; j < 8; j++) {
            printf("%02x ", file->header.bytes[j]);
        }
        printf("\n\n");

        // Print file header (if not custom)
        if (!file->is_custom) {
            printf("File Type Header:\n");
            printf("  File type bytes: ");
            for (int j = 0; j < 10; j++) {
                printf("%02x ", file->file_header.file_type[j]);
            }
            printf("\n  File type: ");
            if (isAsciiFile(file->file_header.file_type)) {
                printf("ASCII\n");
            } else if (isBinaryFile(file->file_header.file_type)) {
                printf("Binary\n");
            } else if (isBasicFile(file->file_header.file_type)) {
                printf("BASIC\n");
            } else {
                printf("Unknown\n");
            }

            printf("  File name bytes: ");
            for (int j = 0; j < 6; j++) {
                printf("%02x ", file->file_header.file_name[j]);
            }
            printf("\n  File name: %.6s\n\n", file->file_header.file_name);

            // Print data block header for binary/basic files
            if (isBinaryFile(file->file_header.file_type) || isBasicFile(file->file_header.file_type)) {
                printf("Data Block Header:\n");
                printf("  Load address: 0x%04x\n", file->data_block_header.load_address);
                printf("  End address:  0x%04x\n", file->data_block_header.end_address);
                printf("  Exec address: 0x%04x\n\n", file->data_block_header.exec_address);
            }
        } else {
            printf("Type: Custom/Data block (no file header)\n\n");
        }

        printf("Data Information:\n");
        printf("  Total data size: %zu bytes\n", file->data_size);
        printf("  Data block count: %zu\n\n", file->data_block_count);

        // Print each data block
        for (size_t j = 0; j < file->data_block_count; j++) {
            cas_DataBlock *block = &file->data_blocks[j];

            printf("  Data Block %zu:\n", j + 1);
            printf("  -----------\n");

            // Print block CAS header
            printf("    CAS Header: ");
            for (int k = 0; k < 8; k++) {
                printf("%02x ", block->header.bytes[k]);
            }
            printf("\n");

            // Get data and padding size from block
            size_t block_data_size = block->data_size;
            size_t padding_size = block->padding_size;
            size_t total_block_size = block_data_size + padding_size;

            printf("    Data size: %zu bytes\n", block_data_size);
            printf("    Padding size: %zu bytes\n", padding_size);
            printf("    Total block size: %zu bytes\n", total_block_size);
            printf("    Data bytes:\n");

            // Print data in rows of 16 bytes
            if (block->data) {
                for (size_t k = 0; k < block_data_size; k++) {
                    if (k % 16 == 0) {
                        printf("      %04zx: ", k);
                    }
                    printf("%02x ", block->data[k]);
                    if ((k + 1) % 16 == 0 || k + 1 == block_data_size) {
                        printf("\n");
                    }
                }
            }

            // Print padding if present
            if (block->padding && padding_size > 0) {
                printf("    Padding bytes:\n");
                for (size_t k = 0; k < padding_size; k++) {
                    if (k % 16 == 0) {
                        printf("      %04zx: ", k);
                    }
                    printf("%02x ", block->padding[k]);
                    if ((k + 1) % 16 == 0 || k + 1 == padding_size) {
                        printf("\n");
                    }
                }
            }

            printf("\n");
        }

        printf("\n");
    }

    free(container.files);
    free(data);
    return 0;
}