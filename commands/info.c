#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../lib/caslib.h"
#include "../lib/cmdlib.h"
#include "../lib/wavlib.h"

int execute_info(const char *input_file, bool verbose) {
    if (verbose) {
        printf("Reading file: %s\n", input_file);
    }
    
    // Read the file into memory
    size_t file_size;
    uint8_t *file_data = readFileIntoMemory(input_file, &file_size);
    if (!file_data) {
        fprintf(stderr, "Error: Failed to read file '%s'\n", input_file);
        return 1;
    }
    
    if (verbose) {
        printf("File size: %zu bytes\n", file_size);
        printf("Parsing CAS container...\n\n");
    }
    
    // Parse the CAS container
    cas_Container container;
    if (!parseCasContainer(file_data, &container, file_size)) {
        fprintf(stderr, "Error: Failed to parse CAS container\n");
        free(file_data);
        return 1;
    }
    
    // =============================================================================
    // 1. FILE STATISTICS
    // =============================================================================
    
    printf("Container Statistics\n");
    printf("====================\n");
    
    printf("Total files: %zu\n", container.file_count);
    
    // Count files by type
    size_t ascii_count = 0;
    size_t binary_count = 0;
    size_t basic_count = 0;
    size_t custom_count = 0;
    
    for (size_t i = 0; i < container.file_count; i++) {
        const cas_File *file = &container.files[i];
        if (file->is_custom) {
            custom_count++;
        } else if (isAsciiFile(file->file_header.file_type)) {
            ascii_count++;
        } else if (isBinaryFile(file->file_header.file_type)) {
            binary_count++;
        } else if (isBasicFile(file->file_header.file_type)) {
            basic_count++;
        }
    }
    
    printf("  ASCII:  %zu\n", ascii_count);
    printf("  Binary: %zu\n", binary_count);
    printf("  BASIC:  %zu\n", basic_count);
    printf("  Custom: %zu\n", custom_count);
    
    char size_str[64];
    formatBytes(file_size, size_str, sizeof(size_str));
    printf("\nContainer size: %s\n", size_str);
    
    // =============================================================================
    // 2. AUDIO DURATION ESTIMATES
    // =============================================================================
    
    printf("\nAudio Estimates\n");
    printf("===============\n");
    
    // Calculate durations for both baud rates (using standard silence: 2.0s/1.0s)
    double duration_1200 = calculateAudioDuration(&container, 1200, 2.0, 1.0);
    double duration_2400 = calculateAudioDuration(&container, 2400, 2.0, 1.0);
    
    char dur_1200_str[32];
    char dur_2400_str[32];
    formatDuration(duration_1200, dur_1200_str, sizeof(dur_1200_str));
    formatDuration(duration_2400, dur_2400_str, sizeof(dur_2400_str));
    
    printf("At 1200 baud (standard):\n");
    printf("  Duration:  %s (%d seconds)\n", dur_1200_str, (int)ceil(duration_1200));
    
    size_t wav_size_1200 = calculateWavFileSize(duration_1200, 43200);
    formatBytes(wav_size_1200, size_str, sizeof(size_str));
    printf("  WAV size:  %s (43200 Hz, 8-bit mono)\n", size_str);
    
    printf("\nAt 2400 baud (turbo):\n");
    printf("  Duration:  %s (%d seconds)\n", dur_2400_str, (int)ceil(duration_2400));
    
    size_t wav_size_2400 = calculateWavFileSize(duration_2400, 43200);
    formatBytes(wav_size_2400, size_str, sizeof(size_str));
    printf("  WAV size:  %s (43200 Hz, 8-bit mono)\n", size_str);
    
    // =============================================================================
    // 3. SIZE ANALYSIS
    // =============================================================================
    
    printf("\nSize Analysis\n");
    printf("=============\n");
    
    // Calculate total data payload and header overhead
    size_t total_payload = 0;
    size_t largest_size = 0;
    size_t smallest_size = SIZE_MAX;
    const cas_File *largest_file = NULL;
    const cas_File *smallest_file = NULL;
    
    for (size_t i = 0; i < container.file_count; i++) {
        const cas_File *file = &container.files[i];
        size_t file_payload = file->data_size;
        
        total_payload += file_payload;
        
        if (file_payload > largest_size) {
            largest_size = file_payload;
            largest_file = file;
        }
        if (file_payload < smallest_size) {
            smallest_size = file_payload;
            smallest_file = file;
        }
    }
    
    size_t cas_overhead = file_size - total_payload;
    double cas_overhead_percent = (double)cas_overhead / (double)file_size * 100.0;
    
    formatBytes(total_payload, size_str, sizeof(size_str));
    printf("CAS File:\n");
    printf("  Data payload:    %s\n", size_str);
    
    formatBytes(cas_overhead, size_str, sizeof(size_str));
    printf("  CAS overhead:    %s (%.1f%%)\n", size_str, cas_overhead_percent);
    
    // Calculate audio expansion at 1200 baud (standard)
    // WAV file size includes silence + sync + framing + encoded headers
    size_t wav_size = calculateWavFileSize(duration_1200, 43200);
    double expansion_ratio = (double)wav_size / (double)total_payload;
    
    formatBytes(wav_size, size_str, sizeof(size_str));
    printf("\nWAV Audio (1200 baud, 43200 Hz):\n");
    printf("  WAV file size:   %s\n", size_str);
    
    char payload_str[64];
    formatBytes(total_payload, payload_str, sizeof(payload_str));
    printf("  Data payload:    %s\n", payload_str);
    
    printf("  Expansion ratio: %.1fx (silence + sync + framing + headers)\n", expansion_ratio);
    
    // Show largest and smallest files
    printf("\nFile Size Range:\n");
    if (largest_file) {
        formatBytes(largest_size, size_str, sizeof(size_str));
        printf("  Largest:  %s", size_str);
        if (!largest_file->is_custom) {
            printf(" (%.6s)", (char*)largest_file->file_header.file_name);
        } else {
            printf(" (Custom block)");
        }
        printf("\n");
    }
    
    if (smallest_file) {
        formatBytes(smallest_size, size_str, sizeof(size_str));
        printf("  Smallest: %s", size_str);
        if (!smallest_file->is_custom) {
            printf(" (%.6s)", (char*)smallest_file->file_header.file_name);
        } else {
            printf(" (Custom block)");
        }
        printf("\n");
    }
    
    // Cleanup
    free(container.files);
    free(file_data);
    
    return 0;
}
