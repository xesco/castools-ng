#ifndef CASLIB_H
#define CASLIB_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define ASCII_BLOCK_SIZE 256

// Disk BASIC file format markers
#define BINARY_FILE_ID_BYTE 0xFE  // Marks start of binary stream (disk BASIC format)
#define BASIC_FILE_ID_BYTE  0xFF  // Marks tokenized BASIC file (disk format only)
#define EOF_MARKER          0x1A  // End-of-file marker for ASCII files

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
    size_t data_offset;      // offset in file where data starts
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
bool isAsciiFile(const uint8_t *file_type);
bool isBinaryFile(const uint8_t *file_type);
bool isBasicFile(const uint8_t *file_type);
const char* getFileTypeString(const cas_File *file);
char* generateFilename(const cas_File *file, int index);
bool parseCasContainer(uint8_t *data, cas_Container *container, size_t length);

#endif // CASLIB_H
