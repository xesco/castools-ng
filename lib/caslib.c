#include "caslib.h"
#include <stdlib.h>
#include <string.h>

const uint8_t CAS_HEADER[8] = {0x1F, 0xA6, 0xDE, 0xBA, 0xCC, 0x13, 0x7D, 0x74};
const uint8_t FILETYPE_ASCII[10]  = {0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA};
const uint8_t FILETYPE_BINARY[10] = {0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0};
const uint8_t FILETYPE_BASIC[10]  = {0xD3, 0xD3, 0xD3, 0xD3, 0xD3, 0xD3, 0xD3, 0xD3, 0xD3, 0xD3};

// Helper to check if there's enough data to read
static bool hasBytesAvailable(size_t pos, size_t needed, size_t total_length) {
    return pos + needed <= total_length;
}

// Helper to read a little-endian 16-bit value
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

// Helper function to check if CAS header exists and read it
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
    // Round up to next 8-byte boundary for efficiency
    // CAS headers are always at 8-byte aligned positions
    size_t aligned_pos = (start_pos + 7) & ~7;

    // Search for next CAS header at 8-byte boundaries
    for (size_t i = aligned_pos; i + 8 <= length; i += 8) {
        if (isCasHeader(data, &i, length)) {
            return i;
        }
    }
    return length; // No header found, return end of data
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

// Helper function to allocate a single data block for a file
static bool allocateDataBlock(cas_File *file, const char *error_msg) {
    file->data_blocks = malloc(sizeof(cas_DataBlock));
    if (!file->data_blocks) {
        fprintf(stderr, "Failed to allocate memory for %s data block\n", error_msg);
        return false;
    }
    file->data_block_count = 1;
    return true;
}

// Helper to initialize data block with no padding
static void initializeDataBlockNoPadding(cas_DataBlock *block) {
    block->padding = NULL;
    block->padding_size = 0;
    block->padding_offset = 0;
}

// Helper function to read CAS header for a data block
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

// Helper function to allocate and copy data into a data block
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

// Helper function to allocate and copy padding into a data block
static bool allocateAndCopyPadding(cas_DataBlock *block, uint8_t *source, size_t padding_size, size_t padding_offset, const char *error_msg) {
    block->padding_size = padding_size;
    block->padding_offset = padding_offset;
    
    if (padding_size > 0) {
        block->padding = malloc(padding_size);
        if (!block->padding) {
            fprintf(stderr, "Failed to allocate memory for %s padding\n", error_msg);
            return false;
        }
        memcpy(block->padding, source, padding_size);
    } else {
        block->padding = NULL;
    }
    return true;
}

static bool parseAsciiFile(uint8_t *data, cas_File *file, size_t *pos, size_t length) {
    size_t block_count = 0;
    size_t capacity = 5;
    size_t total_data_size = 0;
    size_t total_padding_size = 0;
    bool eof_found = false;

    // Allocate initial array for data blocks
    file->data_blocks = allocateArray(capacity, sizeof(cas_DataBlock));
    if (!file->data_blocks) {
        fprintf(stderr, "Failed to allocate memory for ASCII data blocks\n");
        return false;
    }

    // Read data blocks until EOF marker is found
    while (!eof_found) {
        // Find and read next CAS header (for the data block)
        if (!tryReadCasHeader(data, pos, &file->data_blocks[block_count].header, length)) {
            break; // No more blocks
        }

        // Check if we need to resize the array
        if (block_count >= capacity) {
            if (!expandArray((void**)&file->data_blocks, &capacity, sizeof(cas_DataBlock))) {
                fprintf(stderr, "Failed to expand memory for ASCII data blocks\n");
                return false;
            }
        }

        // Find the size of this data block by searching for next header
        size_t block_start = *pos;
        size_t block_end = findNextCasHeader(data, block_start, length);
        size_t block_size = block_end - block_start;

        // Find EOF marker (0x1A) to separate data from padding
        uint8_t *eof_ptr = memchr(data + *pos, 0x1A, block_size);
        size_t data_size = eof_ptr ? (size_t)(eof_ptr - (data + *pos)) : block_size;

        if (eof_ptr) {
            eof_found = true;
        }

        // Allocate and copy data (excluding EOF marker)
        file->data_blocks[block_count].data = malloc(data_size);
        file->data_blocks[block_count].data_size = data_size;
        file->data_blocks[block_count].data_offset = *pos;  // Track file offset
        if (!file->data_blocks[block_count].data) {
            free(file->data_blocks);
            fprintf(stderr, "Failed to allocate memory for ASCII data block data\n");
            return false;
        }
        memcpy(file->data_blocks[block_count].data, data + *pos, data_size);
        total_data_size += data_size;

        // Allocate and copy padding (EOF marker and everything after)
        size_t padding_size = block_size - data_size;
        file->data_blocks[block_count].padding_size = padding_size;
        file->data_blocks[block_count].padding_offset = *pos + data_size;  // Track file offset
        total_padding_size += padding_size;
        if (padding_size > 0) {
            file->data_blocks[block_count].padding = malloc(padding_size);
            if (!file->data_blocks[block_count].padding) {
                free(file->data_blocks[block_count].data);
                free(file->data_blocks);
                fprintf(stderr, "Failed to allocate memory for ASCII data block padding\n");
                return false;
            }
            memcpy(file->data_blocks[block_count].padding, data + *pos + data_size, padding_size);
        } else {
            file->data_blocks[block_count].padding = NULL;
        }

        *pos += block_size;
        block_count++;
    }

    // Store total data size including padding
    file->data_size = total_data_size + total_padding_size;
    file->data_block_count = block_count;
    return eof_found; // Return true only if we found EOF marker
}

