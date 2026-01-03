/*
 * Test program for Phase 7: WAV Cue Chunk Writing
 * 
 * Tests that markers are correctly written to WAV file as cue/adtl chunks
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../lib/wavlib.h"
#include "../lib/caslib.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input.cas> <output.wav>\n", argv[0]);
        return 1;
    }
    
    const char *cas_file = argv[1];
    const char *wav_file = argv[2];
    
    printf("Phase 7 Test: WAV Cue Chunk Writing\n");
    printf("====================================\n");
    printf("Input:  %s\n", cas_file);
    printf("Output: %s\n\n", wav_file);
    
    // Create waveform config with markers enabled
    WaveformConfig config = createDefaultWaveform();
    config.enable_markers = true;  // Enable marker generation
    
    printf("Converting with markers enabled...\n");
    
    // Convert CAS to WAV with markers
    double duration = 0.0;
    bool success = convertCasToWav(cas_file, wav_file, &config, true, &duration);
    
    if (success) {
        printf("\nConversion successful!\n");
        printf("Duration: %.2f seconds\n", duration);
        printf("\nTo view markers in Audacity:\n");
        printf("1. Open %s in Audacity\n", wav_file);
        printf("2. Look for cue point markers in the timeline\n");
        printf("3. Click on markers to see their labels\n");
        printf("4. Labels include categories: [STRUCTURE], [DETAIL], [VERBOSE]\n");
        return 0;
    } else {
        fprintf(stderr, "\nConversion failed!\n");
        return 1;
    }
}
