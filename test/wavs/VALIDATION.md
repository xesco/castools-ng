# Profile Changes Validation Results

## Comparison: Before vs After

| Profile | Change Made | Peak Before | Peak After | RMS Before | RMS After | Status |
|---------|-------------|-------------|------------|------------|-----------|--------|
| **emulator** | Amp 127→120 | -0.07 dB | -0.56 dB | -0.15 dB | -0.64 dB | ✅ FIXED (no longer clips) |
| **tape-recording** | Amp 105→100 | -1.72 dB | -2.14 dB | -2.72 dB | -3.15 dB | ✅ IMPROVED (safer for tape) |
| **conservative** | Fixed bug | CRASHED | -2.06 dB | CRASHED | -5.77 dB | ✅ FIXED (now works!) |
| **triangle** | Amp 125→120 | -1.32 dB | -1.72 dB | -5.89 dB | -6.26 dB | ✅ IMPROVED (more headroom) |
| **long-cable** | Amp 127→120 | -1.32 dB | -1.80 dB | -5.89 dB | -6.37 dB | ✅ IMPROVED (more headroom) |
| **ultra-gentle** | Amp 100→105 | -3.35 dB | -2.96 dB | -7.76 dB | -7.33 dB | ✅ IMPROVED (louder, but still gentle) |
| **turbo-gentle** | Amp 118→122 | -2.77 dB | -2.50 dB | -7.62 dB | -7.31 dB | ✅ IMPROVED (louder) |
| **default** | No amp change | -0.85 dB | -0.85 dB | -4.35 dB | -4.35 dB | ✅ STABLE (as expected) |
| **turbo** | No amp change | -1.40 dB | -1.40 dB | -5.44 dB | -5.44 dB | ✅ STABLE (as expected) |
| **debug** | No amp change | -0.56 dB | -0.56 dB | -3.66 dB | -3.66 dB | ✅ STABLE (as expected) |

## New Profiles Analysis

| Profile | Waveform | Amplitude | Peak dB | RMS dB | Mean dB | Assessment |
|---------|----------|-----------|---------|--------|---------|------------|
| **clean** | SINE | 118 | -0.93 dB | -4.42 dB | -4.4 dB | ✅ EXCELLENT (target range) |
| **vintage-tape** | SINE | 100 | -2.87 dB | -6.84 dB | -6.8 dB | ✅ GOOD (authentic low level) |
| **radio** | SINE | 115 | -1.24 dB | -4.76 dB | -4.8 dB | ✅ EXCELLENT (streaming optimized) |

## Key Improvements Verified

### 1. **Critical Fixes**
- ✅ **emulator**: Reduced peak from -0.07 dB to -0.56 dB
  - Before: Would clip on many systems
  - After: Safe 0.56 dB headroom
  
- ✅ **conservative**: Now generates files successfully
  - Before: Crashed with "Error: --rise option requires --wave trapezoid"
  - After: Works perfectly with -2.06 dB peak, -5.77 dB RMS

### 2. **Amplitude Optimizations**
- ✅ **tape-recording**: -1.72 → -2.14 dB peak (safer for physical tape)
- ✅ **triangle**: -1.32 → -1.72 dB peak (better headroom)
- ✅ **long-cable**: -1.32 → -1.80 dB peak (prevents overshoot)
- ✅ **ultra-gentle**: -7.76 → -7.33 dB RMS (louder while staying gentle)
- ✅ **turbo-gentle**: -7.62 → -7.31 dB RMS (improved signal level)

### 3. **New Profile Quality**
All three new profiles hit their target ranges:
- **clean**: -4.42 dB RMS (high quality, good headroom)
- **vintage-tape**: -6.84 dB RMS (authentic cassette level with 4kHz lowpass)
- **radio**: -4.76 dB RMS (streaming optimized, 48kHz)

## RMS Level Distribution

Current profile distribution across target ranges:

**High Level (Emulators/Digital)**: -0.5 to -2 dB
- emulator: -0.64 dB ✅

**Standard Range (Real Hardware)**: -3 to -5 dB
- debug: -3.66 dB ✅
- tape-recording: -3.15 dB ✅
- default: -4.35 dB ✅
- clean: -4.42 dB ✅
- radio: -4.76 dB ✅

**Problem Hardware Range**: -5 to -7 dB
- turbo: -5.44 dB ✅
- conservative: -5.77 dB ✅
- triangle: -6.26 dB ✅
- long-cable: -6.37 dB ✅
- vintage-tape: -6.84 dB ✅

**Ultra-Safe Range**: -7 to -8 dB
- ultra-gentle: -7.33 dB ✅
- turbo-gentle: -7.31 dB ✅

## Conclusion

**All changes validated successfully!**

- ✅ Bug fixes working (conservative now generates files)
- ✅ Amplitude adjustments hit target ranges
- ✅ No profiles clipping (-0.56 dB minimum headroom)
- ✅ New profiles complement existing lineup perfectly
- ✅ Wide range of options from -0.6 dB to -7.3 dB RMS

The profile suite now provides:
- **Safe defaults** (no clipping)
- **Wide compatibility** (from pristine to damaged hardware)
- **Appropriate levels** for each use case
- **19 total profiles** covering all scenarios
