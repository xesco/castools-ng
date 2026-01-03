/*
 * playlib.h - WAV playback and marker reading library
 * 
 * Provides functionality for:
 * - Reading WAV cue point markers
 * - Audio playback with precise position tracking
 * - Playback controls (play, pause, seek, volume)
 */

#ifndef PLAYLIB_H
#define PLAYLIB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "wavlib.h"  // For MarkerCategory, Marker, MarkerList definitions

// =============================================================================
// Marker Structures (extended with time info for playback)
// =============================================================================

typedef struct {
    uint32_t sample_position;
    double time_seconds;
    MarkerCategory category;
    char description[256];
} MarkerInfo;

typedef struct {
    MarkerInfo *markers;
    size_t count;
    uint32_t sample_rate;
    double total_duration;
} MarkerListInfo;

// =============================================================================
// Audio Player Structures
// =============================================================================

typedef enum {
    PLAYER_STOPPED,
    PLAYER_PLAYING,
    PLAYER_PAUSED,
    PLAYER_ERROR
} PlayerState;

typedef struct {
    const char *filepath;
    MarkerListInfo *markers;
    uint32_t sample_rate;
    uint32_t channels;
    uint64_t total_frames;
    double total_duration;
    PlayerState state;
    double current_position;  // Current position in seconds
    float volume;             // 0.0 to 1.0
    void *ma_decoder;         // miniaudio decoder
    void *ma_device;          // miniaudio device
} AudioPlayer;

// =============================================================================
// Marker Reading Functions
// =============================================================================

/*
 * Read markers from a WAV file
 * Returns NULL if file has no markers or on error
 * Caller must free with freeMarkerList()
 */
MarkerListInfo* readWavMarkers(const char *filename);

/*
 * Find marker at or before specific time
 * Returns NULL if no marker found
 */
const MarkerInfo* findMarkerAtTime(const MarkerListInfo *markers, double time);

/*
 * Free marker list memory
 */
void freeMarkerListInfo(MarkerListInfo *markers);

// =============================================================================
// Audio Playback Functions
// =============================================================================

/*
 * Create audio player for WAV file
 * Returns NULL on error
 */
AudioPlayer* createAudioPlayer(const char *filename);

/*
 * Start playback
 */
void playAudio(AudioPlayer *player);

/*
 * Pause playback
 */
void pauseAudio(AudioPlayer *player);

/*
 * Resume playback (after pause)
 */
void resumeAudio(AudioPlayer *player);

/*
 * Seek to specific time in seconds
 */
bool seekAudio(AudioPlayer *player, double seconds);

/*
 * Set playback volume (0.0 - 1.0)
 */
void setVolume(AudioPlayer *player, float volume);

/*
 * Get current playback position in seconds
 */
double getPlaybackPosition(AudioPlayer *player);

/*
 * Get total duration in seconds
 */
double getAudioDuration(AudioPlayer *player);

/*
 * Check if audio is currently playing
 */
bool isPlaying(AudioPlayer *player);

/*
 * Check if audio is paused
 */
bool isPaused(AudioPlayer *player);

/*
 * Destroy audio player and free resources
 */
void destroyAudioPlayer(AudioPlayer *player);

#endif // PLAYLIB_H
