#include "presetlib.h"
#include <string.h>
#include <strings.h>

// =============================================================================
// Audio Profile Database - Streamlined Essential Profiles
// =============================================================================

static const AudioProfile profiles[] = {
    // Standard Profiles
    // NOTE: default profile generates byte-for-byte identical output to cas2wav reference tool
    {
        .name = "default",
        .short_desc = "Balanced default for most MSX machines",
        .use_case = "Starting point for most MSX hardware, computer-to-MSX playback",
        .category = "Standard",
        .waveform = WAVE_SINE,
        .baud_rate = 1200,
        .sample_rate = 43200,
        .amplitude = 127,
        .trapezoid_rise_percent = 0,
        .long_silence = 2.0f,
        .short_silence = 1.0f,
        .enable_lowpass = false,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Sine wave for reliable real MSX hardware playback, amplitude 127 matches cas2wav reference implementation"
    },
    
    // Fast Loading Profiles
    {
        .name = "turbo",
        .short_desc = "Fast loading at 2400 baud",
        .use_case = "Quick loading on compatible hardware (2x speed)",
        .category = "Fast Loading",
        .waveform = WAVE_SINE,
        .baud_rate = 2400,
        .sample_rate = 48000,
        .amplitude = 127,
        .trapezoid_rise_percent = 0,
        .long_silence = 2.0f,
        .short_silence = 1.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 7000,
        .rationale = "2400 baud with 48kHz sample rate (20 samples/cycle), sine wave for real hardware compatibility"
    },
    {
        .name = "turbo-safe",
        .short_desc = "Safe 2400 baud with extra margins",
        .use_case = "Fast loading with extra safety for less reliable hardware",
        .category = "Fast Loading",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 2400,
        .sample_rate = 48000,
        .amplitude = 127,
        .trapezoid_rise_percent = 12,
        .long_silence = 3.0f,
        .short_silence = 2.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 7000,
        .rationale = "2400 baud gentler waveform with extra timing margins, compact 48kHz files"
    },
    
    // Space Saving Profiles
    {
        .name = "compact",
        .short_desc = "Balanced file size reduction",
        .use_case = "Reduce file size while maintaining good compatibility",
        .category = "Space Saving",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 2400,
        .sample_rate = 36000,
        .amplitude = 127,
        .trapezoid_rise_percent = 10,
        .long_silence = 1.5f,
        .short_silence = 0.8f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 7000,
        .rationale = "2400 baud, 36kHz (15 samples/cycle), trapezoid wave for real hardware, shorter leaders"
    },
    {
        .name = "compact-plus",
        .short_desc = "Aggressive file size reduction",
        .use_case = "Minimize file size with acceptable quality",
        .category = "Space Saving",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 2400,
        .sample_rate = 28800,
        .amplitude = 127,
        .trapezoid_rise_percent = 12,
        .long_silence = 1.2f,
        .short_silence = 0.6f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 7000,
        .rationale = "2400 baud, 28.8kHz (12 samples/cycle), gentler trapezoid, minimal leaders"
    },
    {
        .name = "compact-max",
        .short_desc = "Maximum file size reduction",
        .use_case = "Smallest possible files, good hardware required",
        .category = "Space Saving",
        .waveform = WAVE_SINE,
        .baud_rate = 2400,
        .sample_rate = 24000,
        .amplitude = 127,
        .trapezoid_rise_percent = 0,
        .long_silence = 1.0f,
        .short_silence = 0.5f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 7000,
        .rationale = "2400 baud, 24kHz (10 samples/cycle at Nyquist), sine wave for cleanest signal"
    },
    {
        .name = "compact-extreme",
        .short_desc = "Extreme file size reduction",
        .use_case = "Absolute smallest files, excellent hardware required",
        .category = "Space Saving",
        .waveform = WAVE_TRIANGLE,
        .baud_rate = 2400,
        .sample_rate = 14400,
        .amplitude = 127,
        .trapezoid_rise_percent = 0,
        .long_silence = 0.8f,
        .short_silence = 0.3f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 6500,
        .rationale = "2400 baud, 14.4kHz (6 samples/cycle, 3 at 4800Hz), triangle wave, absolute minimum viable"
    },
    
    // Problem-Solving Profiles
    {
        .name = "conservative",
        .short_desc = "Maximum compatibility for problematic hardware",
        .use_case = "Difficult loading, aging hardware, or when standard settings fail",
        .category = "Problem-Solving",
        .waveform = WAVE_SINE,
        .baud_rate = 1200,
        .sample_rate = 43200,
        .amplitude = 127,
        .trapezoid_rise_percent = 0,
        .long_silence = 5.0f,
        .short_silence = 3.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 5500,
        .rationale = "Maximum safety: sine wave for best MSX hardware compatibility, long leader, filtered, amplitude 127 matches cas2wav"
    },
    {
        .name = "tape-recording",
        .short_desc = "Recording to physical cassette tape",
        .use_case = "Creating physical cassette tapes from WAV files",
        .category = "Problem-Solving",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 1200,
        .sample_rate = 43200,
        .amplitude = 95,
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
        .trapezoid_rise_percent = 0,
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
        .amplitude = 127,
        .trapezoid_rise_percent = 0,
        .long_silence = 2.0f,
        .short_silence = 1.0f,
        .enable_lowpass = false,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Pure sine at high sample rate for clean oscilloscope/audio analysis"
    },
    
    // Experimental Profiles
    {
        .name = "triangle",
        .short_desc = "Triangle wave for noisy environments",
        .use_case = "Electrically noisy environments, RF interference",
        .category = "Experimental",
        .waveform = WAVE_TRIANGLE,
        .baud_rate = 1200,
        .sample_rate = 43200,
        .amplitude = 127,
        .trapezoid_rise_percent = 0,
        .long_silence = 3.0f,
        .short_silence = 2.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 5500,
        .rationale = "Triangle wave has unique harmonic content, filtered for noise immunity"
    },
    {
        .name = "long-cable",
        .short_desc = "Long audio cable compensation",
        .use_case = "Cable runs >3 meters, signal degradation",
        .category = "Experimental",
        .waveform = WAVE_TRIANGLE,
        .baud_rate = 1200,
        .sample_rate = 43200,
        .amplitude = 127,
        .trapezoid_rise_percent = 0,
        .long_silence = 3.0f,
        .short_silence = 2.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 5000,
        .rationale = "Triangle wave survives cable degradation, aggressive low-pass for HF loss"
    },
    {
        .name = "ultra-gentle",
        .short_desc = "Extremely damaged/worn hardware",
        .use_case = "Failing capacitors, severe drift, very poor AGC",
        .category = "Experimental",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 1200,
        .sample_rate = 43200,
        .amplitude = 127,
        .trapezoid_rise_percent = 25,
        .long_silence = 8.0f,
        .short_silence = 5.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 5000,
        .rationale = "Ultra-gentle 25% rise, moderate amplitude, extreme leader times for severely damaged circuits"
    },
    {
        .name = "turbo-gentle",
        .short_desc = "Fast loading on unreliable hardware",
        .use_case = "Need 2400 baud speed but hardware is questionable",
        .category = "Experimental",
        .waveform = WAVE_TRIANGLE,
        .baud_rate = 2400,
        .sample_rate = 48000,
        .amplitude = 122,
        .trapezoid_rise_percent = 0,
        .long_silence = 4.0f,
        .short_silence = 2.5f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 6500,
        .rationale = "Triangle wave at 2400 baud with extreme safety margins, filtered for reliability"
    },
    {
        .name = "turbo-3600",
        .short_desc = "3600 baud (3x speed) - experimental",
        .use_case = "3x speed experiment, may work on some real MSX hardware",
        .category = "Experimental",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 3600,
        .sample_rate = 36000,
        .amplitude = 127,
        .trapezoid_rise_percent = 10,
        .long_silence = 1.0f,
        .short_silence = 0.5f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 8000,
        .rationale = "3600 baud, trapezoid for hardware, filtered, pushes MSX bandwidth limits"
    },
    
    // Quality Profiles
    {
        .name = "clean",
        .short_desc = "Maximum signal quality for real hardware",
        .use_case = "High-quality playback on good MSX hardware",
        .category = "Quality",
        .waveform = WAVE_SINE,
        .baud_rate = 1200,
        .sample_rate = 43200,
        .amplitude = 127,
        .trapezoid_rise_percent = 0,
        .long_silence = 2.0f,
        .short_silence = 1.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 6500,
        .rationale = "Optimized sine wave for best signal quality, moderate levels for headroom (-3.5 dB target)"
    },
    {
        .name = "vintage-tape",
        .short_desc = "Emulate authentic 1980s cassette sound",
        .use_case = "Authentic cassette experience, nostalgia, tape-like quality",
        .category = "Quality",
        .waveform = WAVE_SINE,
        .baud_rate = 1200,
        .sample_rate = 24000,
        .amplitude = 100,
        .trapezoid_rise_percent = 0,
        .long_silence = 5.0f,
        .short_silence = 3.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 4000,
        .rationale = "24kHz sample rate with 4kHz lowpass for authentic tape bandwidth, moderate amplitude"
    },
    {
        .name = "radio",
        .short_desc = "Optimized for streaming/broadcast",
        .use_case = "Internet radio, streaming, audio sharing platforms",
        .category = "Quality",
        .waveform = WAVE_SINE,
        .baud_rate = 1200,
        .sample_rate = 48000,
        .amplitude = 115,
        .trapezoid_rise_percent = 0,
        .long_silence = 1.5f,
        .short_silence = 0.8f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 5500,
        .rationale = "48kHz high-quality audio rate, optimized levels for streaming, shorter silences"
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
