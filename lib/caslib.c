#include "caslib.h"
#include <stdlib.h>
#include <string.h>

const uint8_t CAS_HEADER[8] = {0x1F, 0xA6, 0xDE, 0xBA, 0xCC, 0x13, 0x7D, 0x74};
const uint8_t FILETYPE_ASCII[10]  = {0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA};
const uint8_t FILETYPE_BINARY[10] = {0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0};
const uint8_t FILETYPE_BASIC[10]  = {0xD3, 0xD3, 0xD3, 0xD3, 0xD3, 0xD3, 0xD3, 0xD3, 0xD3, 0xD3};

bool isCasHeader(uint8_t *data, size_t *pos, size_t length) {
    if (*pos + sizeof(CAS_HEADER) <= length) {
        if (memcmp(data + *pos, CAS_HEADER, sizeof(CAS_HEADER)) == 0) {
            return true;
        }
    }
    return false;
}

bool readCasHeader(uint8_t *data, size_t *pos, cas_Header *header, size_t length) {
    if (*pos + sizeof(CAS_HEADER) > length) {
        return false;
    }
    memcpy(header->bytes, data + *pos, sizeof(CAS_HEADER));
    *pos += sizeof(CAS_HEADER);
    return true;
}

bool readFileHeader(uint8_t *data, size_t *pos, cas_FileHeader *file_header, size_t length) {
    if (*pos + sizeof(cas_FileHeader) > length) {
        return false;
    }
    memcpy(file_header->file_type, data + *pos, sizeof(file_header->file_type));
    *pos += sizeof(file_header->file_type);
    memcpy(file_header->file_name, data + *pos, sizeof(file_header->file_name));
    *pos += sizeof(file_header->file_name);
    return true;
}

bool peekFileType(uint8_t *data, size_t pos, uint8_t *file_type, size_t length) {
    if (pos + 10 > length) {
        return false;
    }
    memcpy(file_type, data + pos, 10);
    return true;
}

