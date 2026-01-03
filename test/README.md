# Test Directory

This directory contains test programs and sample output files for the wavlib audio generation library.

## Test Programs

### Phase Tests (Original Development)
- `test_wavlib.c` through `test_wavlib_phase6.c` - Progressive development tests for WAV library phases

### Audio Enhancement Tests

#### Low-Pass Filter Test
- **Program:** `test_lowpass.c`
- **Output:** `test_square_raw.wav`, `test_square_filtered.wav`
- **Purpose:** Demonstrates the low-pass filter reducing high-frequency harmonics
- **Comparison:** Raw square wave vs. 6000 Hz filtered square wave

#### Trapezoid Rise Time Test
- **Program:** `test_trapezoid_rise.c`
- **Output:** `test_trap_rise5.wav`, `test_trap_rise10.wav`, `test_trap_rise15.wav`, `test_trap_rise20.wav`
- **Purpose:** Shows configurable rise/fall times (5%, 10%, 15%, 20%)
- **Comparison:** Sharp edges (5%) to gentle slopes (20%)

#### Leader/Silence Timing Test
- **Program:** `test_leader_timing.c`
- **Output:** `test_leader_standard.wav`, `test_leader_conservative.wav`, `test_leader_extended.wav`
- **Purpose:** Demonstrates different leader timing presets
- **Timing:**
  - Standard: 2.0s/1.0s (fast loading)
  - Conservative: 3.0s/2.0s (more AGC time)
  - Extended: 5.0s/3.0s (maximum compatibility)

## Running Tests

To compile and run all tests:

```bash
# Compile individual test
gcc -Wall -Wextra -O2 -o test/test_lowpass test/test_lowpass.c lib/wavlib.c lib/caslib.c -lm

# Run test
./test/test_lowpass
```

## Analyzing Output

The generated WAV files can be analyzed in audio editors like Audacity to:
- View waveforms visually
- Compare filtered vs. unfiltered signals
- Verify timing and amplitude
- Check for clicks, pops, or artifacts
