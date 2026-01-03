#include "presetlib.h"
#include <string.h>
#include <strings.h>

// =============================================================================
// Audio Profile Database - Streamlined Essential Profiles
// =============================================================================

static const AudioProfile profiles[] = {
    // Standard Profiles
    {
        .name = "default",
        .short_desc = "Balanced default for most MSX machines",
        .use_case = "Starting point for most MSX hardware",
        .category = "Standard",
        .waveform = WAVE_SINE,
        .baud_rate = 1200,
        .sample_rate = 43200,
        .amplitude = 120,
        .trapezoid_rise_percent = 10,
        .long_silence = 2.0f,
        .short_silence = 1.0f,
        .enable_lowpass = false,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Standard MSX cassette format, works on most hardware"
    },
    {
        .name = "computer-direct",
        .short_desc = "Playing WAV from computer to MSX",
        .use_case = "Playing WAV from computer audio output directly to MSX (via cable)",
        .category = "Standard",
        .waveform = WAVE_SINE,
        .baud_rate = 1200,
        .sample_rate = 43200,
        .amplitude = 120,
        .trapezoid_rise_percent = 10,
        .long_silence = 2.0f,
        .short_silence = 1.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Low-pass filter removes computer audio artifacts for cleaner playback"
    },
    
    // Fast Loading Profiles
    {
        .name = "turbo",
        .short_desc = "Fast loading at 2400 baud",
        .use_case = "Quick loading on compatible hardware (2x speed)",
        .category = "Fast Loading",
        .waveform = WAVE_SQUARE,
        .baud_rate = 2400,
        .sample_rate = 48000,
        .amplitude = 120,
        .trapezoid_rise_percent = 10,
        .long_silence = 2.0f,
        .short_silence = 1.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 7000,
        .rationale = "2400 baud with 48kHz sample rate (20 samples/cycle), filtered square wave for compact files"
    },
    {
        .name = "turbo-safe",
        .short_desc = "Safe 2400 baud with extra margins",
        .use_case = "Fast loading with extra safety for less reliable hardware",
        .category = "Fast Loading",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 2400,
        .sample_rate = 48000,
        .amplitude = 115,
        .trapezoid_rise_percent = 12,
        .long_silence = 3.0f,
        .short_silence = 2.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 7000,
        .rationale = "2400 baud gentler waveform with extra timing margins, compact 48kHz files"
    },
    
    // Problem-Solving Profiles
    {
        .name = "conservative",
        .short_desc = "Maximum compatibility for problematic hardware",
        .use_case = "Difficult loading, aging hardware, or when standard settings fail",
        .category = "Problem-Solving",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 1200,
        .sample_rate = 43200,
        .amplitude = 105,
        .trapezoid_rise_percent = 15,
        .long_silence = 5.0f,
        .short_silence = 3.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 5500,
        .rationale = "Maximum safety: gentle slopes, long leader, filtered, lower amplitude"
    },
    {
        .name = "tape-recording",
        .short_desc = "Recording to physical cassette tape",
        .use_case = "Creating physical cassette tapes from WAV files",
        .category = "Problem-Solving",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 1200,
        .sample_rate = 43200,
        .amplitude = 105,
        .trapezoid_rise_percent = 10,
        .long_silence = 5.0f,
        .short_silence = 3.0f,
        .enable_lowpass = false,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Tape-friendly waveform, avoid over-driving, extra leader for motor/AGC"
    },
    
    // Testing/Development
    {
        .name = "emulator",
        .short_desc = "MSX emulator (openMSX, blueMSX)",
        .use_case = "Loading WAV files in MSX emulators",
        .category = "Testing/Development",
        .waveform = WAVE_SQUARE,
        .baud_rate = 2400,
        .sample_rate = 96000,
        .amplitude = 127,
        .trapezoid_rise_percent = 10,
        .long_silence = 1.0f,
        .short_silence = 0.5f,
        .enable_lowpass = false,
        .lowpass_cutoff_hz = 6000,
        .rationale = "96kHz ensures clean 2400 baud square wave (20 samples/cycle), no analog issues"
    },
    {
        .name = "debug",
        .short_desc = "Pure reference signal for analysis",
        .use_case = "Analyzing waveforms, debugging loading issues",
        .category = "Testing/Development",
        .waveform = WAVE_SINE,
        .baud_rate = 1200,
        .sample_rate = 96000,
        .amplitude = 120,
        .trapezoid_rise_percent = 10,
        .long_silence = 2.0f,
        .short_silence = 1.0f,
        .enable_lowpass = false,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Pure sine at high sample rate for clean oscilloscope/audio analysis"
    }
};

static const size_t profile_count = sizeof(profiles) / sizeof(profiles[0]);

// =============================================================================
// Profile Management Functions
// =============================================================================

size_t getProfileCount(void) {
    return profile_count;
}

const AudioProfile* getProfileByIndex(size_t index) {
    if (index >= profile_count) {
        return NULL;
    }
    return &profiles[index];
}

const AudioProfile* findProfile(const char *name) {
    if (!name) {
        return NULL;
    }
    
    for (size_t i = 0; i < profile_count; i++) {
        if (strcasecmp(profiles[i].name, name) == 0) {
            return &profiles[i];
        }
    }
    
    return NULL;
}

void applyProfile(WaveformConfig *config, const AudioProfile *profile) {
    if (!config || !profile) {
        return;
    }
    
    config->type = profile->waveform;
    config->amplitude = profile->amplitude;
    config->baud_rate = profile->baud_rate;
    config->trapezoid_rise_percent = profile->trapezoid_rise_percent;
    config->long_silence = profile->long_silence;
    config->short_silence = profile->short_silence;
    config->enable_lowpass = profile->enable_lowpass;
    config->lowpass_cutoff_hz = profile->lowpass_cutoff_hz;
}
