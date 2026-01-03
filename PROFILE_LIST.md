# Audio Profile Presets for MSX Cassette Loading

This document defines pre-configured audio profiles that combine all available enhancement options for optimal MSX cassette loading across different hardware configurations and use cases.

## Profile Categories

### 1. General Purpose Profiles

#### `default` - Balanced Default
**Use case:** Starting point for most MSX machines
- **Waveform:** Sine (cleanest, most compatible)
- **Baud rate:** 1200 (standard MSX)
- **Leader timing:** Standard (2.0s/1.0s)
- **Low-pass filter:** Disabled
- **Amplitude:** 120
- **Rationale:** Standard MSX cassette format, works on most hardware

#### `computer-direct` - Computer Audio Connection
**Use case:** Playing WAV from computer directly to MSX (via audio cable)
- **Waveform:** Sine
- **Baud rate:** 1200
- **Leader timing:** Standard (2.0s/1.0s)
- **Low-pass filter:** Enabled (6000 Hz)
- **Amplitude:** 120
- **Rationale:** Low-pass filter reduces computer audio artifacts, cleaner signal

---

### 2. MSX Hardware Generation Profiles

#### `msx1` - MSX1 Compatible
**Use case:** Older MSX1 machines (1983-1985), slower AGC circuits
- **Waveform:** Trapezoid (10% rise)
- **Baud rate:** 1200
- **Leader timing:** Conservative (3.0s/2.0s)
- **Low-pass filter:** Enabled (6000 Hz)
- **Amplitude:** 110 (slightly lower to avoid distortion)
- **Rationale:** Gentler waveform, more AGC stabilization time, lower amplitude

#### `msx2` - MSX2/2+/Turbo R
**Use case:** Newer MSX machines with better tape interfaces
- **Waveform:** Sine
- **Baud rate:** 1200
- **Leader timing:** Standard (2.0s/1.0s)
- **Low-pass filter:** Enabled (6000 Hz)
- **Amplitude:** 120
- **Rationale:** Modern hardware can handle cleaner signals

---

### 3. Speed-Optimized Profiles

#### `turbo` - Fast Loading (2400 baud)
**Use case:** Quick loading on compatible hardware
- **Waveform:** Square (sharp transitions for 2400 baud)
- **Baud rate:** 2400
- **Leader timing:** Standard (2.0s/1.0s)
- **Low-pass filter:** Enabled (7000 Hz - higher for 4800 Hz signal)
- **Amplitude:** 120
- **Rationale:** Fast loading with filter to clean up square wave harmonics

#### `turbo-safe` - Fast Loading (Conservative)
**Use case:** 2400 baud with extra safety margins
- **Waveform:** Trapezoid (10% rise)
- **Baud rate:** 2400
- **Leader timing:** Conservative (3.0s/2.0s)
- **Low-pass filter:** Enabled (7000 Hz)
- **Amplitude:** 115
- **Rationale:** Speed with reliability, gentler waveform at high baud

---

### 4. Problem-Solving Profiles

#### `maximum-compatibility` - When Nothing Else Works
**Use case:** Difficult-to-load files, problematic MSX hardware
- **Waveform:** Trapezoid (20% rise - very gentle)
- **Baud rate:** 1200
- **Leader timing:** Extended (5.0s/3.0s)
- **Low-pass filter:** Enabled (5500 Hz)
- **Amplitude:** 100 (lower to avoid saturation)
- **Rationale:** Maximum safety margins, gentle slopes, long stabilization time

#### `worn-equipment` - Aging/Problematic Hardware
**Use case:** Worn MSX tape interfaces, noisy audio connections
- **Waveform:** Trapezoid (15% rise)
- **Baud rate:** 1200
- **Leader timing:** Conservative (3.0s/2.0s)
- **Low-pass filter:** Enabled (5500 Hz - more aggressive filtering)
- **Amplitude:** 105
- **Rationale:** Gentle waveform, filtered, lower amplitude, more time for AGC

#### `clean-sharp` - Best Signal Discrimination
**Use case:** Good hardware, need best signal clarity
- **Waveform:** Trapezoid (5% rise - sharp edges)
- **Baud rate:** 1200
- **Leader timing:** Standard (2.0s/1.0s)
- **Low-pass filter:** Disabled
- **Amplitude:** 120
- **Rationale:** Sharp transitions for clear 0/1 bit distinction

---

### 5. Tape Recording Profiles

#### `tape-recording` - Recording to Real Cassette Tape
**Use case:** Creating physical cassette tapes from WAV files
- **Waveform:** Trapezoid (10% rise)
- **Baud rate:** 1200
- **Leader timing:** Conservative (3.0s/2.0s)
- **Low-pass filter:** Disabled (tape does its own filtering)
- **Amplitude:** 110 (avoid tape saturation)
- **Rationale:** Tape-friendly waveform, avoid over-driving tape, extra leader time

