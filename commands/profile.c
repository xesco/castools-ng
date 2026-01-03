#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "../lib/presetlib.h"

// Helper to get waveform name
static const char* getWaveformName(WaveformType type) {
    switch (type) {
        case WAVE_SINE: return "sine";
        case WAVE_SQUARE: return "square";
        case WAVE_TRIANGLE: return "triangle";
        case WAVE_TRAPEZOID: return "trapezoid";
        default: return "unknown";
    }
}

int execute_profile(const char *profile_name, bool verbose) {
    
    // If no profile name specified, list all profiles
    if (!profile_name) {
        printf("Available Audio Profiles (%zu total)\n", getProfileCount());
        printf("=====================================\n\n");
        
        const char *current_category = NULL;
        
        for (size_t i = 0; i < getProfileCount(); i++) {
            const AudioProfile *profile = getProfileByIndex(i);
            
            // Print category header when it changes
            if (!current_category || strcmp(current_category, profile->category) != 0) {
                if (current_category) {
                    printf("\n");  // Blank line between categories
                }
                printf("%s:\n", profile->category);
                current_category = profile->category;
            }
            
            printf("  %-20s  %s\n", profile->name, profile->short_desc);
        }
        
        printf("\nUse 'cast profile <name>' to see detailed information about a profile.\n");
        return 0;
    }
    
    // Find specific profile
    const AudioProfile *profile = findProfile(profile_name);
    if (!profile) {
        fprintf(stderr, "Error: Profile '%s' not found\n\n", profile_name);
        fprintf(stderr, "Use 'cast profile' to list all available profiles.\n");
        return 1;
    }
    
    // Display detailed profile information
    printf("Profile: %s\n", profile->name);
    printf("========================================\n\n");
    
    printf("Category:    %s\n", profile->category);
    printf("Description: %s\n\n", profile->short_desc);
    
    printf("Use Case:\n");
    printf("  %s\n\n", profile->use_case);
    
    printf("Audio Settings:\n");
    printf("  Waveform:      %s", getWaveformName(profile->waveform));
    if (profile->waveform == WAVE_TRAPEZOID) {
        printf(" (%u%% rise)", profile->trapezoid_rise_percent);
    }
    printf("\n");
    printf("  Baud rate:     %u baud\n", profile->baud_rate);
    printf("  Amplitude:     %u\n", profile->amplitude);
    printf("  Leader timing: %.1fs / %.1fs (long/short)\n", 
           profile->long_silence, profile->short_silence);
    printf("  Low-pass:      %s", profile->enable_lowpass ? "enabled" : "disabled");
    if (profile->enable_lowpass) {
        printf(" (%u Hz)", profile->lowpass_cutoff_hz);
    }
    printf("\n\n");
    
    printf("Rationale:\n");
    printf("  %s\n\n", profile->rationale);
    
    if (verbose) {
        printf("Command Example:\n");
        printf("  cast convert input.cas output.wav --preset %s\n", profile->name);
    }
    
    return 0;
}
