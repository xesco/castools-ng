/*
 * Utility to read and display WAV cue point markers
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
    char cue[4];
    uint32_t chunk_size;
    uint32_t num_cues;
} CueChunkHeader;

typedef struct {
    uint32_t cue_id;
    uint32_t position;
    char chunk_id[4];
    uint32_t chunk_start;
    uint32_t block_start;
    uint32_t sample_offset;
} CuePoint;

typedef struct {
    char list[4];
    uint32_t chunk_size;
    char type[4];
} ListChunkHeader;

typedef struct {
    char labl[4];
    uint32_t chunk_size;
    uint32_t cue_id;
} LabelChunkHeader;
#pragma pack(pop)

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file.wav>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        perror("Error opening file");
        return 1;
    }

    // Skip to end of data chunk - search for cue chunk
    char chunk_id[4];
    uint32_t chunk_size;
    CuePoint *cue_points = NULL;
    uint32_t num_cues = 0;
    
    while (fread(chunk_id, 4, 1, f) == 1) {
        fread(&chunk_size, 4, 1, f);
        
        if (memcmp(chunk_id, "cue ", 4) == 0) {
            printf("Found cue chunk (size: %u bytes)\n", chunk_size);
            fread(&num_cues, 4, 1, f);
            printf("Number of cue points: %u\n\n", num_cues);
            
            cue_points = malloc(num_cues * sizeof(CuePoint));
            for (uint32_t i = 0; i < num_cues; i++) {
                fread(&cue_points[i], 24, 1, f);
                printf("Cue %u: sample %u\n", cue_points[i].cue_id, cue_points[i].sample_offset);
            }
            printf("\n");
        }
        else if (memcmp(chunk_id, "LIST", 4) == 0) {
            long list_start = ftell(f);
            char type[4];
            fread(type, 4, 1, f);
            
            if (memcmp(type, "adtl", 4) == 0) {
                printf("Found adtl chunk with labels:\n");
                printf("================================\n\n");
                
                long list_end = list_start + chunk_size;
                while (ftell(f) < list_end) {
                    LabelChunkHeader label_hdr;
                    if (fread(&label_hdr, 12, 1, f) != 1) break;
                    
                    if (memcmp(label_hdr.labl, "labl", 4) == 0) {
                        char *text = malloc(label_hdr.chunk_size);
                        fread(text, label_hdr.chunk_size - 4, 1, f);
                        
                        // Find corresponding sample offset
                        uint32_t sample = 0;
                        for (uint32_t i = 0; i < num_cues; i++) {
                            if (cue_points[i].cue_id == label_hdr.cue_id) {
                                sample = cue_points[i].sample_offset;
                                break;
                            }
                        }
                        
                        double time_sec = sample / 43200.0;  // 43200 Hz sample rate
                        printf("Marker %u at %.2fs (sample %u):\n  %s\n\n", 
                               label_hdr.cue_id, time_sec, sample, text);
                        free(text);
                    } else {
                        // Unknown chunk in LIST, skip it
                        fseek(f, label_hdr.chunk_size - 4, SEEK_CUR);
                    }
                }
            } else {
                fseek(f, chunk_size - 4, SEEK_CUR);
            }
        }
        else {
            // Skip unknown chunk
            fseek(f, chunk_size, SEEK_CUR);
        }
    }

    if (cue_points) free(cue_points);
    fclose(f);
    
    if (num_cues == 0) {
        printf("No cue markers found in file.\n");
    }
    
    return 0;
}
