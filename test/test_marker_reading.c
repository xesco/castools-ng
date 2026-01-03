/*
 * Test program for marker reading functionality
 */

#include <stdio.h>
#include <stdlib.h>
#include "../lib/playlib.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file.wav>\n", argv[0]);
        return 1;
    }
    
    const char *filename = argv[1];
    
    printf("Reading markers from: %s\n\n", filename);
    
    MarkerListInfo *markers = readWavMarkers(filename);
    
    if (!markers) {
        printf("No markers found in file (or error reading file)\n");
        return 1;
    }
    
    printf("WAV File Markers\n");
    printf("================================================================================\n");
    printf("Sample Rate: %u Hz\n", markers->sample_rate);
    printf("Duration: %.2f seconds\n", markers->total_duration);
    printf("Total Markers: %zu\n\n", markers->count);
    
    printf("%-10s  %-12s  %s\n", "Time", "Category", "Description");
    printf("--------------------------------------------------------------------------------\n");
    
    for (size_t i = 0; i < markers->count; i++) {
        const MarkerInfo *m = &markers->markers[i];
        const char *category;
        
        switch (m->category) {
            case MARKER_STRUCTURE: category = "STRUCTURE"; break;
            case MARKER_DETAIL: category = "DETAIL"; break;
            case MARKER_VERBOSE: category = "VERBOSE"; break;
            default: category = "UNKNOWN"; break;
        }
        
        printf("%8.3fs  %-12s  %s\n", m->time_seconds, category, m->description);
    }
    
    printf("\n");
    
    // Test findMarkerAtTime
    printf("Testing findMarkerAtTime:\n");
    printf("-------------------------\n");
    
    double test_times[] = {0.0, 5.0, 15.0, 100.0};
    for (size_t i = 0; i < sizeof(test_times) / sizeof(test_times[0]); i++) {
        const MarkerInfo *marker = findMarkerAtTime(markers, test_times[i]);
        if (marker) {
            printf("  At %.1fs: Found marker at %.3fs - %s\n", 
                   test_times[i], marker->time_seconds, marker->description);
        } else {
            printf("  At %.1fs: No marker found\n", test_times[i]);
        }
    }
    
    free(markers->markers);
    free(markers);
    
    printf("\n✓ Marker reading test complete!\n");
    
    return 0;
}