bool compareFileType(const uint8_t *file_type, const uint8_t *type_pattern) {
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

bool readDataBlockHeader(uint8_t *data, size_t *pos, cas_DataBlockHeader *data_block_header, size_t length) {
    if (*pos + sizeof(cas_DataBlockHeader) > length) {
        return false;
    }
    data_block_header->load_address = data[*pos] | (data[*pos+1] << 8);
    *pos += 2;
    data_block_header->end_address  = data[*pos] | (data[*pos+1] << 8);
    *pos += 2;
    data_block_header->exec_address = data[*pos] | (data[*pos+1] << 8);
    *pos += 2;
    return true;
}

size_t findNextCasHeader(uint8_t *data, size_t start_pos, size_t length) {
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

bool parseAsciiFile(uint8_t *data, cas_File *file, size_t *pos, size_t length) {
    size_t block_count = 0;
    size_t capacity = 5;
    size_t total_data_size = 0;
    bool eof_found = false;

    // Allocate initial array for data blocks
    file->data_blocks = allocateArray(capacity, sizeof(cas_DataBlock));
    if (!file->data_blocks) {
        fprintf(stderr, "Failed to allocate memory for ASCII data blocks\n");
        return false;
    }

    // Read data blocks until EOF marker is found
    while (!eof_found) {
        // Find next CAS header (for the data block)
        if (!isCasHeader(data, pos, length)) {
            break; // No more blocks
        }

        // Check if we need to resize the array
        if (block_count >= capacity) {
            if (!expandArray((void**)&file->data_blocks, &capacity, sizeof(cas_DataBlock))) {
                fprintf(stderr, "Failed to expand memory for ASCII data blocks\n");
                return false;
            }
        }

        // Read the CAS header
        if (!readCasHeader(data, pos, &file->data_blocks[block_count].header, length)) {
            free(file->data_blocks);
            fprintf(stderr, "Failed to read CAS header for ASCII data block\n");
            return false;
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

    // Store total data size summed from actual blocks
    file->data_size = total_data_size;
    file->data_block_count = block_count;
    return eof_found; // Return true only if we found EOF marker
}

bool parseBinaryFile(uint8_t *data, cas_File *file, size_t *pos, size_t length) {
    // Allocate one data block (binary/basic files only have one block of data)
    file->data_blocks = malloc(sizeof(cas_DataBlock));
    if (!file->data_blocks) {
        fprintf(stderr, "Failed to allocate memory for binary data block\n");
        return false;
    }

    // Find the second CAS header (for the data block)
    if (!isCasHeader(data, pos, length)) {
        free(file->data_blocks);
        fprintf(stderr, "Failed to find second CAS header for binary data block\n");
        return false;
    }

    // Read the CAS header into the data block struct
    if (!readCasHeader(data, pos, &file->data_blocks[0].header, length)) {
        free(file->data_blocks);
        fprintf(stderr, "Failed to read CAS header for binary data block\n");
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
    // (end address is inclusive - it's the address of the last byte)
    size_t data_size = file->data_block_header.end_address - file->data_block_header.load_address + 1;

    // Check if we have enough data
    if (*pos + data_size > length) {
        free(file->data_blocks);
        fprintf(stderr, "Not enough data for binary data block\n");
        return false;
    }

    // Allocate and read the program data
    file->data_blocks[0].data = malloc(data_size);
    file->data_blocks[0].data_size = data_size;
    if (!file->data_blocks[0].data) {
        free(file->data_blocks);
        fprintf(stderr, "Failed to allocate memory for binary data block data\n");
        return false;
    }
    memcpy(file->data_blocks[0].data, data + *pos, data_size);
    *pos += data_size;

    // Find next CAS header to determine actual padding
    size_t next_header_pos = findNextCasHeader(data, *pos, length);

    // Calculate actual padding size (bytes between end of data and next header)
    size_t padding_size = next_header_pos - *pos;
    file->data_blocks[0].padding_size = padding_size;

    if (padding_size > 0) {
        file->data_blocks[0].padding = malloc(padding_size);
        if (!file->data_blocks[0].padding) {
            free(file->data_blocks[0].data);
            free(file->data_blocks);
            fprintf(stderr, "Failed to allocate memory for binary padding\n");
            return false;
        }
        memcpy(file->data_blocks[0].padding, data + *pos, padding_size);
        *pos += padding_size;
    } else {
        file->data_blocks[0].padding = NULL;
    }

    // Calculate total data size (header + data, excluding CAS header and padding)
    file->data_size = 6 + data_size;  // 6 bytes for data block header + actual data
    file->data_block_count = 1;
    return true;
}

bool parseCustomFile(uint8_t *data, cas_File *file, size_t *pos, size_t length) {
    // For custom blocks, just read raw data until next CAS_HEADER
    file->is_custom = true;
    size_t start_pos = *pos;

    // Find next CAS_HEADER or end of file
    size_t next_header_pos = findNextCasHeader(data, *pos, length);
    file->data_size = next_header_pos - start_pos;

    // Allocate one data block to store the custom data
    file->data_blocks = malloc(sizeof(cas_DataBlock));
    if (!file->data_blocks) {
        fprintf(stderr, "Failed to allocate memory for custom data block\n");
        return false;
    }

    // Allocate and copy the custom data
    file->data_blocks[0].data = malloc(file->data_size);
    file->data_blocks[0].data_size = file->data_size;
    if (!file->data_blocks[0].data) {
        fprintf(stderr, "Failed to allocate memory for custom data\n");
        free(file->data_blocks);
        return false;
    }
    memcpy(file->data_blocks[0].data, data + *pos, file->data_size);
    file->data_blocks[0].padding = NULL;
    file->data_blocks[0].padding_size = 0;
    file->data_block_count = 1;

    *pos = next_header_pos;
    return true;
}

bool expandArray(void **array, size_t *capacity, size_t item_size) {
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

void* allocateArray(size_t capacity, size_t item_size) {
    void *array = malloc(capacity * item_size);
    if (!array) {
        fprintf(stderr, "Failed to allocate memory for array\n");
    }
    return array;
}

bool parseFile(uint8_t *data, cas_File *file, size_t *pos, size_t length) {
    file->is_custom = false;

    if (!readCasHeader(data, pos, &file->header, length)) {
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
        if (is_binary || is_basic) {
            if (!parseBinaryFile(data, file, pos, length)) {
                fprintf(stderr, "Failed to parse binary/basic file\n");
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
    while (isCasHeader(data, &pos, length)) {
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
