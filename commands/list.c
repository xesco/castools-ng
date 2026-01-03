#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/caslib.h"
#include "../lib/printlib.h"
#include "../lib/cmdlib.h"
#include "../lib/playlib.h"

int execute_list(const char *input_file, bool extended, int filter_index, bool show_markers, bool verbose) {
    // Check if we should show WAV markers instead of CAS listing
    if (show_markers) {
        if (verbose) {
            printf("Reading WAV markers from: %s\n\n", input_file);
        }
        
        MarkerListInfo *markers = readWavMarkers(input_file);
        if (!markers || markers->count == 0) {
            fprintf(stderr, "Error: No markers found in WAV file\n");
            if (markers) {
                free(markers->markers);
                free(markers);
            }
            return 1;
        }
        
        printf("WAV File Markers (%zu total)\n", markers->count);
        for (size_t i = 0; i < markers->count; i++) {
            const MarkerInfo *m = &markers->markers[i];
            
            // Format time as MM:SS.mmm
            int minutes = (int)(m->time_seconds / 60);
            double seconds = m->time_seconds - (minutes * 60);
            
            printf("%4zu. %2d:%06.3f - %s\n", 
                   i + 1, minutes, seconds, m->description);
        }
        printf("Total markers: %zu\n", markers->count);
        
        free(markers->markers);
        free(markers);
        return 0;
    }
    
    // Original CAS listing functionality
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
        printf("Parsing CAS container...\n");
    }
    
    // Parse the CAS container
    cas_Container container;
    if (!parseCasContainer(file_data, &container, file_size)) {
        fprintf(stderr, "Error: Failed to parse CAS container\n");
        free(file_data);
        return 1;
    }
    
    if (verbose) {
        printf("Successfully parsed %zu file(s)\n\n", container.file_count);
    }
    
    // If filtering by index, show the specific file (only in extended mode)
    if (filter_index) {
        if (filter_index < 1 || (size_t)filter_index > container.file_count) {
            fprintf(stderr, "Error: Index %d out of range (1-%zu)\n", filter_index, container.file_count);
            free(container.files);
            free(file_data);
            return 1;
        }
        
        printFile(&container.files[filter_index - 1], filter_index);
        free(container.files);
        free(file_data);
        return 0;
    }
    
    // Print the list
    if (extended) {
        // Print full details with hexdumps
        printDetailedContainer(&container);
    } else {
        // Print compact list
        printCompactContainer(&container);
    }
    
    // Cleanup
    free(container.files);
    free(file_data);
    
    return 0;
}