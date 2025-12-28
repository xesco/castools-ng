#ifndef CASLIB_H
#define CASLIB_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define ASCII_BLOCK_SIZE 256

extern const uint8_t CAS_HEADER[8];
extern const uint8_t FILETYPE_ASCII[10];
extern const uint8_t FILETYPE_BINARY[10];
extern const uint8_t FILETYPE_BASIC[10];

typedef struct {
    uint8_t bytes[8];
} cas_Header;

typedef struct {
    uint8_t file_type[10];
    uint8_t file_name[6];
} cas_FileHeader;

typedef struct {
    cas_Header header;
    uint8_t* data;
    size_t data_size;
    uint8_t* padding;
    size_t padding_size;
} cas_DataBlock;

typedef struct {
    uint16_t load_address;
    uint16_t end_address;
    uint16_t exec_address;
} cas_DataBlockHeader;

typedef struct {
    cas_Header header;
    cas_FileHeader file_header;
    cas_DataBlockHeader data_block_header;
    cas_DataBlock* data_blocks;
    size_t data_block_count;  // number of data blocks
    bool is_custom;  // true if this is a custom/data block with no file header
    size_t data_size;  // actual data size (for all file types)
} cas_File;

typedef struct {
    cas_File* files;
    size_t file_count;
} cas_Container;

// Function declarations
bool isCasHeader(uint8_t *data, size_t *pos, size_t length);
bool readCasHeader(uint8_t *data, size_t *pos, cas_Header *header, size_t length);
bool readFileHeader(uint8_t *data, size_t *pos, cas_FileHeader *file_header, size_t length);
bool peekFileType(uint8_t *data, size_t pos, uint8_t *file_type, size_t length);
bool compareFileType(const uint8_t *file_type, const uint8_t *type_pattern);
bool isAsciiFile(const uint8_t *file_type);
bool isBinaryFile(const uint8_t *file_type);
bool isBasicFile(const uint8_t *file_type);
bool readDataBlockHeader(uint8_t *data, size_t *pos, cas_DataBlockHeader *data_block_header, size_t length);
size_t findNextCasHeader(uint8_t *data, size_t start_pos, size_t length);
bool parseAsciiFile(uint8_t *data, cas_File *file, size_t *pos, size_t length);
bool parseBinaryFile(uint8_t *data, cas_File *file, size_t *pos, size_t length);
bool parseCustomFile(uint8_t *data, cas_File *file, size_t *pos, size_t length);
bool expandArray(void **array, size_t *capacity, size_t item_size);
void* allocateArray(size_t capacity, size_t item_size);
bool parseFile(uint8_t *data, cas_File *file, size_t *pos, size_t length);
bool parseCasContainer(uint8_t *data, cas_Container *container, size_t length);

#endif // CASLIB_H
