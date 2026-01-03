#include "presetlib.h"
#include <string.h>
#include <strings.h>

// =============================================================================
// Audio Profile Database
// =============================================================================

static const AudioProfile profiles[] = {
    // General Purpose Profiles
    {
        .name = "default",
        .short_desc = "Balanced default settings for most MSX machines",
        .use_case = "Starting point for most MSX machines",
        .category = "General Purpose",
        .waveform = WAVE_SINE,
        .baud_rate = 1200,
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
        .short_desc = "Playing WAV from computer directly to MSX",
        .use_case = "Playing WAV from computer directly to MSX (via audio cable)",
        .category = "General Purpose",
        .waveform = WAVE_SINE,
        .baud_rate = 1200,
        .amplitude = 120,
        .trapezoid_rise_percent = 10,
        .long_silence = 2.0f,
        .short_silence = 1.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Low-pass filter reduces computer audio artifacts, cleaner signal"
    },
    
    // MSX Hardware Generation Profiles
    {
        .name = "msx1",
        .short_desc = "Older MSX1 machines with slower AGC circuits",
        .use_case = "Older MSX1 machines (1983-1985), slower AGC circuits",
        .category = "MSX Hardware",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 1200,
        .amplitude = 110,
        .trapezoid_rise_percent = 10,
        .long_silence = 3.0f,
        .short_silence = 2.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Gentler waveform, more AGC stabilization time, lower amplitude"
    },
    {
        .name = "msx2",
        .short_desc = "Newer MSX2/2+/Turbo R machines",
        .use_case = "Newer MSX machines with better tape interfaces",
        .category = "MSX Hardware",
        .waveform = WAVE_SINE,
        .baud_rate = 1200,
        .amplitude = 120,
        .trapezoid_rise_percent = 10,
        .long_silence = 2.0f,
        .short_silence = 1.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Modern hardware can handle cleaner signals"
    },
    
    // Speed-Optimized Profiles
    {
        .name = "turbo",
        .short_desc = "Fast loading at 2400 baud",
        .use_case = "Quick loading on compatible hardware",
        .category = "Speed-Optimized",
        .waveform = WAVE_SQUARE,
        .baud_rate = 2400,
        .amplitude = 120,
        .trapezoid_rise_percent = 10,
        .long_silence = 2.0f,
        .short_silence = 1.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 7000,
        .rationale = "Fast loading with filter to clean up square wave harmonics"
    },
    {
        .name = "turbo-safe",
        .short_desc = "Fast loading with extra safety margins",
        .use_case = "2400 baud with extra safety margins",
        .category = "Speed-Optimized",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 2400,
        .amplitude = 115,
        .trapezoid_rise_percent = 10,
        .long_silence = 3.0f,
        .short_silence = 2.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 7000,
        .rationale = "Speed with reliability, gentler waveform at high baud"
    },
    
    // Problem-Solving Profiles
    {
        .name = "maximum-compatibility",
        .short_desc = "When nothing else works - maximum safety margins",
        .use_case = "Difficult-to-load files, problematic MSX hardware",
        .category = "Problem-Solving",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 1200,
        .amplitude = 100,
        .trapezoid_rise_percent = 20,
        .long_silence = 5.0f,
        .short_silence = 3.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 5500,
        .rationale = "Maximum safety margins, gentle slopes, long stabilization time"
    },
    {
        .name = "worn-equipment",
        .short_desc = "Aging or problematic MSX hardware",
        .use_case = "Worn MSX tape interfaces, noisy audio connections",
        .category = "Problem-Solving",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 1200,
        .amplitude = 105,
        .trapezoid_rise_percent = 15,
        .long_silence = 3.0f,
        .short_silence = 2.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 5500,
        .rationale = "Gentle waveform, filtered, lower amplitude, more time for AGC"
    },
    {
        .name = "clean-sharp",
        .short_desc = "Best signal discrimination for good hardware",
        .use_case = "Good hardware, need best signal clarity",
        .category = "Problem-Solving",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 1200,
        .amplitude = 120,
        .trapezoid_rise_percent = 5,
        .long_silence = 2.0f,
        .short_silence = 1.0f,
        .enable_lowpass = false,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Sharp transitions for clear 0/1 bit distinction"
    },
    
    // Tape Recording Profiles
    {
        .name = "tape-recording",
        .short_desc = "Recording to real cassette tape",
        .use_case = "Creating physical cassette tapes from WAV files",
        .category = "Tape Recording",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 1200,
        .amplitude = 110,
        .trapezoid_rise_percent = 10,
        .long_silence = 3.0f,
        .short_silence = 2.0f,
        .enable_lowpass = false,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Tape-friendly waveform, avoid over-driving tape, extra leader time"
    },
    {
        .name = "tape-dubbing",
        .short_desc = "Creating high-quality master tapes",
        .use_case = "Creating high-quality master tapes for dubbing",
        .category = "Tape Recording",
        .waveform = WAVE_SINE,
        .baud_rate = 1200,
        .amplitude = 105,
        .trapezoid_rise_percent = 10,
        .long_silence = 5.0f,
        .short_silence = 3.0f,
        .enable_lowpass = false,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Clean signal, extra leader for tape deck motor start"
    },
    
    // Testing/Development Profiles
    {
        .name = "emulator",
        .short_desc = "MSX emulator use (openMSX, blueMSX)",
        .use_case = "Loading WAV files in emulators (openMSX, blueMSX)",
        .category = "Testing/Development",
        .waveform = WAVE_SQUARE,
        .baud_rate = 2400,
        .amplitude = 127,
        .trapezoid_rise_percent = 10,
        .long_silence = 2.0f,
        .short_silence = 1.0f,
        .enable_lowpass = false,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Emulators don't have analog issues, can use ideal settings"
    },
    {
        .name = "debug",
        .short_desc = "Testing and analysis reference",
        .use_case = "Analyzing waveforms, debugging loading issues",
        .category = "Testing/Development",
        .waveform = WAVE_SINE,
        .baud_rate = 1200,
        .amplitude = 120,
        .trapezoid_rise_percent = 10,
        .long_silence = 2.0f,
        .short_silence = 1.0f,
        .enable_lowpass = false,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Pure reference waveform for analysis"
    },
    
    // Brand-Specific Hardware Profiles
    {
        .name = "sony-msx",
        .short_desc = "Sony HitBit series (sensitive tape inputs)",
        .use_case = "Sony HitBit MSX machines (known for sensitive tape inputs)",
        .category = "Brand-Specific",
        .waveform = WAVE_SINE,
        .baud_rate = 1200,
        .amplitude = 105,
        .trapezoid_rise_percent = 10,
        .long_silence = 3.0f,
        .short_silence = 2.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 5500,
        .rationale = "Conservative settings for Sony's particular tape circuit design"
    },
    {
        .name = "panasonic-msx",
        .short_desc = "Panasonic FS-A1 series",
        .use_case = "Panasonic MSX2/2+ machines",
        .category = "Brand-Specific",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 1200,
        .amplitude = 115,
        .trapezoid_rise_percent = 10,
        .long_silence = 2.0f,
        .short_silence = 1.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Panasonic tape interfaces are generally robust but benefit from filtered trapezoid"
    },
    {
        .name = "philips-msx",
        .short_desc = "Philips VG/NMS series",
        .use_case = "Philips MSX machines",
        .category = "Brand-Specific",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 1200,
        .amplitude = 110,
        .trapezoid_rise_percent = 15,
        .long_silence = 3.0f,
        .short_silence = 2.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Philips machines sometimes have slower AGC, gentler waveform helps"
    },
    
    // Experimental Profiles
    {
        .name = "ultra-clean",
        .short_desc = "Maximum signal purity experiment",
        .use_case = "Testing hypothesis that cleaner = better",
        .category = "Experimental",
        .waveform = WAVE_SINE,
        .baud_rate = 1200,
        .amplitude = 90,
        .trapezoid_rise_percent = 10,
        .long_silence = 5.0f,
        .short_silence = 3.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 5000,
        .rationale = "Extreme filtering and low amplitude, minimal harmonics"
    },
    {
        .name = "ultra-sharp",
        .short_desc = "Maximum signal clarity experiment",
        .use_case = "When bit transitions need to be crystal clear",
        .category = "Experimental",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 1200,
        .amplitude = 127,
        .trapezoid_rise_percent = 5,
        .long_silence = 2.0f,
        .short_silence = 1.0f,
        .enable_lowpass = false,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Sharp transitions, maximum amplitude for clear bit detection"
    },
    {
        .name = "high-amplitude",
        .short_desc = "Loud signal for weak/noisy connections",
        .use_case = "Weak/noisy connections, long cables",
        .category = "Experimental",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 1200,
        .amplitude = 127,
        .trapezoid_rise_percent = 15,
        .long_silence = 3.0f,
        .short_silence = 2.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Drive through noise with high amplitude, filter out distortion"
    },
    {
        .name = "triangle-test",
        .short_desc = "Triangle wave experiment",
        .use_case = "Testing linear ramp waveforms",
        .category = "Experimental",
        .waveform = WAVE_TRIANGLE,
        .baud_rate = 1200,
        .amplitude = 120,
        .trapezoid_rise_percent = 10,
        .long_silence = 2.0f,
        .short_silence = 1.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Different harmonic content than sine/square, may work where others fail"
    },
    {
        .name = "extreme-gentle",
        .short_desc = "Ultra-smooth waveform for damaged hardware",
        .use_case = "Very problematic or damaged MSX tape circuits",
        .category = "Experimental",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 1200,
        .amplitude = 95,
        .trapezoid_rise_percent = 50,
        .long_silence = 5.0f,
        .short_silence = 3.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 5000,
        .rationale = "Gentlest possible slopes, maximum compatibility with damaged hardware"
    },
    {
        .name = "square-raw",
        .short_desc = "Unfiltered digital square wave",
        .use_case = "Testing if harsh square wave actually works better",
        .category = "Experimental",
        .waveform = WAVE_SQUARE,
        .baud_rate = 1200,
        .amplitude = 120,
        .trapezoid_rise_percent = 10,
        .long_silence = 2.0f,
        .short_silence = 1.0f,
        .enable_lowpass = false,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Pure digital signal, maximum harmonic content"
    },
    
    // Special Use Case Profiles
    {
        .name = "noisy-environment",
        .short_desc = "High noise immunity",
        .use_case = "Loading in electrically noisy environments",
        .category = "Special Use",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 1200,
        .amplitude = 127,
        .trapezoid_rise_percent = 10,
        .long_silence = 3.0f,
        .short_silence = 2.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 6500,
        .rationale = "High amplitude to overcome noise, filtered to reduce interference"
    },
    {
        .name = "long-cables",
        .short_desc = "Extended cable runs (>3 meters)",
        .use_case = "Computer far from MSX, long audio cables (>3 meters)",
        .category = "Special Use",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 1200,
        .amplitude = 127,
        .trapezoid_rise_percent = 15,
        .long_silence = 3.0f,
        .short_silence = 2.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 5500,
        .rationale = "High amplitude, filtered to compensate for cable loss, gentle slopes"
    },
    {
        .name = "basic-programs",
        .short_desc = "Optimized for BASIC program files",
        .use_case = "Optimized for BASIC program files",
        .category = "Special Use",
        .waveform = WAVE_SINE,
        .baud_rate = 1200,
        .amplitude = 115,
        .trapezoid_rise_percent = 10,
        .long_silence = 2.0f,
        .short_silence = 1.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 6000,
        .rationale = "BASIC files are ASCII, clean sine wave works well"
    },
    {
        .name = "binary-programs",
        .short_desc = "Binary files, games, machine code",
        .use_case = "Binary files, games, machine code programs",
        .category = "Special Use",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 1200,
        .amplitude = 120,
        .trapezoid_rise_percent = 10,
        .long_silence = 3.0f,
        .short_silence = 2.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 6000,
        .rationale = "Binary data needs reliability, conservative timing helps"
    },
    {
        .name = "multi-file",
        .short_desc = "Multiple files in CAS container",
        .use_case = "CAS files with many small files",
        .category = "Special Use",
        .waveform = WAVE_SINE,
        .baud_rate = 1200,
        .amplitude = 115,
        .trapezoid_rise_percent = 10,
        .long_silence = 3.0f,
        .short_silence = 2.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 6000,
        .rationale = "More leader time between files helps MSX track transitions"
    },
    {
        .name = "bluetooth-audio",
        .short_desc = "Bluetooth audio adapter compensation",
        .use_case = "Using Bluetooth transmitter/receiver in audio chain",
        .category = "Special Use",
        .waveform = WAVE_TRAPEZOID,
        .baud_rate = 1200,
        .amplitude = 110,
        .trapezoid_rise_percent = 15,
        .long_silence = 5.0f,
        .short_silence = 3.0f,
        .enable_lowpass = true,
        .lowpass_cutoff_hz = 5000,
        .rationale = "Bluetooth can introduce latency/artifacts, gentle conservative approach"
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
