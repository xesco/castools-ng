# Audio Enhancement Ideas for Real MSX Hardware

This document lists potential improvements to the WAV generation code to improve compatibility with real MSX cassette tape interfaces.

## Current Implementation

The code already supports:
- Multiple waveform types: sine, square, triangle, **trapezoid** (with 10% rise/fall time)
- Configurable amplitude (0-127 for 8-bit)
- Configurable sample rate (must be divisible by 1200)
- Configurable baud rates (1200 standard, 2400 turbo)

## Proposed Enhancements

### 1. Low-Pass Filtering
**Problem:** High-frequency harmonics from square/triangle waves can be distorted by cassette mechanisms and audio cables.

**Solution:** Implement a simple low-pass filter to smooth the output signal.
- **Type:** Butterworth or simple RC-style single-pole filter
- **Cutoff:** ~6000 Hz (well above 4800 Hz max signal, below tape limitations)
- **Implementation:** Post-process generated samples with a simple IIR filter

**Benefits:**
- Reduces tape saturation from harmonics
- Smoother playback on real hardware
- Less susceptible to cable/connection quality

### 2. Configurable Rise Time for Trapezoid Waves
**Problem:** Fixed 10% rise time may be too fast for some tape decks or too slow for others.

**Solution:** Make rise time configurable (e.g., 5%, 10%, 15%, 20%).
- Add parameter to `WaveformConfig` structure
- Allow user to tune for their specific hardware
- Gentler slopes = better for worn/cheap tape mechanisms
- Steeper slopes = better signal discrimination

**Benefits:**
- Adaptable to different MSX models and tape deck quality
- Can optimize for specific hardware combinations
- Helps with aging cassette interfaces

### 3. Extended Silence/Leader Options
**Problem:** Some MSX machines need longer leader times to stabilize motor speed and AGC.

**Solution:** Add configurable silence durations.
- Current: 2s for long header, 1s for short header
- Add options for: conservative (3s/2s), extended (5s/3s)
- Possibly add trailing silence option

**Benefits:**
- Better compatibility with MSX models that have slow AGC
- Helps with cassette decks that have slow motor start
- More reliable loading on marginal hardware

### 4. Phase Consistency & Zero Crossings
**Problem:** Waveforms that don't start at zero crossing can cause clicks/pops.

**Solution:** Ensure all pulses start and end at the zero crossing point.
- Force waveforms to begin at 128 (8-bit center)
- Align pulse boundaries to avoid discontinuities
- Smooth transitions between different frequencies

**Benefits:**
- Eliminates audio artifacts
- Cleaner signal for MSX to decode
- Reduces DC offset issues

### 5. DC Offset Removal
**Problem:** Accumulated DC offset can saturate tape or shift signal baseline.

**Solution:** Implement DC offset tracking and removal.
- Monitor running average of signal
- Periodically re-center to 128 (8-bit) or 0 (16-bit signed)
- Add option for strict centering vs. natural waveform

**Benefits:**
- Prevents tape saturation
- More consistent signal levels
- Better for long recordings

### 6. Adaptive Amplitude (AGC Simulation)
**Problem:** Fixed amplitude may be too loud (distortion) or too quiet (noise issues).

**Solution:** Add option for adaptive amplitude adjustment.
- Start with lower amplitude, gradually increase
- Helps MSX AGC circuits stabilize
- Option to ramp amplitude during leader

**Benefits:**
- Better compatibility with MSX AGC behavior
- Reduces initial transient issues
- Mimics real tape recorder behavior

### 7. Pre-emphasis/De-emphasis
**Problem:** Tape has frequency-dependent losses (high frequencies attenuate more).

**Solution:** Add optional pre-emphasis filter.
- Boost high frequencies slightly before "recording"
- Standard: 50µs or 75µs time constant
- Helps compensate for tape characteristics

**Benefits:**
- Better high-frequency preservation
- Improved signal-to-noise ratio
- More faithful to real cassette behavior

## Implementation Priority

**High Priority (most impact for real hardware):**
1. Configurable rise time for trapezoid waves (easiest, most flexible)
2. Extended silence/leader options (simple, helps compatibility)
3. Low-pass filtering (moderate effort, significant quality improvement)

**Medium Priority:**
4. Phase consistency & zero crossings (quality improvement)
5. DC offset removal (preventive measure)

**Low Priority (nice to have):**
6. Adaptive amplitude
7. Pre-emphasis (complex, may not be needed if low-pass is good)

## Testing Recommendations

When implementing these features:
1. Test with the existing `head_test.wav` file
2. Verify waveforms with audio analysis tools (Audacity, etc.)
3. Test on real MSX hardware with various tape deck qualities
4. Compare loading success rates with original mcp-generated WAVs
5. Document which settings work best for different MSX models

## Notes

- All enhancements should be **optional** with sensible defaults
- Maintain backward compatibility with existing command-line interface
- Consider adding a "preset" system (e.g., `--preset conservative`, `--preset turbo`)
- Document the trade-offs of each option in user-facing help
