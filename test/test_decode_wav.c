#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/wavlib.h"
#include "../lib/caslib.h"

// Read a WAV file and decode the bits to see if they match the CAS file
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <wav_file> <cas_file>\n", argv[0]);
        return 1;
    }
    
    const char *wav_filename = argv[1];
    const char *cas_filename = argv[2];
    
    // Read CAS file for comparison
    FILE *fp = fopen(cas_filename, "rb");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open CAS file\n");
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    size_t cas_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    uint8_t *cas_data = malloc(cas_size);
    fread(cas_data, 1, cas_size, fp);
    fclose(fp);
    
    cas_Container container;
    if (!parseCasContainer(cas_data, &container, cas_size)) {
        fprintf(stderr, "Error: Failed to parse CAS file\n");
        free(cas_data);
        return 1;
    }
    
    printf("CAS file has %zu files:\n", container.file_count);
    for (size_t i = 0; i < container.file_count; i++) {
        const cas_File *file = &container.files[i];
        if (file->is_custom) {
            printf("  File %zu: CUSTOM, %zu data blocks\n", i+1, file->data_block_count);
        } else {
            printf("  File %zu: %s \"%.6s\", %zu data blocks\n", i+1, 
                   getFileTypeString(file), (char*)file->file_header.file_name,
                   file->data_block_count);
        }
        for (size_t j = 0; j < file->data_block_count; j++) {
            printf("    Block %zu: %zu bytes\n", j+1, file->data_blocks[j].data_size);
            // Print first few bytes
            printf("      First bytes: ");
            for (size_t k = 0; k < 16 && k < file->data_blocks[j].data_size; k++) {
                printf("%02x ", file->data_blocks[j].data[k]);
            }
            printf("\n");
        }
    }
    
    free(cas_data);
    
    // Now let's check the WAV file size and duration
    FILE *wav_fp = fopen(wav_filename, "rb");
    if (!wav_fp) {
        fprintf(stderr, "Error: Cannot open WAV file\n");
        return 1;
    }
    
    // Read WAV header
    fseek(wav_fp, 0, SEEK_END);
    long wav_size = ftell(wav_fp);
    fseek(wav_fp, 0, SEEK_SET);
    
    // Read RIFF header
    char riff[4];
    uint32_t chunk_size;
    char wave[4];
    fread(riff, 1, 4, wav_fp);
    fread(&chunk_size, 4, 1, wav_fp);
    fread(wave, 1, 4, wav_fp);
    
    if (memcmp(riff, "RIFF", 4) != 0 || memcmp(wave, "WAVE", 4) != 0) {
        fprintf(stderr, "Error: Not a valid WAV file\n");
        fclose(wav_fp);
        return 1;
    }
    
    // Find fmt chunk
    bool found_fmt = false;
    uint16_t sample_rate = 0;
    uint16_t channels = 0;
    uint16_t bits_per_sample = 0;
    
    while (!feof(wav_fp)) {
        char chunk_id[4];
        uint32_t chunk_len;
        
        if (fread(chunk_id, 1, 4, wav_fp) != 4) break;
        if (fread(&chunk_len, 4, 1, wav_fp) != 1) break;
        
        if (memcmp(chunk_id, "fmt ", 4) == 0) {
            uint16_t audio_format;
            fread(&audio_format, 2, 1, wav_fp);
            fread(&channels, 2, 1, wav_fp);
            fread(&sample_rate, 4, 1, wav_fp);
            // Skip rest of fmt chunk
            fseek(wav_fp, chunk_len - 10, SEEK_CUR);
            found_fmt = true;
        } else if (memcmp(chunk_id, "data", 4) == 0) {
            printf("\nWAV file info:\n");
            printf("  Sample rate: %u Hz\n", sample_rate);
            printf("  Data size: %u bytes\n", chunk_len);
            printf("  Duration: %.2f seconds\n", (double)chunk_len / sample_rate);
            printf("  File size: %ld bytes\n", wav_size);
            break;
        } else {
            // Skip unknown chunk
            fseek(wav_fp, chunk_len, SEEK_CUR);
        }
    }
    
    fclose(wav_fp);
    
    printf("\n✓ Analysis complete\n");
    return 0;
}
