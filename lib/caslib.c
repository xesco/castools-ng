#include "caslib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const uint8_t CAS_HEADER[8] = {0x1F, 0xA6, 0xDE, 0xBA, 0xCC, 0x13, 0x7D, 0x74};
const uint8_t FILETYPE_ASCII[10]  = {0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA};
const uint8_t FILETYPE_BINARY[10] = {0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0};
const uint8_t FILETYPE_BASIC[10]  = {0xD3, 0xD3, 0xD3, 0xD3, 0xD3, 0xD3, 0xD3, 0xD3, 0xD3, 0xD3};

static bool hasBytesAvailable(size_t pos, size_t needed, size_t total_length) {
    return pos + needed <= total_length;
}

static uint16_t readLittleEndian16(const uint8_t *data) {
    return data[0] | (data[1] << 8);
}

static bool isCasHeader(uint8_t *data, size_t *pos, size_t length) {
    if (!hasBytesAvailable(*pos, sizeof(CAS_HEADER), length)) {
        return false;
    }
    return memcmp(data + *pos, CAS_HEADER, sizeof(CAS_HEADER)) == 0;
}

static bool readCasHeader(uint8_t *data, size_t *pos, cas_Header *header, size_t length) {
    if (!hasBytesAvailable(*pos, sizeof(CAS_HEADER), length)) {
        return false;
    }
    memcpy(header->bytes, data + *pos, sizeof(CAS_HEADER));
    *pos += sizeof(CAS_HEADER);
    return true;
}

static bool tryReadCasHeader(uint8_t *data, size_t *pos, cas_Header *header, size_t length) {
    if (!isCasHeader(data, pos, length)) {
        return false;
    }
    return readCasHeader(data, pos, header, length);
}

static bool readFileHeader(uint8_t *data, size_t *pos, cas_FileHeader *file_header, size_t length) {
    if (!hasBytesAvailable(*pos, sizeof(cas_FileHeader), length)) {
        return false;
    }
    memcpy(file_header->file_type, data + *pos, sizeof(file_header->file_type));
    *pos += sizeof(file_header->file_type);
    memcpy(file_header->file_name, data + *pos, sizeof(file_header->file_name));
    *pos += sizeof(file_header->file_name);
    return true;
}

static bool peekFileType(uint8_t *data, size_t pos, uint8_t *file_type, size_t length) {
    if (!hasBytesAvailable(pos, 10, length)) {
        return false;
    }
    memcpy(file_type, data + pos, 10);
    return true;
}

static bool compareFileType(const uint8_t *file_type, const uint8_t *type_pattern) {
    return memcmp(file_type, type_pattern, 10) == 0;
}

bool isAsciiFile(const uint8_t *file_type) {
    return compareFileType(file_type, FILETYPE_ASCII);
}

bool isBinaryFile(const uint8_t *file_type) {
    return compareFileType(file_type, FILETYPE_BINARY);
}

bool isBasicFile(const uint8_t *file_type) {
    return compareFileType(file_type, FILETYPE_BASIC);
}

const char* getFileTypeString(const cas_File *file) {
    if (file->is_custom) {
        return "CUSTOM";
    } else if (isAsciiFile(file->file_header.file_type)) {
        return "ASCII";
    } else if (isBinaryFile(file->file_header.file_type)) {
        return "BINARY";
    } else if (isBasicFile(file->file_header.file_type)) {
        return "BASIC";
    } else {
        return "UNKNOWN";
    }
}

