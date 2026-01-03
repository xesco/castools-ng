/*
 * test_audio_playback.c - Test audio playback functionality
 */

#include "../lib/playlib.h"
#include <stdio.h>
#include <unistd.h>  // for sleep()

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <wav_file>\n", argv[0]);
        return 1;
    }
    
    printf("Creating audio player for: %s\n", argv[1]);
    AudioPlayer *player = createAudioPlayer(argv[1]);
    if (!player) {
        fprintf(stderr, "Failed to create audio player\n");
        return 1;
    }
    
    printf("\nAudio Info:\n");
    printf("  Sample Rate: %u Hz\n", player->sample_rate);
    printf("  Channels: %u\n", player->channels);
    printf("  Duration: %.2f seconds\n", player->total_duration);
    printf("  Volume: %.0f%%\n", player->volume * 100);
    
    if (player->markers) {
        printf("  Markers: %zu\n", player->markers->count);
    } else {
        printf("  Markers: none\n");
    }
    
    printf("\nStarting playback...\n");
    playAudio(player);
    
    // Play for 5 seconds
    printf("Playing for 5 seconds...\n");
    for (int i = 0; i < 50; i++) {
        usleep(100000);  // 0.1 second
        printf("\rPosition: %.2f / %.2f seconds", 
               getPlaybackPosition(player), 
               getAudioDuration(player));
        fflush(stdout);
    }
    printf("\n");
    
    // Test pause
    printf("\nPausing...\n");
    pauseAudio(player);
    sleep(2);
    
    // Test resume
    printf("Resuming...\n");
    resumeAudio(player);
    sleep(2);
    
    // Test seek
    printf("\nSeeking to 10 seconds...\n");
    if (seekAudio(player, 10.0)) {
        printf("Seek successful\n");
        sleep(2);
    }
    
    // Test volume change
    printf("\nChanging volume to 50%%...\n");
    setVolume(player, 0.5f);
    sleep(2);
    
    printf("\nStopping playback...\n");
    destroyAudioPlayer(player);
    
    printf("✓ Audio playback test complete!\n");
    return 0;
}