static bool parseBasicFile(uint8_t *data, cas_File *file, size_t *pos, size_t length) {
    if (!allocateDataBlock(file, "basic")) {
        return false;
    }

    if (!readDataBlockCasHeader(data, file, pos, length, "basic")) {
        return false;
    }

    // BASIC files don't have a data block header - just raw tokenized data
    // Find next CAS header to determine data size
    size_t data_start = *pos;
    size_t next_header_pos = findNextCasHeader(data, *pos, length);
    size_t total_size = next_header_pos - data_start;
    
    // Data size is everything up to the padding (aligned to 8 bytes)
    size_t data_size = total_size;
    size_t padding_size = 0;
    
    // Check for padding (should be at 8-byte boundary)
    if (total_size % 8 != 0) {
        // This shouldn't happen, but handle it
        data_size = total_size;
    } else {
        // Find actual data end by looking for padding zeroes
        while (data_size > 0 && data[data_start + data_size - 1] == 0) {
            data_size--;
            padding_size++;
        }
    }

    // Allocate and read the program data
    if (!allocateAndCopyData(&file->data_blocks[0], data + *pos, data_size, *pos, "basic data block")) {
        free(file->data_blocks);
        return false;
    }
    *pos += data_size;

    // Handle padding
    if (!allocateAndCopyPadding(&file->data_blocks[0], data + *pos, padding_size, *pos, "basic")) {
        free(file->data_blocks[0].data);
        free(file->data_blocks);
        return false;
    }
    *pos += padding_size;

    file->data_size = data_size + padding_size;
    return true;
}

static bool parseBinaryFile(uint8_t *data, cas_File *file, size_t *pos, size_t length) {
    if (!allocateDataBlock(file, "binary")) {
        return false;
    }

    if (!readDataBlockCasHeader(data, file, pos, length, "binary")) {
        return false;
    }

    // Read the data block header (little endian - load/end/exec addresses)
    // Note: This 6-byte header will be included in file->data_size calculation below
    if (!readDataBlockHeader(data, pos, &file->data_block_header, length)) {
        free(file->data_blocks);
        fprintf(stderr, "Failed to read data block header for binary data block\n");
        return false;
    }

    // With the addresses compute file size: end address - load address + 1
    // (the +1 includes the end address byte itself)
    size_t data_size = file->data_block_header.end_address - file->data_block_header.load_address + 1;

    // Check if we have enough data
    if (*pos + data_size > length) {
        free(file->data_blocks);
        fprintf(stderr, "Not enough data for binary data block\n");
        return false;
    }

    // Allocate and read the program data
    if (!allocateAndCopyData(&file->data_blocks[0], data + *pos, data_size, *pos, "binary data block")) {
        free(file->data_blocks);
        return false;
    }
    *pos += data_size;

    // Find next CAS header to determine actual padding
    size_t next_header_pos = findNextCasHeader(data, *pos, length);
    size_t padding_size = next_header_pos - *pos;

    // Allocate and copy padding
    if (!allocateAndCopyPadding(&file->data_blocks[0], data + *pos, padding_size, *pos, "binary")) {
        free(file->data_blocks[0].data);
        free(file->data_blocks);
        return false;
    }
    *pos += padding_size;

    // Calculate total data size (header + data + padding, excluding CAS header)
    file->data_size = 6 + data_size + padding_size;  // 6 bytes for data block header + actual data + padding
    return true;
}

static bool parseCustomFile(uint8_t *data, cas_File *file, size_t *pos, size_t length) {
    // For custom blocks, just read raw data until next CAS_HEADER
    file->is_custom = true;
    size_t start_pos = *pos;

    // Find next CAS_HEADER or end of file
    size_t next_header_pos = findNextCasHeader(data, *pos, length);
    file->data_size = next_header_pos - start_pos;

    if (!allocateDataBlock(file, "custom")) {
        return false;
    }

    // Allocate and copy the custom data
    if (!allocateAndCopyData(&file->data_blocks[0], data + *pos, file->data_size, *pos, "custom")) {
        free(file->data_blocks);
        return false;
    }
    initializeDataBlockNoPadding(&file->data_blocks[0]);
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

    // Peek at file type to determine parser
    uint8_t file_type[10];
    if (!peekFileType(data, *pos, file_type, length)) {
        fprintf(stderr, "Truncated data: not enough bytes for file type\n");
        return false;
    }

    // Check if it matches a known file type
    bool is_ascii  = isAsciiFile(file_type);
    bool is_binary = isBinaryFile(file_type);
    bool is_basic  = isBasicFile(file_type);

    // If it matches a known type, read the full file header
    if (is_ascii || is_binary || is_basic) {
        if (!readFileHeader(data, pos, &file->file_header, length)) {
            fprintf(stderr, "Failed to read file header\n");
            return false;
        }

        // Check file type and use appropriate parser
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

    // Unknown type - treat as custom block
    return parseCustomFile(data, file, pos, length);
}

bool parseCasContainer(uint8_t *data, cas_Container *container, size_t length) {
    size_t pos = 0;
    size_t capacity = 5;
    container->file_count = 0;

    // Allocate initial array for files
    container->files = allocateArray(capacity, sizeof(cas_File));
    if (!container->files) {
        fprintf(stderr, "Failed to allocate memory for CAS container files\n");
        return false;
    }

    // Parse files while we can find CAS headers
    while (pos < length && isCasHeader(data, &pos, length)) {
        // Check if we need to resize the array
        if (container->file_count >= capacity) {
            if (!expandArray((void**)&container->files, &capacity, sizeof(cas_File))) {
                return false;
            }
        }

        if (!parseFile(data, &container->files[container->file_count], &pos, length)) {
            // Not a valid file, stop parsing
            fprintf(stderr, "Failed to parse file at position %zu\n", pos);
            break;
        }
        container->file_count++;
    }
    return true;
}
