#include "cmdlib.h"
#include "caslib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

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

bool fileExists(const char *filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

bool createDirectory(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return true;
        }
        fprintf(stderr, "Error: '%s' exists but is not a directory\n", path);
        return false;
    }
    
    if (mkdir(path, 0755) != 0) {
        fprintf(stderr, "Error: Cannot create directory '%s': %s\n", path, strerror(errno));
        return false;
    }
    
    return true;
}

char* buildFilePath(const char *dir, const char *filename) {
    // No directory specified - return just the filename
    if (!dir) {
        return strdup(filename);
    }
    
    // Calculate total path length: "dir/filename\0"
    size_t path_len = strlen(dir) + 1 + strlen(filename) + 1;
    
    char *path = malloc(path_len);
    if (!path) {
        return NULL;
    }
    
    snprintf(path, path_len, "%s/%s", dir, filename);
    return path;
}

char* generateFilename(const cas_File *file, int index) {
    // Determine extension based on file type
    const char *ext = isAsciiFile(file->file_header.file_type) ? "asc" :
                      isBinaryFile(file->file_header.file_type) ? "bin" :
                      isBasicFile(file->file_header.file_type) ? "bas" : "dat";
    
    char *filename = malloc(256);
    if (!filename) return NULL;
    
    // Custom blocks don't have valid file headers
    if (file->is_custom) {
        snprintf(filename, 256, "%d-custom.%s", index, ext);
        return filename;
    }
    
    // Extract and trim filename from header (6 chars, space-padded at end)
    char base_name[7];
    memcpy(base_name, file->file_header.file_name, 6);
    base_name[6] = '\0';
    
    // Trim trailing spaces
    char *end = base_name + 5;
    while (end >= base_name && *end == ' ') *end-- = '\0';
    
    // Format filename: index-name.ext or index.ext if name is empty
    snprintf(filename, 256, *base_name ? "%d-%s.%s" : "%d.%s", 
             index, *base_name ? base_name : ext, ext);
    
    return filename;
}

// Helper function to write a 16-bit value in little-endian format
static inline void write_le16(uint8_t *dest, uint16_t value) {
    dest[0] = value & 0xFF;
    dest[1] = (value >> 8) & 0xFF;
}

// Helper function to write the data block header (6 bytes: load, end, exec addresses)
static bool write_data_block_header(FILE *fp, const char *filename, const cas_DataBlockHeader *header) {
    uint8_t buffer[6];
    write_le16(&buffer[0], header->load_address);
    write_le16(&buffer[2], header->end_address);
    write_le16(&buffer[4], header->exec_address);
    
    if (fwrite(buffer, 1, 6, fp) != 6) {
        fprintf(stderr, "Error: Failed to write header to '%s'\n", filename);
        return false;
    }
    return true;
}

bool writeFileData(const char *filename, const cas_File *file, bool verbose, bool disk_format) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Error: Cannot create file '%s': %s\n", filename, strerror(errno));
        return false;
    }
    
    // DIFFERENCE FROM MCP:
    // mcp (https://github.com/ibidem/mcp) adds 0xFE for BINARY files but omits the 0xFF end marker.
    // It also includes padding bytes in all exports. We provide clean exports by default.
    //
    // BINARY FILE FORMAT
    // - 0xFE (start marker)
    // - 6-byte header (load, end, exec addresses)
    // - program data
    // - 0xFF (end marker)
    //
    // BASIC FILE FORMAT
    // - 6-byte header (load, end, exec addresses)
    // - tokenized BASIC data
    // - NO 0xFE/0xFF markers
    //
    // RATIONALE:
    // - Default export: clean data without markers (for analysis/modification)
    // - --disk-format: full MSX-compatible format with markers
    
    // For BINARY files: write 0xFE prefix if disk format
    if (isBinaryFile(file->file_header.file_type) && disk_format) {
        uint8_t prefix = 0xFE;
        if (fwrite(&prefix, 1, 1, fp) != 1) {
            fprintf(stderr, "Error: Failed to write prefix to '%s'\n", filename);
            fclose(fp);
            return false;
        }
    }
    
    // Write header for BINARY and BASIC files (BASIC has NO 0xFE/0xFF markers)
    if (isBinaryFile(file->file_header.file_type) || isBasicFile(file->file_header.file_type)) {
        if (!write_data_block_header(fp, filename, &file->data_block_header)) {
            fclose(fp);
            fprintf(stderr, "Error: Failed to write data block header to '%s'\n", filename);
            return false;
        }
    }
    
    // Write all data blocks (common for all file types)
    for (size_t i = 0; i < file->data_block_count; i++) {
        const cas_DataBlock *block = &file->data_blocks[i];
        if (block->data && block->data_size > 0) {
            size_t written = fwrite(block->data, 1, block->data_size, fp);
            if (written != block->data_size) {
                fprintf(stderr, "Error: Failed to write data to '%s'\n", filename);
                fclose(fp);
                return false;
            }
        }
    }
    
    // For BINARY files: write 0xFF suffix (if disk format)
    if (isBinaryFile(file->file_header.file_type) && disk_format) {
        uint8_t suffix = 0xFF;
        if (fwrite(&suffix, 1, 1, fp) != 1) {
            fprintf(stderr, "Error: Failed to write suffix to '%s'\n", filename);
            fclose(fp);
            return false;
        }
    }
    
    fclose(fp);
    
    if (verbose) {
        printf("Exported: %s (%zu bytes)\n", filename, file->data_size);
    }
    
    return true;
}
