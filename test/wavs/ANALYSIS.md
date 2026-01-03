# Audio Profile Analysis Results

## Test Setup
- Source: RUR1.cas (46.5 KB, 5 files)
- Analysis Tool: FFmpeg volumedetect + astats
- Date: 2026-01-03

## Key Findings

### 1. **CRITICAL BUG FOUND**
- **conservative** profile fails with "Error: --rise option requires --wave trapezoid"
- Problem: Profile sets `trapezoid_rise_percent` but uses `WAVE_SINE`
- **FIX NEEDED**: Remove or zero out trapezoid_rise_percent for non-trapezoid waveforms

### 2. **Amplitude Levels Analysis**

| Profile | Waveform | Amplitude | Peak dB | RMS dB | Mean dB | File Size |
|---------|----------|-----------|---------|--------|---------|-----------|
| **emulator** | SQUARE | 127 | -0.07 | -0.15 | -0.2 | 21M |
| **tape-recording** | TRAPEZOID | 105 | -1.72 | -2.72 | -2.7 | 20M |
| **debug** | SINE | 120 | -0.56 | -3.66 | -3.7 | 43M |
| **compact** | TRAPEZOID | 120 | -0.63 | -3.71 | -3.7 | 7.8M |
| **compact-plus** | TRAPEZOID | 118 | -0.85 | -4.12 | -4.1 | 6.4M |
| **default** | SINE | 120 | -0.85 | -4.35 | -4.4 | 19M |
| **turbo-safe** | TRAPEZOID | 115 | -1.01 | -4.20 | -4.2 | 11M |
| **turbo-3600** | TRAPEZOID | 125 | -0.56 | -4.49 | -4.5 | 5.4M |
| **turbo** | SINE | 120 | -1.40 | -5.44 | -5.4 | 11M |
| **compact-max** | SINE | 120 | -1.72 | -5.81 | -5.8 | 5.3M |
| **triangle** | TRIANGLE | 125 | -1.32 | -5.89 | -5.9 | 20M |
| **long-cable** | TRIANGLE | 127 | -1.32 | -5.89 | -5.9 | 20M |
| **compact-extreme** | TRIANGLE | 120 | -2.77 | -7.15 | -7.2 | 3.2M |
| **turbo-gentle** | TRIANGLE | 118 | -2.77 | -7.62 | -7.6 | 11M |
| **ultra-gentle** | TRAPEZOID | 100 | -3.35 | -7.76 | -7.8 | 21M |

### 3. **Observations**

#### Too Hot (May Clip/Distort):
- **emulator**: -0.07 dB peak - TOO HOT, will clip on some systems
- **tape-recording**: -1.72 dB - borderline, may distort

#### Well Balanced (-3 to -6 dB range):
- **default**: -4.4 dB - Good
- **compact**: -3.7 dB - Good
- **turbo**: -5.4 dB - Good
- **debug**: -3.7 dB - Good

#### Too Quiet (-7+ dB):
- **ultra-gentle**: -7.8 dB - May be too quiet for noisy environments
- **turbo-gentle**: -7.6 dB - May be too quiet
- **compact-extreme**: -7.2 dB - Acceptable for extreme compression

#### Waveform Effectiveness:
- **SINE**: Clean, predictable levels (-3.7 to -5.8 dB)
- **TRAPEZOID**: Variable (-2.7 to -7.8 dB), depends heavily on rise%
- **TRIANGLE**: Consistently lower (-5.9 to -7.6 dB)
- **SQUARE**: Hottest (-0.2 dB with amp 127)

### 4. **File Size Comparison**

**Most Efficient** (Duration vs Size):
- compact-extreme: 3:51 → 3.2M (0.83 MB/min)
- compact-max: 3:53 → 5.3M (1.36 MB/min)
- turbo-3600: 2:37 → 5.4M (2.06 MB/min)
- compact-plus: 3:54 → 6.4M (1.64 MB/min)

**Largest**:
- debug: 7:46 → 43M (5.54 MB/min) - 96kHz for analysis
- emulator: 3:53 → 21M (5.40 MB/min) - 96kHz for perfect emulation

## Recommendations

### Immediate Fixes:

1. **FIX BUG: conservative profile**
   - Remove `trapezoid_rise_percent = 15` or set to 0
   - SINE waves don't use rise time

2. **REDUCE emulator amplitude**
   - Current: 127 → Peak: -0.07 dB (WILL CLIP!)
   - Recommend: 120 → Should give ~-1.5 dB headroom

3. **INCREASE ultra-gentle amplitude**
   - Current: 100 → Peak: -3.35 dB, RMS: -7.8 dB
   - Recommend: 105-108 → Target -6 dB RMS
   - Rationale: Too quiet defeats purpose on noisy hardware

4. **REDUCE tape-recording amplitude**
   - Current: 105 → Peak: -1.72 dB (borderline hot)
   - Recommend: 100 → Target -2.5 dB for tape headroom

### New Profile Suggestions:

1. **"vintage-tape"** - Emulate authentic 1980s cassette sound
   - SINE wave, amplitude 100
   - Sample rate: 22050 Hz (authentic cassette bandwidth)
   - Enable lowpass at 4000 Hz (tape bandwidth limit)
   - Long silence for motor startup
   - Target: -5 dB RMS

2. **"clean"** - Maximum signal quality for real hardware
   - SINE wave, amplitude 118
   - Sample rate: 43200 Hz
   - Enable lowpass at 6500 Hz
   - Moderate silences
   - Target: -3.5 dB RMS

3. **"radio"** - Optimized for air transmission/streaming
   - SINE wave, amplitude 115
   - Sample rate: 44100 Hz (standard audio)
   - Enable lowpass at 5500 Hz
   - Shorter silences
   - Target: -4.5 dB RMS

### Profile Improvements:

1. **long-cable**: Reduce from 127 → 120 amplitude
   - Currently hitting -1.32 dB, should have more headroom
   - Long cables may cause overshoot

2. **triangle**: Reduce from 125 → 120 amplitude
   - Same reasoning as long-cable

3. **turbo-gentle**: Increase from 118 → 122 amplitude
   - Currently -7.6 dB RMS is too quiet
   - Triangle waves need higher amplitude

## Target RMS Levels by Use Case:

- **Emulators**: -0.5 to -2 dB (digital, no analog issues)
- **Standard Hardware**: -3 to -5 dB (good signal, headroom for variation)
- **Problem Hardware**: -5 to -7 dB (extra headroom, lower amplitude safer)
- **Physical Tape**: -4 to -6 dB (avoid tape saturation)
- **Extreme Compression**: -6 to -8 dB (low sample rate artifacts)

## Waveform Recommendations by Use Case:

- **Real MSX Hardware (Standard)**: SINE (proven to work, matches cas2wav)
- **Real MSX Hardware (Fast)**: SINE or TRAPEZOID
- **Emulators**: SQUARE (perfect digital timing)
- **Noisy Environments**: TRIANGLE (unique harmonics, noise immunity)
- **Maximum Compression**: SINE (cleanest at low sample rates)
- **Tape Recording**: TRAPEZOID (gradual transitions, tape-friendly)
