#ifndef PRINTLIB_H
#define PRINTLIB_H

#include "caslib.h"

// Function declarations for printing CAS structures
void printHexDump(const uint8_t *data, size_t size, size_t base_offset);
void printCasHeader(const cas_Header *header);
void printFileHeader(const cas_FileHeader *file_header);
void printDataBlockHeader(const cas_DataBlockHeader *data_block_header);
void printDataBlock(const cas_DataBlock *data_block, size_t block_num);
void printFile(const cas_File *file, size_t file_num);
void printContainer(const cas_Container *container);

#endif // PRINTLIB_H
