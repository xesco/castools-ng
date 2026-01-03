# cast play Command - Implementation Plan

## Overview
Implement a `cast play` command that plays back converted WAV files with real-time loading progress visualization using embedded cue point markers.

## Goals
1. Play WAV audio files through system audio
2. Display real-time loading progress synchronized with playback
3. Show current file/block being loaded from the MSX tape
4. Provide visual feedback similar to actual MSX tape loading experience
5. Support pause/resume and seeking functionality

## Command Syntax
```bash
cast play <file.wav> [options]

Options:
  --speed <factor>    Playback speed multiplier (0.5-4.0) [default: 1.0]
                      Values > 1.0 = faster, < 1.0 = slower
  --volume <level>    Output volume (0.0-1.0) [default: 0.8]
                      0.0 = silent, 1.0 = full volume
  --markers-only      Just display markers without playing audio
  --start-at <time>   Start playback at specific time (seconds or mm:ss)
  --no-progress       Disable progress display (audio only)
  --verbose           Show detailed marker information
  -h, --help          Show help message
```

## Architecture

### Phase 1: Marker Reading Infrastructure
**Goal**: Read WAV cue/adtl chunks into memory

**Components**:
- `lib/playlib.c/h` - New library for playback operations
  - Marker reading (cue/adtl chunk parsing)
  - Audio playback (miniaudio integration)
  - Position tracking and controls
  - `MarkerInfo` struct (sample, time, category, description)
  - `readWavMarkers(filename)` - Parse cue/adtl chunks
  - `findMarkerAtTime(markers, time)` - Locate marker by timestamp
  - `freeMarkerInfo()` - Cleanup

**Implementation**:
```c
typedef struct {
    uint32_t sample_position;
    double time_seconds;
    char category[16];      // "STRUCTURE", "DETAIL", "VERBOSE"
    char description[256];
} MarkerInfo;

typedef struct {
    MarkerInfo *markers;
    size_t count;
    uint32_t sample_rate;
} MarkerList;
```

**Rationale**:
- Keep `wavlib` focused on WAV file *writing*
- `playlib` handles WAV file *reading* and playback
- Clean separation of concerns
- Easier to test and maintain

**Tasks**:
1. Create lib/playlib.c and lib/playlib.h
2. Implement WAV chunk reader (cue/adtl structures)
3. Parse cue chunk for sample positions
4. Parse adtl/labl chunks for descriptions
5. Match cue IDs to labels
6. Calculate timestamps from sample positions

### Phase 2: Audio Playback (miniaudio)
**Goal**: Play audio with precise position tracking