char* generateFilename(const cas_File *file, int index) {
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

static bool readDataBlockHeader(uint8_t *data, size_t *pos, cas_DataBlockHeader *data_block_header, size_t length) {
    if (!hasBytesAvailable(*pos, sizeof(cas_DataBlockHeader), length)) {
        return false;
    }
    data_block_header->load_address = readLittleEndian16(&data[*pos]);
    *pos += 2;
    data_block_header->end_address  = readLittleEndian16(&data[*pos]);
    *pos += 2;
    data_block_header->exec_address = readLittleEndian16(&data[*pos]);
    *pos += 2;
    return true;
}

static size_t findNextCasHeader(uint8_t *data, size_t start_pos, size_t length) {
    // Search for next CAS header starting from start_pos
    // Note: Headers are NOT always 8-byte aligned (e.g., RUR1.cas has one at 0x9042)
    for (size_t i = start_pos; i + 8 <= length; i++) {
        if (memcmp(data + i, CAS_HEADER, 8) == 0) {
            return i;
        }
    }
    return length;
}

static bool expandArray(void **array, size_t *capacity, size_t item_size) {
    *capacity *= 2;
    void *new_array = realloc(*array, (*capacity) * item_size);
    if (!new_array) {
        fprintf(stderr, "Failed to reallocate memory for array\n");
        free(*array);
        *array = NULL;
        return false;
    }
    *array = new_array;
    return true;
}

static void* allocateArray(size_t capacity, size_t item_size) {
    void *array = malloc(capacity * item_size);
    if (!array) {
        fprintf(stderr, "Failed to allocate memory for array\n");
    }
    return array;
}

static bool allocateDataBlock(cas_File *file, const char *error_msg) {
    file->data_blocks = malloc(sizeof(cas_DataBlock));
    if (!file->data_blocks) {
        fprintf(stderr, "Failed to allocate memory for %s data block\n", error_msg);
        return false;
    }
    file->data_block_count = 1;
    return true;
}

static bool readDataBlockCasHeader(uint8_t *data, cas_File *file, size_t *pos, size_t length, const char *error_msg) {
    if (!isCasHeader(data, pos, length)) {
        free(file->data_blocks);
        fprintf(stderr, "Failed to find second CAS header for %s data block\n", error_msg);
        return false;
    }

    if (!readCasHeader(data, pos, &file->data_blocks[0].header, length)) {
        free(file->data_blocks);
        fprintf(stderr, "Failed to read CAS header for %s data block\n", error_msg);
        return false;
    }
    return true;
}

static bool allocateAndCopyData(cas_DataBlock *block, uint8_t *source, size_t data_size, size_t data_offset, const char *error_msg) {
    block->data = malloc(data_size);
    block->data_size = data_size;
    block->data_offset = data_offset;
    if (!block->data) {
        fprintf(stderr, "Failed to allocate memory for %s data\n", error_msg);
        return false;
    }
    memcpy(block->data, source, data_size);
    return true;
}



static bool parseAsciiFile(uint8_t *data, cas_File *file, size_t *pos, size_t length) {
    size_t block_count = 0;
    size_t capacity = 5;
    size_t total_data_size = 0;
    bool eof_found = false;

    file->data_blocks = allocateArray(capacity, sizeof(cas_DataBlock));
    if (!file->data_blocks) {
        fprintf(stderr, "Failed to allocate memory for ASCII data blocks\n");
        return false;
    }

    // Read blocks separated by CAS headers until EOF marker (0x1A)
    while (!eof_found && *pos < length) {
        if (!tryReadCasHeader(data, pos, &file->data_blocks[block_count].header, length)) {
            break;
        }

        if (block_count >= capacity) {
            if (!expandArray((void**)&file->data_blocks, &capacity, sizeof(cas_DataBlock))) {
                fprintf(stderr, "Failed to expand memory for ASCII data blocks\n");
                return false;
            }
        }

        size_t block_start = *pos;
        
        while (*pos < length && !isCasHeader(data, pos, length)) {
            if (data[*pos] == 0x1A) {
                eof_found = true;
                (*pos)++;
                // Skip remaining bytes until next header
                while (*pos < length && !isCasHeader(data, pos, length)) {
                    (*pos)++;
                }
                break;
            }
            (*pos)++;
        }
        
        size_t block_size = *pos - block_start;
        if (!allocateAndCopyData(&file->data_blocks[block_count], data + block_start, block_size, block_start, "ASCII data block")) {
            free(file->data_blocks);
            return false;
        }
        total_data_size += block_size;
        block_count++;
    }

    file->data_size = total_data_size;
    file->data_block_count = block_count;
    return eof_found;
}

static bool parseBasicFile(uint8_t *data, cas_File *file, size_t *pos, size_t length) {
    if (!allocateDataBlock(file, "basic")) {
        return false;
    }

    if (!readDataBlockCasHeader(data, file, pos, length, "basic")) {
        return false;
    }

    // BASIC files have raw tokenized data (no 6-byte header like BINARY)
    size_t data_start = *pos;
    size_t next_header_pos = findNextCasHeader(data, *pos, length);
    size_t data_size = next_header_pos - data_start;
    if (!allocateAndCopyData(&file->data_blocks[0], data + *pos, data_size, *pos, "basic data block")) {
        free(file->data_blocks);
        return false;
    }
    *pos += data_size;

    file->data_size = data_size;
    return true;
}

static bool parseBinaryFile(uint8_t *data, cas_File *file, size_t *pos, size_t length) {
    if (!allocateDataBlock(file, "binary")) {
        return false;
    }

    if (!readDataBlockCasHeader(data, file, pos, length, "binary")) {
        return false;
    }

    // Read 6-byte data block header (load/end/exec addresses)
    if (!readDataBlockHeader(data, pos, &file->data_block_header, length)) {
        free(file->data_blocks);
        fprintf(stderr, "Failed to read data block header for binary data block\n");
        return false;
    }

    // Find data size by scanning for next CAS header (end_address field is unreliable)
    size_t data_start = *pos;
    size_t next_header_pos = findNextCasHeader(data, *pos, length);
    size_t data_size = next_header_pos - data_start;

    if (*pos + data_size > length) {
        free(file->data_blocks);
        fprintf(stderr, "Not enough data for binary data block\n");
        return false;
    }

    if (!allocateAndCopyData(&file->data_blocks[0], data + *pos, data_size, *pos, "binary data block")) {
        free(file->data_blocks);
        return false;
    }
    *pos += data_size;

    file->data_size = 6 + data_size;  // 6-byte header + data
    return true;
}

static bool parseCustomFile(uint8_t *data, cas_File *file, size_t *pos, size_t length) {
    file->is_custom = true;
    size_t start_pos = *pos;

    size_t next_header_pos = findNextCasHeader(data, *pos, length);
    file->data_size = next_header_pos - start_pos;

    if (!allocateDataBlock(file, "custom")) {
        return false;
    }

    if (!allocateAndCopyData(&file->data_blocks[0], data + *pos, file->data_size, *pos, "custom")) {
        free(file->data_blocks);
        return false;
    }
    file->data_block_count = 1;

    *pos = next_header_pos;
    return true;
}

static bool parseFile(uint8_t *data, cas_File *file, size_t *pos, size_t length) {
    file->is_custom = false;

    if (!tryReadCasHeader(data, pos, &file->header, length)) {
        fprintf(stderr, "Failed to read CAS header\n");
        return false;
    }

    uint8_t file_type[10];
    if (!peekFileType(data, *pos, file_type, length)) {
        fprintf(stderr, "Truncated data: not enough bytes for file type\n");
        return false;
    }

    bool is_ascii  = isAsciiFile(file_type);
    bool is_binary = isBinaryFile(file_type);
    bool is_basic  = isBasicFile(file_type);

    if (is_ascii || is_binary || is_basic) {
        if (!readFileHeader(data, pos, &file->file_header, length)) {
            fprintf(stderr, "Failed to read file header\n");
            return false;
        }

        if (is_binary) {
            if (!parseBinaryFile(data, file, pos, length)) {
                fprintf(stderr, "Failed to parse binary file\n");
                return false;
            }
            return true;
        } else if (is_basic) {
            if (!parseBasicFile(data, file, pos, length)) {
                fprintf(stderr, "Failed to parse basic file\n");
                return false;
            }
            return true;
        } else if (is_ascii) {
            if (!parseAsciiFile(data, file, pos, length)) {
                fprintf(stderr, "Failed to parse ASCII file\n");
                return false;
            }
            return true;
        }
    }

    return parseCustomFile(data, file, pos, length);
}

bool parseCasContainer(uint8_t *data, cas_Container *container, size_t length) {
    size_t pos = 0;
    size_t capacity = 5;
    container->file_count = 0;

    container->files = allocateArray(capacity, sizeof(cas_File));
    if (!container->files) {
        fprintf(stderr, "Failed to allocate memory for CAS container files\n");
        return false;
    }

    while (pos < length && isCasHeader(data, &pos, length)) {
        if (container->file_count >= capacity) {
            if (!expandArray((void**)&container->files, &capacity, sizeof(cas_File))) {
                return false;
            }
        }

        if (!parseFile(data, &container->files[container->file_count], &pos, length)) {
            fprintf(stderr, "Failed to parse file at position %zu\n", pos);
            break;
        }
        container->file_count++;
    }
    return true;
}