#### `tape-dubbing` - Creating Master Tapes for Dubbing
**Use case:** Creating high-quality master tapes
- **Waveform:** Sine (cleanest for analog tape)
- **Baud rate:** 1200
- **Leader timing:** Extended (5.0s/3.0s)
- **Low-pass filter:** Disabled
- **Amplitude:** 105 (conservative for tape)
- **Rationale:** Clean signal, extra leader for tape deck motor start

---

### 6. Testing/Development Profiles

#### `emulator` - MSX Emulator Use
**Use case:** Loading WAV files in emulators (openMSX, blueMSX)
- **Waveform:** Square (digital perfect)
- **Baud rate:** 2400 (faster in emulator)
- **Leader timing:** Standard (2.0s/1.0s)
- **Low-pass filter:** Disabled (emulator has perfect decoding)
- **Amplitude:** 127 (maximum for digital)
- **Rationale:** Emulators don't have analog issues, can use ideal settings

#### `debug` - Testing/Analysis
**Use case:** Analyzing waveforms, debugging loading issues
- **Waveform:** Sine
- **Baud rate:** 1200
- **Leader timing:** Standard (2.0s/1.0s)
- **Low-pass filter:** Disabled
- **Amplitude:** 120
- **Rationale:** Pure reference waveform for analysis

---

### 7. Brand-Specific Hardware Profiles

#### `sony-msx` - Sony HitBit Series
**Use case:** Sony HitBit MSX machines (known for sensitive tape inputs)
- **Waveform:** Sine
- **Baud rate:** 1200
- **Leader timing:** Conservative (3.0s/2.0s)
- **Low-pass filter:** Enabled (5500 Hz)
- **Amplitude:** 105 (Sony tape interfaces can be sensitive)
- **Rationale:** Conservative settings for Sony's particular tape circuit design

#### `panasonic-msx` - Panasonic FS-A1 Series
**Use case:** Panasonic MSX2/2+ machines
- **Waveform:** Trapezoid (10% rise)
- **Baud rate:** 1200
- **Leader timing:** Standard (2.0s/1.0s)
- **Low-pass filter:** Enabled (6000 Hz)
- **Amplitude:** 115
- **Rationale:** Panasonic tape interfaces are generally robust but benefit from filtered trapezoid

#### `philips-msx` - Philips VG/NMS Series
**Use case:** Philips MSX machines
- **Waveform:** Trapezoid (15% rise)
- **Baud rate:** 1200
- **Leader timing:** Conservative (3.0s/2.0s)
- **Low-pass filter:** Enabled (6000 Hz)
- **Amplitude:** 110
- **Rationale:** Philips machines sometimes have slower AGC, gentler waveform helps

---

### 8. Experimental Profiles

#### `ultra-clean` - Maximum Signal Purity
**Use case:** Testing hypothesis that cleaner = better
- **Waveform:** Sine
- **Baud rate:** 1200
- **Leader timing:** Extended (5.0s/3.0s)
- **Low-pass filter:** Enabled (5000 Hz - aggressive filtering)
- **Amplitude:** 90 (very conservative)
- **Rationale:** Extreme filtering and low amplitude, minimal harmonics

#### `ultra-sharp` - Maximum Signal Clarity
**Use case:** When bit transitions need to be crystal clear
- **Waveform:** Trapezoid (5% rise - almost square)
- **Baud rate:** 1200
- **Leader timing:** Standard (2.0s/1.0s)
- **Low-pass filter:** Disabled
- **Amplitude:** 127 (maximum)
- **Rationale:** Sharp transitions, maximum amplitude for clear bit detection

#### `high-amplitude` - Loud Signal
**Use case:** Weak/noisy connections, long cables
- **Waveform:** Trapezoid (15% rise)
- **Baud rate:** 1200
- **Leader timing:** Conservative (3.0s/2.0s)
- **Low-pass filter:** Enabled (6000 Hz)
- **Amplitude:** 127 (maximum)
- **Rationale:** Drive through noise with high amplitude, filter out distortion

#### `triangle-test` - Triangle Wave Experiment
**Use case:** Testing linear ramp waveforms
- **Waveform:** Triangle
- **Baud rate:** 1200
- **Leader timing:** Standard (2.0s/1.0s)
- **Low-pass filter:** Enabled (6000 Hz)
- **Amplitude:** 120
- **Rationale:** Different harmonic content than sine/square, may work where others fail

#### `extreme-gentle` - Ultra-Smooth Waveform
**Use case:** Very problematic or damaged MSX tape circuits
- **Waveform:** Trapezoid (50% rise - almost triangle)
- **Baud rate:** 1200
- **Leader timing:** Extended (5.0s/3.0s)
- **Low-pass filter:** Enabled (5000 Hz)
- **Amplitude:** 95
- **Rationale:** Gentlest possible slopes, maximum compatibility with damaged hardware

#### `square-raw` - Unfiltered Digital
**Use case:** Testing if harsh square wave actually works better
- **Waveform:** Square
- **Baud rate:** 1200
- **Leader timing:** Standard (2.0s/1.0s)
- **Low-pass filter:** Disabled
- **Amplitude:** 120
- **Rationale:** Pure digital signal, maximum harmonic content

