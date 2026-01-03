/*
 * playlib.c - WAV playback and marker reading implementation
 */

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "playlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// WAV Chunk Structures (for reading)
// =============================================================================

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

typedef struct {
    char riff[4];
    uint32_t file_size;
    char wave[4];
} RiffHeader;

typedef struct {
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
} FmtChunkData;
#pragma pack(pop)

// =============================================================================
// Helper Functions
// =============================================================================

static MarkerCategory parseCategory(const char *description) {
    if (strncmp(description, "[STRUCTURE]", 11) == 0) {
        return MARKER_STRUCTURE;
    } else if (strncmp(description, "[DETAIL]", 8) == 0) {
        return MARKER_DETAIL;
    } else if (strncmp(description, "[VERBOSE]", 9) == 0) {
        return MARKER_VERBOSE;
    }
    return MARKER_DETAIL;  // Default
}

// =============================================================================
// Marker Reading Implementation
// =============================================================================

MarkerListInfo* readWavMarkers(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Could not open file '%s'\n", filename);
        return NULL;
    }
    
    // Read and validate RIFF header
    RiffHeader riff_hdr;
    if (fread(&riff_hdr, sizeof(RiffHeader), 1, f) != 1) {
        fprintf(stderr, "Error: Could not read RIFF header\n");
        fclose(f);
        return NULL;
    }
    
    if (memcmp(riff_hdr.riff, "RIFF", 4) != 0 || memcmp(riff_hdr.wave, "WAVE", 4) != 0) {
        fprintf(stderr, "Error: Not a valid WAV file\n");
        fclose(f);
        return NULL;
    }
    
    // Find fmt chunk to get sample rate
    uint32_t sample_rate = 0;
    double total_duration = 0.0;
    
    // Temporary storage for cue points and labels
    CuePoint *cue_points = NULL;
    uint32_t num_cues = 0;
    
    typedef struct {
        uint32_t cue_id;
        char text[256];
    } LabelEntry;
    LabelEntry *labels = NULL;
    size_t label_count = 0;
    
    // Scan chunks
    char chunk_id[4];
    uint32_t chunk_size;
    
    while (fread(chunk_id, 4, 1, f) == 1) {
        if (fread(&chunk_size, 4, 1, f) != 1) break;
        
        long chunk_data_start = ftell(f);
        
        if (memcmp(chunk_id, "fmt ", 4) == 0) {
            FmtChunkData fmt;
            size_t to_read = chunk_size < sizeof(FmtChunkData) ? chunk_size : sizeof(FmtChunkData);
            fread(&fmt, to_read, 1, f);
            sample_rate = fmt.sample_rate;
        }
        else if (memcmp(chunk_id, "data", 4) == 0 && sample_rate > 0) {
            // Calculate duration from data chunk size
            total_duration = chunk_size / (double)sample_rate;
        }
        else if (memcmp(chunk_id, "cue ", 4) == 0) {
            fread(&num_cues, 4, 1, f);
            cue_points = malloc(num_cues * sizeof(CuePoint));
            for (uint32_t i = 0; i < num_cues; i++) {
                fread(&cue_points[i], sizeof(CuePoint), 1, f);
            }
        }
        else if (memcmp(chunk_id, "LIST", 4) == 0) {
            char list_type[4];
            fread(list_type, 4, 1, f);
            
            if (memcmp(list_type, "adtl", 4) == 0) {
                long list_end = chunk_data_start + chunk_size;
                
                while (ftell(f) < list_end) {
                    LabelChunkHeader label_hdr;
                    if (fread(&label_hdr, sizeof(LabelChunkHeader), 1, f) != 1) break;
                    
                    if (memcmp(label_hdr.labl, "labl", 4) == 0) {
                        labels = realloc(labels, (label_count + 1) * sizeof(LabelEntry));
                        labels[label_count].cue_id = label_hdr.cue_id;
                        
                        size_t text_size = label_hdr.chunk_size - 4;  // -4 for cue_id
                        if (text_size > 255) text_size = 255;
                        
                        fread(labels[label_count].text, text_size, 1, f);
                        labels[label_count].text[text_size] = '\0';
                        
                        // Remove null padding
                        for (size_t i = 0; i < text_size; i++) {
                            if (labels[label_count].text[i] == '\0') {
                                labels[label_count].text[i] = '\0';
                                break;
                            }
                        }
                        
                        label_count++;
                    } else {
                        // Skip unknown chunk in LIST
                        fseek(f, label_hdr.chunk_size - 4, SEEK_CUR);
                    }
                }
            }
        }
        
        // Move to next chunk (align to even boundary if needed)
        long next_chunk = chunk_data_start + chunk_size;
        if (chunk_size % 2) next_chunk++;  // WAV chunks are word-aligned
        fseek(f, next_chunk, SEEK_SET);
    }
    
    fclose(f);
    
    // Check if we found markers
    if (num_cues == 0 || !cue_points) {
        free(cue_points);
        free(labels);
        return NULL;  // No markers found
    }
    
    // Create marker list
    MarkerListInfo *marker_list = malloc(sizeof(MarkerListInfo));
    marker_list->count = num_cues;
    marker_list->sample_rate = sample_rate;
    marker_list->total_duration = total_duration;
    marker_list->markers = malloc(num_cues * sizeof(MarkerInfo));
    
    // Merge cue points with labels
    for (uint32_t i = 0; i < num_cues; i++) {
        MarkerInfo *marker = &marker_list->markers[i];
        marker->sample_position = cue_points[i].sample_offset;
        marker->time_seconds = marker->sample_position / (double)sample_rate;
        
        // Find matching label
        const char *label_text = NULL;
        for (size_t j = 0; j < label_count; j++) {
            if (labels[j].cue_id == cue_points[i].cue_id) {
                label_text = labels[j].text;
                break;
            }
        }
        
        if (label_text) {
            strncpy(marker->description, label_text, 255);
            marker->description[255] = '\0';
            marker->category = parseCategory(label_text);
        } else {
            snprintf(marker->description, 256, "Marker %u", cue_points[i].cue_id);
            marker->category = MARKER_DETAIL;
        }
    }
    
    free(cue_points);
    free(labels);
    
    return marker_list;
}

