#include "printlib.h"
#include "caslib.h"
#include <stdio.h>
#include <ctype.h>

// Helper function to check if file has addresses (binary or basic)
static bool has_addresses(const cas_File *file) {
    return !file->is_custom && (isBinaryFile(file->file_header.file_type) || 
                                 isBasicFile(file->file_header.file_type));
}

void printHexDump(const uint8_t *data, size_t size, size_t base_offset) {
    for (size_t i = 0; i < size; i += 16) {
        // Print offset
        printf("    %08zx  ", base_offset + i);
        
        // Print hex bytes (first 8)
        for (size_t j = 0; j < 8; j++) {
            if (i + j < size) {
                printf("%02x ", data[i + j]);
            } else {
                printf("   ");
            }
        }
        printf(" ");
        
        // Print hex bytes (second 8)
        for (size_t j = 8; j < 16; j++) {
            if (i + j < size) {
                printf("%02x ", data[i + j]);
            } else {
                printf("   ");
            }
        }
        
        // Print ASCII representation
        printf(" |");
        for (size_t j = 0; j < 16 && i + j < size; j++) {
            unsigned char c = data[i + j];
            if (isprint(c)) {
                printf("%c", c);
            } else {
                printf(".");
            }
        }
        printf("|\n");
    }
}

void printCasHeader(const cas_Header *header) {
    printf("  CAS Header: ");
    for (size_t i = 0; i < sizeof(header->bytes); i++) {
        printf("%02X ", header->bytes[i]);
    }
    printf("\n");
}

void printFileHeader(const cas_FileHeader *file_header) {
    printf("  FileID: ");
    for (size_t i = 0; i < sizeof(file_header->file_type); i++) {
        printf("%02X ", file_header->file_type[i]);
    }
    printf("\n");

    printf("  File Name: ");
    for (size_t i = 0; i < sizeof(file_header->file_name); i++) {
        unsigned char c = file_header->file_name[i];
        if (isprint(c)) {
            printf("%c", c);
        } else {
            printf(".");
        }
    }
    printf("\n");
}

void printDataBlockHeader(const cas_DataBlockHeader *data_block_header) {
    printf("  Data Block Header:\n");
    printf("    Load Address: 0x%04X\n", data_block_header->load_address);
    printf("    End Address:  0x%04X\n", data_block_header->end_address);
    printf("    Exec Address: 0x%04X\n", data_block_header->exec_address);
}

void printDataBlock(const cas_DataBlock *data_block, size_t block_num) {
    printf("  Data Block #%zu:\n", block_num);
    printf("    Data Size:    %zu bytes\n", data_block->data_size);
    printf("    Padding Size: %zu bytes\n", data_block->padding_size);
    
    if (data_block->data_size > 0) {
        printf("\n");
        printf("    Data:\n");
        printHexDump(data_block->data, data_block->data_size, data_block->data_offset);
    }
    
    if (data_block->padding_size > 0) {
        printf("\n");
        printf("    Padding:\n");
        printHexDump(data_block->padding, data_block->padding_size, data_block->padding_offset);
    }
}

void printFile(const cas_File *file, size_t file_num) {
    printf("File #%zu:\n", file_num);
    printf("  Type: %s\n", getFileTypeString(file));
    
    if (!file->is_custom) {
        printFileHeader(&file->file_header);
        
        // Only print data block header for binary/basic files
        if (has_addresses(file)) {
            printDataBlockHeader(&file->data_block_header);
        }
    }
    
    printf("  Total Data Size: %zu bytes\n", file->data_size);
    printf("  Data Block Count: %zu\n", file->data_block_count);
    
    for (size_t i = 0; i < file->data_block_count; i++) {
        printDataBlock(&file->data_blocks[i], i + 1);
    }
}

void printDetailedContainer(const cas_Container *container) {
    printf("CAS Container:\n");
    printf("  Total Files: %zu\n", container->file_count);
    
    for (size_t i = 0; i < container->file_count; i++) {
        printf("\n");
        printFile(&container->files[i], i + 1);
    }
    printf("\n");
}

void printCompactContainer(const cas_Container *container) {
    for (size_t i = 0; i < container->file_count; i++) {
        const cas_File *file = &container->files[i];
        
        printf("%2zu. | %-6s | ", i + 1, getFileTypeString(file));
        if (!file->is_custom) {
            printf("%-6.6s | ", (char*)file->file_header.file_name);
        } else {
            printf("       | ");
        }
        
        printf("%6zu bytes", file->data_size);
        
        // Print addresses for binary/basic files
        if (has_addresses(file)) {
            printf(" | [0x%04x,0x%04x]:0x%04x",
                   file->data_block_header.load_address,
                   file->data_block_header.end_address,
                   file->data_block_header.exec_address);
        } else {
            printf(" |");
        }
        printf("\n");
    }
}
