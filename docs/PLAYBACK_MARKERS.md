# WAV Playback Markers Design

## Overview

Embed cue points in WAV files during conversion to enable real-time playback monitoring in the `cast play` command. Markers track what MSX cassette loading activity is happening at each point in the audio timeline.

## Critical Requirement

**Cue points must be generated during WAV conversion** using the actual `WaveformConfig` parameters:
- `baud_rate` - affects all timing
- `long_silence` / `short_silence` - affects marker positions
- `sample_rate` - affects sample position calculations
- Profile settings affect all of the above

Different profiles = different marker timings for the same CAS file.

## Marker Generation Strategy

**During Conversion:** Simple on/off flag
```bash
cast convert game.cas game.wav --markers
```
- If enabled: Generate ALL markers with maximum detail
- If disabled (default): No markers, WAV file is pure audio
- Only need to convert once with markers enabled

**During Playback:** Choose display level
```bash
cast play game.wav                    # Standard display
cast play game.wav --show=minimal     # Structure only
cast play game.wav --show=verbose     # Everything
cast play game.wav --no-markers       # Hide all markers
```
- User chooses which markers to DISPLAY
- Same WAV file, different viewing preferences
- Fast filtering, no re-processing needed

## Marker Categories

All markers generated during conversion are tagged with a category. During playback, these categories control filtering:

### Category: STRUCTURE
Essential structure only:
- File boundaries (start/end)
- File headers
- Data blocks (start/end)

**Typical count:** ~10-20 markers per file

### Category: DETAIL
Includes STRUCTURE + operational details:
- Silence periods
- Sync periods
- File type information
- Address ranges (Binary/BASIC)
- Data block progress (25% intervals)

**Typical count:** ~30-50 markers per file

### Category: VERBOSE
Includes DETAIL + fine-grained information:
- Sync start/end boundaries
- Header phase transitions
- Data progress at 10% intervals
- All timing details

**Typical count:** ~100+ markers per file

## Implementation Architecture

### 1. Data Structure

Each marker contains:
- **Sample position:** Exact location in WAV audio data
- **Category:** STRUCTURE, DETAIL, or VERBOSE
- **Description:** Human-readable text (e.g., "File header: Binary GAME")

### 2. During Conversion

When `--markers` flag is present:
- Track current sample position throughout conversion
- Add markers at every significant event with appropriate category
- After audio generation, write cue points to WAV metadata

Markers inserted at:
- **File boundaries:** Start/end of each file in container
- **Silence periods:** Before file headers and data blocks
- **Sync pulses:** Long (8000 bits) and short (2000 bits)
- **Headers:** File headers and data block headers
- **Data transmission:** Block start/end and progress percentages
- **Phase transitions:** Entry/exit of encoding phases (verbose only)

### 3. During Playback

Read cue points from WAV file:
- Parse WAV metadata chunks (cue + associated labels)
- Build timeline of events with sample positions
- Filter based on display level (minimal/standard/verbose)
- Display markers as playback position advances

### 4. WAV Cue Chunk Format

Standard RIFF WAV structure with metadata chunks:

```
WAV File Layout:
┌─────────────────────┐
│ RIFF Header         │
├─────────────────────┤
│ fmt Chunk           │
├─────────────────────┤
│ data Chunk          │ ← MSX reads only this (ignores metadata)
├─────────────────────┤
│ cue Chunk           │ ← Marker sample positions
├─────────────────────┤
│ LIST/adtl Chunk     │ ← Marker labels and categories
└─────────────────────┘
```

The `cue` chunk contains sample positions, and the `LIST/adtl` chunk contains the descriptive text and category tags for each marker.

### 5. Playback Display

During playback, monitor audio position and display current marker:

**Display filtering based on level:**
- **Minimal:** Only show STRUCTURE category markers
- **Standard (default):** Show STRUCTURE + DETAIL category markers
- **Verbose:** Show all markers (STRUCTURE + DETAIL + VERBOSE)

**Update frequency:** Poll playback position every 50-100ms to detect marker transitions

## Command-Line Interface

### Conversion with Markers

```bash
# No markers (default for backward compatibility)
cast convert game.cas game.wav

# Minimal markers
cast convert game.cas game.wav --markers=minimal

# Standard markers
cast convert game.cas game.wav --markers=standard
cast convert game.cas game.wav --markers  # same as standard

# Verbose markers
cast convert game.cas game.wav --markers=verbose

# With profile (markers reflect profile timing)
cast convert game.cas game.wav --profile turbo --markers=standard
```

### Playback with Marker Display

```bash
# Play and show markers (if they exist)
cast play game.wav

# Play without showing markers (even if present)
cast play game.wav --no-markers

# Play with specific display options
cast play game.wav --markers-only  # Just markers, no progress bar
cast play game.wav --progress      # Progress bar + markers
```

## Profile Dependency Examples

Same CAS file, different profiles = different marker positions:

**Profile: default (1200 baud, 2.0s/1.0s silence)**
```
[00:00] File 1/1: Binary "GAME"
[00:00] Silence (2.0s)
[00:02] Sync long (8000 bits)
[00:08] File header
[00:09] Silence (1.0s)
[00:10] Sync short (2000 bits)
[00:11] Data block (32768 bytes)
[04:45] Complete
```

**Profile: turbo (2400 baud, 2.0s/1.0s silence)**
```
[00:00] File 1/1: Binary "GAME"
[00:00] Silence (2.0s)
[00:02] Sync long (8000 bits)  ← Half the time at 2x baud
[00:05] File header
[00:06] Silence (1.0s)
[00:07] Sync short (2000 bits)
[00:08] Data block (32768 bytes)
[02:30] Complete  ← Half total time
```

**Profile: conservative (1200 baud, 3.0s/2.0s silence)**
```
[00:00] File 1/1: Binary "GAME"
[00:00] Silence (3.0s)  ← Longer silence
[00:03] Sync long (8000 bits)
[00:09] File header
[00:10] Silence (2.0s)  ← Longer silence
[00:12] Sync short (2000 bits)
[00:13] Data block (32768 bytes)
[04:47] Complete
```

## Validation

Markers must be validated during generation:
- Sample positions are monotonically increasing
- All positions are within WAV data chunk bounds
- Labels are valid UTF-8 strings
- Cue IDs are unique

## Future Enhancements

- **Interactive mode:** Highlight which file MSX is currently loading
- **Seek support:** Jump to specific file/block in timeline
- **Remote control integration:** Show pause point in context
- **Export timeline:** Generate human-readable timeline text file
- **Marker editing:** Post-process markers without re-converting
