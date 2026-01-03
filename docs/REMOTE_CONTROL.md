# MSX Remote Control Audio Playback

## Problem Statement

When loading multi-stage games or two-sided tapes from computer-generated WAV files to a real MSX, the MSX uses its cassette REMOTE control signal to pause/resume the tape motor. Currently, there's no way for the computer playing the WAV file to detect and respond to this signal, requiring manual pause/resume on the media player.

## MSX REMOTE Signal Specifications

### Electrical Characteristics
Based on MSX I/O ports documentation:

**Source:** PPI Register C (Port #AA) - Bit 4
- **Signal Type:** TTL-level digital output
- **Logic Levels:**
  - `MOTOR ON` (play): **0V (LOW)**
  - `MOTOR OFF` (pause): **+5V (HIGH)**
- **Note:** Active-low logic - signal is LOW when motor should run

**Also available:**
- PSG Register 14 - Bit 7: Cassette input signal (for reading audio data)

### Physical Connection
User has MSX cassette cable with:
- 2x mini-jack (for LOAD/SAVE audio)
- 1x micro-jack (for REMOTE motor control)

REMOTE micro-jack provides the TTL signal directly.

## Proposed Solution: Microphone-Based Detection

No additional hardware required - uses computer's existing audio input.

### Architecture Overview

```
MSX REMOTE (micro-jack) → Computer Mic/Line-In Input
                          ↓
                    Audio Monitor Daemon
                          ↓
                    ┌─────┴─────┐
                    ↓           ↓
            Signal Detector   WAV Player
                    ↓           ↓
              State Machine   Audio Output
                    └─────┬─────┘
                          ↓
                    Play/Pause Control
```

### Component Design

#### 1. Audio Input Monitor

**Technology:** Python (PyAudio) or C (PortAudio)

**Configuration:**
```
Input device: Microphone/Line-In
Sample rate: 44100 Hz (standard)
Channels: Mono (REMOTE on one channel)
Buffer size: 256-512 samples (minimize latency)
```

**Processing Pipeline:**
1. Read audio samples continuously
2. Calculate RMS/average voltage level per buffer
3. Apply threshold detection:
   - `LOW threshold: < 0.5V` → MOTOR ON (play)
   - `HIGH threshold: > 2.0V` → MOTOR OFF (pause)
4. Debounce: Ignore transitions < 50ms

#### 2. State Machine

**States:**
- `STOPPED`: Initial state, no playback
- `PLAYING`: WAV audio playing, REMOTE is LOW
- `PAUSED`: WAV audio paused, REMOTE is HIGH

**Transitions:**
- `STOPPED → PLAYING`: REMOTE goes LOW
- `PLAYING → PAUSED`: REMOTE goes HIGH
- `PAUSED → PLAYING`: REMOTE goes LOW
- `Any → STOPPED`: User stops playback

#### 3. WAV Playback Controller

**Option A: External Player Control**
- Use mpv/VLC with JSON IPC or socket API
- Daemon sends play/pause commands
- **Pro:** Simple, uses existing player
- **Con:** Some latency, requires player setup

**Option B: Built-in Playback**
- Daemon reads WAV file
- Opens audio output device
- Manages playback position internally
- **Pro:** Lower latency, full control
- **Con:** More complex, reinvents player functionality

### Signal Detection Algorithm

```python
# Configuration
THRESHOLD_LOW = 0.5   # Below this = motor on
THRESHOLD_HIGH = 2.0  # Above this = motor off
DEBOUNCE_TIME = 0.05  # 50ms

# State
current_state = STOPPED
last_change_time = 0

# Main loop
while True:
    # Read audio buffer from mic input
    samples = read_audio_input(buffer_size=512)
    
    # Calculate average voltage (RMS)
    avg_voltage = calculate_rms(samples)
    
    # Detect signal state
    if avg_voltage < THRESHOLD_LOW:
        signal_state = MOTOR_ON
    elif avg_voltage > THRESHOLD_HIGH:
        signal_state = MOTOR_OFF
    else:
        # In-between zone, keep previous state
        continue
    
    # Debounce check
    current_time = get_time()
    if current_time - last_change_time < DEBOUNCE_TIME:
        continue
    
    # State machine transitions
    if signal_state == MOTOR_ON and current_state != PLAYING:
        play_audio()
        current_state = PLAYING
        last_change_time = current_time
    elif signal_state == MOTOR_OFF and current_state == PLAYING:
        pause_audio()
        current_state = PAUSED
        last_change_time = current_time
```

## Implementation Options

### Option 1: Standalone Python Script

**File:** `castools-ng/remote-player.py`

**Dependencies:**
- PyAudio (mic input monitoring)
- python-vlc or mpv subprocess (playback control)
- argparse (CLI interface)

**Usage:**
```bash
./remote-player.py game.wav --input-device 1 --threshold 1.5
```

**Pros:**
- Quick to prototype
- Easy to test and iterate
- Portable (cross-platform)

**Cons:**
- Separate tool from `cast`
- Requires Python + dependencies

### Option 2: Integrated into cast Command

**Implementation:** C with PortAudio library

**Usage:**
```bash
cast play game.wav --remote-control
```

**Features:**
- Same codebase as rest of castools-ng
- Simultaneous mic monitoring + audio output
- Self-contained binary

**Pros:**
- Native performance
- Integrated user experience
- No external dependencies (beyond PortAudio)

**Cons:**
- More complex implementation
- Audio I/O handling in C

### Option 3: Two-Process Approach

**Process 1:** `remote-monitor`
- Monitors mic input for REMOTE signal
- Sends commands to named pipe/socket

**Process 2:** Standard media player
- Controlled via IPC (mpv JSON socket)
- Responds to play/pause from monitor

**Pros:**
- Separation of concerns
- Can use any media player
- Monitor can be reused

**Cons:**
- IPC complexity
- Multiple processes to manage

## Challenges & Solutions

### Challenge 1: Latency
**Problem:** Delay between REMOTE signal change and audio response

**Solution:**
- Small buffer sizes (256-512 samples)
- Priority/real-time scheduling
- Direct audio device access
- Target: < 50ms response time

### Challenge 2: Threshold Calibration
**Problem:** Different cables/soundcards have different voltage levels

**Solution:**
- Auto-calibration mode:
  ```bash
  cast remote-calibrate
  > Please toggle MSX MOTOR ON/OFF...
  > LOW detected: 0.3V
  > HIGH detected: 4.2V
  > Recommended thresholds: LOW=1.5V HIGH=3.0V
  ```
- Manual override via CLI flags
- Store calibration in config file

### Challenge 3: Multi-Stage Games
**Problem:** Some games do rapid MOTOR ON/OFF sequences

**Solution:**
- Debouncing with configurable timeout
- State history tracking
- Min/max hold times

### Challenge 4: Audio Device Selection
**Problem:** User has multiple inputs (mic, line-in, USB audio, etc.)

**Solution:**
- Device enumeration and listing:
  ```bash
  cast remote-devices
  > 0: Built-in Microphone
  > 1: Line In
  > 2: USB Audio Interface
  ```
- Selection via CLI flag: `--input-device 1`
- Remember last used device

### Challenge 5: Voltage Level Detection
**Problem:** Mic input expects small AC signals, REMOTE is DC

**Solutions:**
- **DC coupling:** If soundcard supports it (line-in usually does)
- **AC coupling workaround:** Monitor signal transitions rather than absolute level
- **Bias detection:** Look for baseline shift patterns

## Proof of Concept Test Plan

### Test 1: Monitor Mic Input Levels
```bash
cast remote-test --show-levels
```

**Expected Output:**
```
Input device: Built-in Microphone
Monitoring REMOTE signal (press Ctrl+C to stop)...

Time    | Voltage | State
--------|---------|--------
00:00   | 0.1V    | MOTOR ON
00:05   | 4.2V    | MOTOR OFF
00:08   | 0.1V    | MOTOR ON
```

### Test 2: Play with Remote Control
```bash
cast play game.wav --remote
```

**Expected Output:**
```
Monitoring REMOTE signal on input device 0
Thresholds: LOW=0.5V HIGH=2.0V
[PAUSED] Waiting for MOTOR ON signal...
[PLAYING] Started playback (REMOTE=LOW)
[00:42] PAUSED (REMOTE=HIGH)
[00:42] PLAYING (REMOTE=LOW)
[08:34] Playback complete
```

### Test 3: Real MSX Multi-Stage Load
```bash
# Terminal 1: Start playback with remote control
cast play disc.wav --remote

# Terminal 2: Monitor status
cast remote-status
```

**Validate:**
- MSX can control playback via REMOTE
- Audio starts/stops with < 100ms latency
- No audio glitches during pause/resume
- Playback position preserved across pauses

## Next Steps

1. **Prototype in Python**
   - Quick proof of concept
   - Validate signal detection approach
   - Test with real MSX hardware

2. **Measure Performance**
   - Signal detection latency
   - Audio response time
   - CPU usage
   - Acceptable for gaming?

3. **Decide Integration**
   - Standalone tool vs. built into `cast`
   - Python vs. C implementation
   - External player vs. built-in playback

4. **Implement Features**
   - Auto-calibration
   - Device selection
   - Config file support
   - Status monitoring

5. **Documentation**
   - User guide for setup
   - Troubleshooting common issues
   - Cable requirements

## References

- MSX I/O Ports: https://map.grauw.nl/resources/msx_io_ports.php
- PPI Register C (Port #AA), Bit 4: Cassette motor control
- MSX Technical Data Book: Cassette interface specifications
