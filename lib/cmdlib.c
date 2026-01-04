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
    
    // For disk format: add MSX disk file identifier bytes
    // BASIC: 0xFF prefix, BINARY: 0xFE prefix
    if (disk_format) {
        if (isBasicFile(file->file_header.file_type)) {
            uint8_t prefix = 0xFF;
            if (fwrite(&prefix, 1, 1, fp) != 1) {
                fprintf(stderr, "Error: Failed to write BASIC prefix to '%s'\n", filename);
                fclose(fp);
                return false;
            }
            if (verbose) {
                printf("Added 0xFF prefix (BASIC file identifier)\n");
            }
        } else if (isBinaryFile(file->file_header.file_type)) {
            uint8_t prefix = BINARY_FILE_ID_BYTE;
            if (fwrite(&prefix, 1, 1, fp) != 1) {
                fprintf(stderr, "Error: Failed to write binary prefix to '%s'\n", filename);
                fclose(fp);
                return false;
            }
            if (verbose) {
                printf("Added 0xFE prefix (BSAVE file identifier)\n");
            }
        }
    }
    
    // Write header for BINARY files only (BINARY has 6-byte address header, BASIC does not)
    if (isBinaryFile(file->file_header.file_type)) {
        if (!write_data_block_header(fp, filename, &file->data_block_header)) {
            fclose(fp);
            fprintf(stderr, "Error: Failed to write data block header to '%s'\n", filename);
            return false;
        }
    }
    
    // Write all data blocks
    for (size_t i = 0; i < file->data_block_count; i++) {
        const cas_DataBlock *block = &file->data_blocks[i];
        if (block->data && block->data_size > 0) {
            size_t write_size = block->data_size;
            
            // For ASCII files, stop at EOF marker (0x1A) - exclude the marker itself
            if (isAsciiFile(file->file_header.file_type)) {
                for (size_t j = 0; j < block->data_size; j++) {
                    if (block->data[j] == 0x1A) {
                        write_size = j;  // Stop before 0x1A
                        break;
                    }
                }
            }
            
            size_t written = fwrite(block->data, 1, write_size, fp);
            if (written != write_size) {
                fprintf(stderr, "Error: Failed to write data to '%s'\n", filename);
                fclose(fp);
                return false;
            }
        }
    }
    
    fclose(fp);
    
    if (verbose) {
        printf("Exported: %s (%zu bytes)\n", filename, file->data_size);
    }
    
    return true;
}

void formatBytes(size_t bytes, char *buffer, size_t buffer_size) {
    if (bytes < 1024) {
        snprintf(buffer, buffer_size, "%zu bytes", bytes);
    } else if (bytes < 1024 * 1024) {
        snprintf(buffer, buffer_size, "%.1f KB", bytes / 1024.0);
    } else {
        snprintf(buffer, buffer_size, "%.1f MB", bytes / (1024.0 * 1024.0));
    }
}

void formatDuration(double seconds, char *buffer, size_t buffer_size) {
    int total_secs = (int)(seconds + 0.5);  // Round to nearest second
    int minutes = total_secs / 60;
    int secs = total_secs % 60;
    snprintf(buffer, buffer_size, "%d:%02d", minutes, secs);
}

char* generateOutputFilename(const char *input_file, const char *new_ext) {
    // Get basename (filename without directory path)
    const char *basename = strrchr(input_file, '/');
    basename = basename ? basename + 1 : input_file;
    
    // Find the last dot for extension
    const char *dot = strrchr(basename, '.');
    size_t name_len = dot ? (size_t)(dot - basename) : strlen(basename);
    
    // Calculate total length: name + "." + ext + null
    size_t output_len = name_len + 1 + strlen(new_ext) + 1;
    
    // Allocate buffer
    char *output = malloc(output_len);
    if (!output) {
        fprintf(stderr, "Error: Failed to allocate memory for output filename\n");
        return NULL;
    }
    
    // Build output filename: basename without extension + new extension
    snprintf(output, output_len, "%.*s.%s", (int)name_len, basename, new_ext);
    
    return output;
}