---

### 9. Special Use Case Profiles

#### `noisy-environment` - High Noise Immunity
**Use case:** Loading in electrically noisy environments
- **Waveform:** Trapezoid (10% rise)
- **Baud rate:** 1200
- **Leader timing:** Conservative (3.0s/2.0s)
- **Low-pass filter:** Enabled (6500 Hz)
- **Amplitude:** 127 (maximize signal-to-noise ratio)
- **Rationale:** High amplitude to overcome noise, filtered to reduce interference

#### `long-cables` - Extended Cable Runs
**Use case:** Computer far from MSX, long audio cables (>3 meters)
- **Waveform:** Trapezoid (15% rise)
- **Baud rate:** 1200
- **Leader timing:** Conservative (3.0s/2.0s)
- **Low-pass filter:** Enabled (5500 Hz)
- **Amplitude:** 127
- **Rationale:** High amplitude, filtered to compensate for cable loss, gentle slopes

#### `basic-programs` - BASIC Program Loading
**Use case:** Optimized for BASIC program files
- **Waveform:** Sine
- **Baud rate:** 1200
- **Leader timing:** Standard (2.0s/1.0s)
- **Low-pass filter:** Enabled (6000 Hz)
- **Amplitude:** 115
- **Rationale:** BASIC files are ASCII, clean sine wave works well

#### `binary-programs` - Binary/Machine Code Loading
**Use case:** Binary files, games, machine code programs
- **Waveform:** Trapezoid (10% rise)
- **Baud rate:** 1200
- **Leader timing:** Conservative (3.0s/2.0s)
- **Low-pass filter:** Enabled (6000 Hz)
- **Amplitude:** 120
- **Rationale:** Binary data needs reliability, conservative timing helps

#### `multi-file` - Multiple Files in Container
**Use case:** CAS files with many small files
- **Waveform:** Sine
- **Baud rate:** 1200
- **Leader timing:** Conservative (3.0s/2.0s)
- **Low-pass filter:** Enabled (6000 Hz)
- **Amplitude:** 115
- **Rationale:** More leader time between files helps MSX track transitions

#### `bluetooth-audio` - Bluetooth Audio Adapters
**Use case:** Using Bluetooth transmitter/receiver in audio chain
- **Waveform:** Trapezoid (15% rise)
- **Baud rate:** 1200
- **Leader timing:** Extended (5.0s/3.0s)
- **Low-pass filter:** Enabled (5000 Hz - accommodate Bluetooth bandwidth)
- **Amplitude:** 110
- **Rationale:** Bluetooth can introduce latency/artifacts, gentle conservative approach

---

## Profile Selection Guide

### By Hardware Type:
- **MSX1 (any brand):** `msx1`
- **MSX2/2+/Turbo R:** `msx2` or `computer-direct`
- **Sony HitBit:** `sony-msx`
- **Panasonic FS-A1:** `panasonic-msx`
- **Philips VG/NMS:** `philips-msx`
- **Unknown/Generic MSX:** `default`
- **Damaged/Worn hardware:** `worn-equipment` or `extreme-gentle`

### By Connection Method:
- **Computer → MSX (audio cable):** `computer-direct`
- **Recording to tape:** `tape-recording`
- **Emulator:** `emulator`

### By Loading Problems:
- **Won't load at all:** `maximum-compatibility` or `extreme-gentle`
- **Intermittent loading:** `worn-equipment` or `noisy-environment`
- **Slow loading (want faster):** `turbo-safe` or `turbo`
- **Loads but want optimal quality:** `clean-sharp` or `computer-direct`
- **Long cable issues:** `long-cables`
- **Bluetooth artifacts:** `bluetooth-audio`

### By Priority:
- **Speed:** `turbo` or `turbo-safe` or `emulator`
- **Reliability:** `maximum-compatibility` or `extreme-gentle`
- **Quality:** `clean-sharp`, `ultra-clean`, or `computer-direct`
- **Compatibility:** `msx1`, `maximum-compatibility`, or `extreme-gentle`

### By File Type:
- **BASIC programs:** `basic-programs`
- **Binary/Games:** `binary-programs`
- **Multiple files:** `multi-file`

### Experimental Testing:
- **Clean signal theory:** `ultra-clean`
- **Sharp signal theory:** `ultra-sharp` or `clean-sharp`
- **High amplitude theory:** `high-amplitude`
- **Alternative waveforms:** `triangle-test` or `square-raw`
- **Gentle waveform theory:** `extreme-gentle`

---

## Implementation Notes

Each profile should be accessible via:
```bash
# Using preset
./cast convert game.cas game.wav --preset computer-direct

# Overriding specific settings
./cast convert game.cas game.wav --preset msx1 --baud 2400
```

Profiles should be implemented in `lib/presetlib.c` with a simple lookup function.
