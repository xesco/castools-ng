# Waveform Selection Guide

## Overview

The MSX cassette format uses frequency-shift keying (FSK) to encode data as audio. Different waveform shapes affect signal quality, compatibility, and file size. This guide explains when to use each waveform type.

## Waveform Types

### Square Wave (Default - Recommended)

**Characteristics:**
- Instant transitions between high and low states
- Sharp zero-crossings
- Contains all odd harmonics (3rd, 5th, 7th, etc.)
- Most closely matches real cassette tape behavior

**Advantages:**
- ✅ Most reliable on real MSX hardware
- ✅ Precise timing - MSX detects zero-crossings instantly
- ✅ Tolerates audio path imperfections (cables, sound cards)
- ✅ Less sensitive to volume variations
- ✅ Works well with computer-to-MSX playback

**Disadvantages:**
- ❌ Larger file size than sine wave (more harmonics to encode)
- ❌ Contains high-frequency content

**When to use:**
- **Computer-to-MSX playback** (primary use case)
- **Real MSX hardware** loading from WAV files
- **Consumer audio equipment** (standard sound cards, cables)
- **When reliability matters more than file size**
- **Problematic/aging hardware** (use with `conservative` profile)

**Profiles using square wave:**
- `default` - Standard choice for most users
- `turbo` - Fast loading with square wave
- `conservative` - Maximum compatibility
- `emulator` - Clean signal for emulators

---

### Trapezoid Wave

**Characteristics:**
- Gradual rise/fall edges (typically 10-15% of cycle)
- Softer transitions than square wave
- Reduced high-frequency harmonics
- More "analog-like" signal

**Advantages:**
- ✅ Gentler on audio hardware (less harsh)
- ✅ Reduced high-frequency content
- ✅ Can work better with low-pass filters
- ✅ Theoretically more tape-like

**Disadvantages:**
- ❌ Timing ambiguity during rise/fall periods
- ❌ Sensitive to signal amplitude variations
- ❌ Zero-crossing point varies with volume
- ❌ More susceptible to audio path distortion
- ❌ Can cause data corruption on marginal signals

**When to use:**
- **Experimental scenarios** where square wave has issues
- **Very specific hardware** that doesn't like sharp edges
- **Academic/research purposes**
- **NOT recommended as default** (square is more reliable)

**Profiles using trapezoid wave:**
- `turbo-safe` - Safer 2400 baud with margins
- `compact` / `compact-plus` - File size reduction
- `tape-recording` - Physical cassette creation

**Note:** User testing showed trapezoid caused graphical corruption on real hardware where square wave worked perfectly. Use with caution.

---

### Sine Wave

**Characteristics:**
- Pure tone, single frequency
- No harmonics (mathematically perfect)
- Minimal bandwidth requirement
- Smoothest possible waveform

**Advantages:**
- ✅ Smallest file size (can use minimum sample rate)
- ✅ Pure signal for analysis
- ✅ Works well in emulators
- ✅ Minimal bandwidth (good for Nyquist-limited scenarios)
- ✅ Clean oscilloscope/audio analysis

**Disadvantages:**
- ❌ Does NOT match real cassette tape behavior
- ❌ Gradual zero-crossings (timing ambiguity)
- ❌ Requires excellent hardware quality
- ❌ Less reliable on consumer audio equipment
- ❌ Not recommended for real MSX hardware

**When to use:**
- **File size critical** (emulator distribution, storage)
- **Emulators only** (openMSX, blueMSX)
- **Signal analysis** (debugging, oscilloscope)
- **Professional audio equipment** with perfect signal path
- **NOT for typical computer-to-MSX playback**

**Profiles using sine wave:**
- `compact-max` - Extreme file size reduction
- `debug` - Pure reference signal for analysis

---

### Triangle Wave

**Characteristics:**
- Linear rise/fall (50% of each half-cycle)
- Contains only odd harmonics (like square, but lower amplitude)
- Gentler than square, sharper than sine
- Unique harmonic profile

**Advantages:**
- ✅ More robust to cable degradation than sine/trapezoid
- ✅ Better noise immunity than sine
- ✅ Can survive longer cable runs
- ✅ Moderate file size
- ✅ Different harmonic content may help in noisy environments

**Disadvantages:**
- ❌ Not extensively tested on real hardware
- ❌ Linear slope still has timing ambiguity
- ❌ Less proven than square wave
- ❌ Experimental status

**When to use:**
- **Electrically noisy environments** (RF interference)
- **Long cable runs** (>3 meters)
- **Experimental troubleshooting** when square fails
- **Specific hardware quirks** that reject square wave

**Profiles using triangle wave:**
- `triangle` - Noisy environment compensation
- `long-cable` - Cable degradation compensation
- `compact-extreme` - Extreme file size + triangle
- `turbo-gentle` - Fast loading on unreliable hardware