const MarkerInfo* findMarkerAtTime(const MarkerListInfo *markers, double time) {
    if (!markers || markers->count == 0) {
        return NULL;
    }
    
    const MarkerInfo *closest = NULL;
    for (size_t i = 0; i < markers->count; i++) {
        if (markers->markers[i].time_seconds <= time) {
            closest = &markers->markers[i];
        } else {
            break;  // Markers are in chronological order
        }
    }
    
    return closest;
}

void freeMarkerListInfo(MarkerListInfo *markers) {
    if (markers) {
        free(markers->markers);
        free(markers);
    }
}

// =============================================================================
// Audio Playback (Stub - will implement next)
// =============================================================================

// =============================================================================
// Audio Playback Implementation
// =============================================================================

/*
 * Audio callback for miniaudio
 * This is called by miniaudio when it needs more audio data
 */
static void audio_data_callback(ma_device *device, void *output, const void *input, ma_uint32 frame_count) {
    AudioPlayer *player = (AudioPlayer *)device->pUserData;
    (void)input; // Unused for playback
    
    if (player->state != PLAYER_PLAYING) {
        // Output silence if not playing
        memset(output, 0, frame_count * player->channels * sizeof(float));
        return;
    }
    
    ma_decoder *decoder = (ma_decoder *)player->ma_decoder;
    
    // Read frames from decoder
    ma_uint64 frames_read;
    ma_result result = ma_decoder_read_pcm_frames(decoder, output, frame_count, &frames_read);
    
    if (result != MA_SUCCESS || frames_read < frame_count) {
        // End of file or error - fill rest with silence
        if (frames_read < frame_count) {
            size_t silence_bytes = (frame_count - frames_read) * player->channels * sizeof(float);
            memset((float *)output + (frames_read * player->channels), 0, silence_bytes);
        }
        
        // Stop playback at end of file
        if (frames_read == 0) {
            player->state = PLAYER_STOPPED;
        }
    }
    
    // Update position based on frames actually output (not decoder cursor)
    // This gives accurate real-time position matching what's heard
    if (player->state == PLAYER_PLAYING) {
        player->current_position += (double)frame_count / player->sample_rate;
    }
    
    // Apply volume
    if (player->volume != 1.0f) {
        float *samples = (float *)output;
        size_t total_samples = frames_read * player->channels;
        for (size_t i = 0; i < total_samples; i++) {
            samples[i] *= player->volume;
        }
    }
}