**Library**: miniaudio (https://miniaud.io/)
- Single-header library (just `lib/miniaudio.h`)
- Cross-platform: Linux, macOS, Windows, BSD
- Zero external dependencies
- Public domain license
- Battle-tested (used in Godot engine, many projects)

**Setup**:
```bash
curl -o lib/miniaudio.h https://raw.githubusercontent.com/mackron/miniaudio/master/miniaudio.h
```

**Implementation in `lib/playlib.c`** (~200 lines):
```c
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

typedef struct {
    ma_decoder decoder;
    ma_device device;
    bool is_playing;
    bool is_paused;
    float volume;
} AudioPlayer;

// Init
AudioPlayer* createPlayer(const char *filename);

// Get current position (precise!)
double getPlaybackPosition(AudioPlayer *player);

// Control
void playAudio(AudioPlayer *player);
void pauseAudio(AudioPlayer *player);
void seekAudio(AudioPlayer *player, double seconds);
void setVolume(AudioPlayer *player, float volume);
void destroyPlayer(AudioPlayer *player);
```

**Advantages**:
- **Precise position tracking** - know exact playback time anytime
- **No position drift** - accurate to the sample
- **Full playback control** - pause, resume, seek built-in
- **No external dependencies** - just drop in one .h file
- **Cross-platform** - works on Linux, macOS, Windows
- **All in playlib** - single library for markers + playback

**Tasks**:
1. Download miniaudio.h to lib/
2. Implement AudioPlayer in playlib.c (~200 lines)
3. Add audio callback for streaming playback
4. Integrate position tracking with marker timeline
5. Implement pause/resume/seek/volume functions

### Phase 3: Progress Display
**Goal**: Real-time visual feedback during playback

**Display Strategy**:
Analysis of marker timing shows many markers fire at the same timestamp (0-second gaps). To ensure all information is visible:
- **STRUCTURE markers** update the main "Loading" and "Phase" lines (stay visible until next STRUCTURE marker)
- **DETAIL markers** accumulate in a "Recent Events" scrolling list (show last 3-4 events)
- Display remains stable during long data transfers (e.g., 227s for large blocks)
- User always sees current loading phase, even when no new markers arrive

**Display Layout**:
```
╔═══════════════════════════════════════════════════════════╗
║ MSX Tape Player - disc.wav                                ║
╠═══════════════════════════════════════════════════════════╣
║ Loading: File 2/3 - BINARY "GRAPH "                       ║
║ Phase:   Data block 1/1 (24578 bytes)                     ║
║                                                            ║
║ Recent Events (3 most recent):                            ║
║   15.830s  Silence (2.0s)                                 ║
║   22.640s  Sync long (8000 bits)                          ║
║   23.640s  Sync short (2000 bits)                         ║
║                                                            ║
║ Progress: [████████████████████░░░░░░░░░░] 62.5%          ║
║ Time:     2:15 / 8:42 (remaining: 6:27)                   ║
║ Volume:   80%                                              ║
╚═══════════════════════════════════════════════════════════╝

Controls: SPACE=pause  Q=quit  ←/→=seek ±5s  ↑/↓=volume
```

**Marker Display Logic**:
```
On each marker encountered:
  if marker.category == STRUCTURE:
    if "File X/Y" in description:
      Update "Loading:" line
    else:
      Update "Phase:" line
  else if marker.category == DETAIL:
    Add to recent_events list (keep last 3)
    Format timestamp as X.XXXs (millisecond precision)
    Scroll display upward if needed
```

**Terminal Control**:
- ANSI escape codes for cursor positioning
- Update display without scrolling
- Show progress bar based on elapsed time
- Smart marker accumulation (not just "current" marker)
- Update every 100ms

**Tasks**:
1. Implement ANSI terminal control functions
2. Create progress bar rendering
3. Format time display (mm:ss for total time, X.XXXs for event timestamps)
4. Update display loop (separate from playback)
5. Handle terminal resize
6. Implement marker categorization logic
7. Manage recent events circular buffer (keep last 3 DETAIL markers)

### Phase 4: Interactive Controls
**Goal**: User interaction during playback

**Controls**:
- `SPACE` - Pause/Resume playback
- `Q` or `CTRL-C` - Quit
- `←/→` - Seek backward/forward 5 seconds
- `[/]` - Seek to previous/next marker
- `+/-` - Adjust playback speed (if supported by miniaudio)
- `↑/↓` - Increase/decrease volume by 10%

**Implementation**:
- Set terminal to raw mode (disable line buffering)
- Read stdin in non-blocking mode
- Handle key presses in main loop
- Use miniaudio's pause/seek functions

**Tasks**:
1. Implement terminal raw mode setup/restore
2. Non-blocking keyboard input
3. Pause/resume via `ma_device_stop()`/`ma_device_start()`
4. Seek by calling `ma_decoder_seek_to_pcm_frame()`
5. Speed control (investigate miniaudio resampling capabilities)
6. Volume control via `ma_device_set_master_volume()` (runtime adjustment)

### Phase 5: Markers-Only Mode
**Goal**: Display marker list without playing audio

**Output Format**:
```
WAV File Markers - disc.wav
Sample Rate: 43200 Hz
Duration: 522.68 seconds (8:42)
Total Markers: 21

  0.00s  [STRUCTURE] File 1/3: ASCII "DWAR  "
  0.00s  [STRUCTURE] File header
  2.00s  [DETAIL]    Silence (2.0s)
  8.81s  [DETAIL]    Sync long (8000 bits)
  9.81s  [STRUCTURE] Data block 1/1 (256 bytes)
 13.83s  [STRUCTURE] File 2/3: BINARY "GRAPH "
 15.83s  [DETAIL]    Silence (2.0s)
...
```

**Tasks**:
1. Parse WAV file for markers only
2. Format and print marker table
3. Optional: export as CSV/JSON

## File Structure

```
lib/playlib.c        - Marker reading + audio playback
lib/playlib.h        - Public API for playback
lib/miniaudio.h      - Single-header audio library (downloaded)
commands/play.c      - Play command implementation
test/test_playback.c - Playback functionality tests
```

## Implementation Phases

### Minimal Viable Product (MVP)
**Scope**: Basic playback with marker display
- Phase 1: Marker reading in playlib
- Phase 2: Playback with miniaudio in playlib
- Phase 3: Basic progress display (time + current marker)

**Estimated effort**: 3-4 hours

### Full Feature Set
**Scope**: All planned features
- Phase 4: Interactive controls
- Phase 5: Markers-only mode
- Additional: Playback speed, seeking, enhanced UI

**Estimated effort**: 4-6 hours additional

## Testing Strategy

1. **Unit Tests**: Marker reading from known WAV files
2. **Integration Tests**: Full playback cycle
3. **Manual Tests**: 
   - Various CAS files with different marker counts
   - Long files (>5 minutes)
   - Files with/without markers
   - Terminal resize handling

## Alternative/Future Enhancements

1. **Waveform Visualization**: Mini ASCII waveform in display
2. **Export Progress Log**: Save marker timestamps to file
3. **Network Streaming**: Play from URL
4. **Multiple File Queue**: Load sequence of WAV files
5. **Recording Mode**: Record actual MSX loading and compare timing
6. **Marker Editing**: Add/remove/edit markers in existing files

## Risks & Mitigations

| Risk | Impact | Mitigation |
|------|--------|------------|
| miniaudio learning curve | Low | Well-documented, examples available |
| Terminal compatibility issues | Low | Test on common terminals, fallback to simple output |
| Large files memory usage | Low | miniaudio streams, don't load entire WAV |
| Audio playback interruptions | Low | miniaudio handles buffering automatically |
| Zero-gap markers not visible | **SOLVED** | Accumulate markers by category, show state not events |

## Dependencies

- **External**: miniaudio.h (single-header, included in project)
- **Internal**: playlib (new - markers + playback), wavlib (existing, only for shared WAV structures if needed)
- **System**: None (miniaudio handles all platform-specific audio)

## Success Criteria

- [ ] Can read markers from WAV file reliably
- [ ] Plays audio synchronized with display
- [ ] Progress updates within 100ms of actual playback position
- [ ] Displays correct marker descriptions at right times
- [ ] Handles files with 0 markers gracefully
- [ ] Clean shutdown on interrupt
- [ ] Works on standard macOS Terminal and iTerm2

## Example Session

```bash
$ ./cast convert game.cas game.wav --markers -v
✓ Conversion complete! (21 markers embedded)

$ ./cast play game.wav
╔═══════════════════════════════════════════════════════════╗
║ MSX Tape Player - game.wav                                ║
╠═══════════════════════════════════════════════════════════╣
║ Loading: File 1/3 - BASIC "MENU   "                       ║
║ Phase:   Data block 1/1 (2048 bytes)                      ║
║                                                            ║
║ Recent Events (3 most recent):                            ║
║    0.000s  File header                                    ║
║    2.000s  Silence (2.0s)                                 ║
║    8.810s  Sync long (8000 bits)                          ║
║                                                            ║
║ Progress: [████░░░░░░░░░░░░░░░░░░░░] 15.2%                ║
║ Time:     0:45 / 4:52 (remaining: 4:07)                   ║
╚═══════════════════════════════════════════════════════════╝

# During long data block transfer (227 seconds):
# Display remains stable showing the same block info
# Users can see it's still transferring the large file

$ ./cast play game.wav --markers-only
WAV File Markers - game.wav
21 markers found (0:00 to 4:52)

  0:00  [STRUCTURE] File 1/3: BASIC "MENU   "
  0:00  [STRUCTURE] File header
  0:00  [DETAIL]    Silence (2.0s)
  0:02  [DETAIL]    Sync long (8000 bits)
  ...
```

## Next Steps

1. Create `lib/markerlib.c` with marker reading functions
2. Implement basic `cast play` command with afplay integration
3. Add progress display with marker tracking
4. Test with existing WAV files from test suite
5. Iterate based on user feedback