---

## Waveform Comparison

| Waveform  | Real MSX | Emulators | File Size | Timing Precision | Cable Tolerance | Recommendation |
|-----------|----------|-----------|-----------|------------------|-----------------|----------------|
| Square    | ★★★★★    | ★★★★★     | Medium    | ★★★★★            | ★★★★★           | **Primary choice** |
| Trapezoid | ★★★☆☆    | ★★★★★     | Medium    | ★★★☆☆            | ★★★☆☆           | Experimental |
| Sine      | ★★☆☆☆    | ★★★★★     | Small     | ★★☆☆☆            | ★★☆☆☆           | Emulators only |
| Triangle  | ★★★☆☆    | ★★★★★     | Small     | ★★★☆☆            | ★★★★☆           | Special cases |

## Decision Tree

```
Are you loading on real MSX hardware from a computer?
├─ YES → Use SQUARE wave (default profile)
│   ├─ Having issues? → Try SQUARE + conservative profile
│   └─ Need faster? → Use SQUARE + turbo profile
│
└─ NO (emulator only)
    ├─ Want smallest file? → Use SINE wave (compact-max profile)
    ├─ Want fast loading? → Use SQUARE (turbo/emulator profile)
    └─ Debugging signal? → Use SINE (debug profile)

Special scenarios:
├─ Long cables (>3m) → Try TRIANGLE (long-cable profile)
├─ Noisy environment → Try TRIANGLE (triangle profile)
├─ Recording to tape → Use TRAPEZOID (tape-recording profile)
└─ File size critical + real hardware → Use TRIANGLE (compact-extreme profile)
```

## Recommendations by Use Case

### Typical User (Computer to MSX)
**Use: Square wave + default profile**
- Most reliable
- Works with consumer audio equipment
- Proven in real-world testing

### Fast Loading
**Use: Square wave + turbo profile**
- 2400 baud with square wave
- Reliable 2x speed increase
- Well-tested combination

### Problematic Hardware
**Use: Square wave + conservative profile**
- Lower amplitude (more headroom)
- Longer silence periods
- Still uses square for precise timing

### Emulator Distribution
**Use: Sine wave + compact-max profile**
- Smallest files
- Emulators handle perfectly
- No analog signal issues

### Debugging/Analysis
**Use: Sine wave + debug profile**
- Pure reference signal
- High sample rate
- Easy to analyze with tools

### Experimental/Last Resort
**Try: Triangle wave + specific profile**
- When square mysteriously fails
- Long cable runs
- Noisy electrical environments

## Technical Deep Dive

### Why Square Wave Works Best

**MSX cassette input circuit:**
1. Input signal → AC coupling capacitor
2. Amplification/automatic gain control (AGC)
3. Comparator (zero-crossing detector)
4. Edge-triggered bit decoder

**The comparator triggers on zero-crossings.** Square wave provides:
- Instant transitions = precise trigger timing
- No ambiguity about when crossing happens
- Robust against amplitude variations (comparator is binary)

**Trapezoid/sine problems:**
- Gradual slope = timing varies with amplitude
- Volume changes affect zero-crossing point
- Cable filtering changes slope shape
- AGC settling affects effective amplitude

### Harmonic Content

**Square wave:**
- Fundamental + all odd harmonics (1, 3, 5, 7...)
- 1200 Hz carrier → harmonics at 3600, 6000, 8400 Hz
- All within audio bandwidth (<20 kHz)

**Sine wave:**
- Only fundamental frequency
- Minimal bandwidth
- Smallest file size
- But doesn't match tape behavior

**Triangle wave:**
- Odd harmonics only (like square)
- But harmonics decay faster (1/n² vs 1/n)
- Middle ground between sine and square

## File Size Impact

**Same CAS file (75 KB), 1200 baud, 43200 Hz:**

| Waveform  | Sample Rate | File Size | Relative |
|-----------|-------------|-----------|----------|
| Square    | 43200 Hz    | 31.0 MB   | 100%     |
| Sine      | 24000 Hz    | 10.5 MB   | 34%      |
| Triangle  | 36000 Hz    | 15.7 MB   | 51%      |
| Trapezoid | 43200 Hz    | 31.0 MB   | 100%     |

Lower sample rates possible with sine/triangle because less harmonic content to preserve.

## Summary

**Default choice: Square wave**
- Proven reliable on real MSX hardware
- Sharp zero-crossings = precise timing
- Tolerates consumer audio equipment
- Your testing confirmed: trapezoid caused corruption, square worked perfectly

**Only use alternatives when:**
- Emulator-only distribution (sine for file size)
- Specific hardware issues (triangle for noise/cables)
- Analysis/debugging (sine for clean signal)

**Avoid trapezoid for real hardware** unless you have specific evidence it works better in your setup.