AudioPlayer* createAudioPlayer(const char *filename) {
    AudioPlayer *player = calloc(1, sizeof(AudioPlayer));
    if (!player) {
        fprintf(stderr, "Failed to allocate AudioPlayer\n");
        return NULL;
    }
    
    // Initialize player fields
    player->filepath = strdup(filename);
    player->state = PLAYER_STOPPED;
    player->volume = 0.8f;  // Default 80% volume
    player->current_position = 0.0;
    
    // Load markers
    player->markers = readWavMarkers(filename);
    // Note: markers may be NULL if file has no markers - this is OK
    
    // Create decoder
    ma_decoder *decoder = malloc(sizeof(ma_decoder));
    if (!decoder) {
        fprintf(stderr, "Failed to allocate decoder\n");
        free((void *)player->filepath);
        freeMarkerListInfo(player->markers);
        free(player);
        return NULL;
    }
    
    ma_decoder_config decoder_config = ma_decoder_config_init(ma_format_f32, 0, 0);
    ma_result result = ma_decoder_init_file(filename, &decoder_config, decoder);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "Failed to initialize decoder: %d\n", result);
        free(decoder);
        free((void *)player->filepath);
        freeMarkerListInfo(player->markers);
        free(player);
        return NULL;
    }
    
    player->ma_decoder = decoder;
    
    // Get audio format info
    ma_uint64 length_in_frames;
    result = ma_decoder_get_length_in_pcm_frames(decoder, &length_in_frames);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "Failed to get decoder length\n");
        ma_decoder_uninit(decoder);
        free(decoder);
        free((void *)player->filepath);
        freeMarkerListInfo(player->markers);
        free(player);
        return NULL;
    }
    
    player->sample_rate = decoder->outputSampleRate;
    player->channels = decoder->outputChannels;
    player->total_frames = length_in_frames;
    player->total_duration = (double)length_in_frames / player->sample_rate;
    
    // Create playback device
    ma_device *device = malloc(sizeof(ma_device));
    if (!device) {
        fprintf(stderr, "Failed to allocate device\n");
        ma_decoder_uninit(decoder);
        free(decoder);
        free((void *)player->filepath);
        freeMarkerListInfo(player->markers);
        free(player);
        return NULL;
    }
    
    ma_device_config device_config = ma_device_config_init(ma_device_type_playback);
    device_config.playback.format = ma_format_f32;
    device_config.playback.channels = player->channels;
    device_config.sampleRate = player->sample_rate;
    device_config.dataCallback = audio_data_callback;
    device_config.pUserData = player;
    
    result = ma_device_init(NULL, &device_config, device);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "Failed to initialize playback device: %d\n", result);
        free(device);
        ma_decoder_uninit(decoder);
        free(decoder);
        free((void *)player->filepath);
        freeMarkerListInfo(player->markers);
        free(player);
        return NULL;
    }
    
    player->ma_device = device;
    
    return player;
}

void playAudio(AudioPlayer *player) {
    if (!player || player->state == PLAYER_PLAYING) return;
    
    ma_device *device = (ma_device *)player->ma_device;
    
    // Start device if not already started
    if (!ma_device_is_started(device)) {
        ma_result result = ma_device_start(device);
        if (result != MA_SUCCESS) {
            fprintf(stderr, "Failed to start audio device: %d\n", result);
            return;
        }
    }
    
    player->state = PLAYER_PLAYING;
}

void pauseAudio(AudioPlayer *player) {
    if (!player || player->state != PLAYER_PLAYING) return;
    player->state = PLAYER_PAUSED;
}

void resumeAudio(AudioPlayer *player) {
    if (!player || player->state != PLAYER_PAUSED) return;
    player->state = PLAYER_PLAYING;
}

bool seekAudio(AudioPlayer *player, double seconds) {
    if (!player) return false;
    
    ma_decoder *decoder = (ma_decoder *)player->ma_decoder;
    ma_uint64 target_frame = (ma_uint64)(seconds * player->sample_rate);
    
    // Clamp to valid range
    if (target_frame > player->total_frames) {
        target_frame = player->total_frames;
    }
    
    ma_result result = ma_decoder_seek_to_pcm_frame(decoder, target_frame);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "Failed to seek: %d\n", result);
        return false;
    }
    
    player->current_position = seconds;
    return true;
}

void setVolume(AudioPlayer *player, float volume) {
    if (!player) return;
    
    // Clamp volume to valid range
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    
    player->volume = volume;
}

double getPlaybackPosition(AudioPlayer *player) {
    if (!player) return 0.0;
    return player->current_position;
}

double getAudioDuration(AudioPlayer *player) {
    if (!player) return 0.0;
    return player->total_duration;
}

bool isPlaying(AudioPlayer *player) {
    return player && player->state == PLAYER_PLAYING;
}

bool isPaused(AudioPlayer *player) {
    return player && player->state == PLAYER_PAUSED;
}

void destroyAudioPlayer(AudioPlayer *player) {
    if (!player) return;
    
    // Stop device
    ma_device *device = (ma_device *)player->ma_device;
    if (device) {
        if (ma_device_is_started(device)) {
            ma_device_stop(device);
        }
        ma_device_uninit(device);
        free(device);
    }
    
    // Uninit decoder
    ma_decoder *decoder = (ma_decoder *)player->ma_decoder;
    if (decoder) {
        ma_decoder_uninit(decoder);
        free(decoder);
    }
    
    // Free resources
    freeMarkerListInfo(player->markers);
    free((void *)player->filepath);
    free(player);
}
