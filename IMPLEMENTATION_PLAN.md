# Implementation Plan: WAV Cue Point Markers

## Phase 1: Core Infrastructure (Foundation)

**1.1 Data Structures (lib/wavlib.h)**
- [ ] Define `MarkerCategory` enum (STRUCTURE, DETAIL, VERBOSE)
- [ ] Define `Marker` struct (sample_position, category, description)
- [ ] Define `MarkerList` struct (array of markers, count, capacity)
- [ ] Add marker tracking to `WavWriter` struct

**1.2 Marker Management (lib/wavlib.c)**
- [ ] `createMarkerList()` - Initialize empty marker list
- [ ] `addMarker(list, sample_pos, category, description)` - Append marker
- [ ] `freeMarkerList()` - Clean up memory
- [ ] Track current sample position in WavWriter (increment on every write)

**Estimated effort:** 2-3 hours

---

## Phase 2: Marker Generation (Instrumentation)

**2.1 Modify Audio Generation Functions (lib/wavlib.c)**
- [ ] `writeSilence()` - Add DETAIL marker when silence starts
- [ ] `writeSync()` - Add DETAIL/VERBOSE markers for sync boundaries
- [ ] `writeByte()` - Add VERBOSE markers for data progress (every N bytes)
- [ ] `writePulse()` - No markers (too granular)

**2.2 Modify CAS Conversion (lib/wavlib.c or commands/convert.c)**
- [ ] Add STRUCTURE markers: File start/end
- [ ] Add STRUCTURE markers: Header start/end
- [ ] Add STRUCTURE/DETAIL markers: Data block start/end
- [ ] Add DETAIL markers: File type, address ranges
- [ ] Add DETAIL/VERBOSE markers: Data progress (25%/10% intervals)

**2.3 Pass Marker Enable Flag**
- [ ] Add `bool enable_markers` to `WaveformConfig`
- [ ] Add `MarkerList*` to `WavWriter` (NULL if disabled)
- [ ] Only generate markers if flag is true

**Estimated effort:** 4-5 hours

---

## Phase 3: WAV Cue Chunk Writing

**3.1 RIFF Chunk Structures (lib/wavlib.c)**
```c
typedef struct {
    char id[4];        // "cue "
    uint32_t size;
    uint32_t cue_count;
} CueChunkHeader;

typedef struct {
    uint32_t cue_id;
    uint32_t position;
    char chunk_id[4];  // "data"
    uint32_t chunk_start;
    uint32_t block_start;
    uint32_t sample_offset;
} CuePoint;

typedef struct {
    char list[4];      // "LIST"
    uint32_t size;
    char type[4];      // "adtl"
} ListChunkHeader;

typedef struct {
    char labl[4];      // "labl"
    uint32_t size;
    uint32_t cue_id;
    // Followed by null-terminated string
} LabelChunkHeader;
```

**3.2 Chunk Writing Functions**
- [ ] `writeCueChunk(file, markers)` - Write cue points
- [ ] `writeAdtlChunk(file, markers)` - Write labels with category tags
- [ ] Format label as: `"[CATEGORY] Description"` (e.g., `"[DETAIL] Silence (2.0s)"`)

**3.3 Modify closeWavFile()**
- [ ] After data chunk, append cue chunk
- [ ] Append LIST/adtl chunk
- [ ] Update RIFF file size in header

**Estimated effort:** 3-4 hours

---

## Phase 4: Command-Line Interface

**4.1 Modify convert command (cast.c)**
- [ ] Add `--markers` flag to argument parser
- [ ] Pass flag through to execute_convert()

**4.2 Modify execute_convert() (commands/convert.c)**
- [ ] Accept `bool enable_markers` parameter
- [ ] Set in WaveformConfig
- [ ] Pass to conversion function

**4.3 Verbose Output**
- [ ] Show marker count after conversion
- [ ] Example: `"Generated 47 markers (12 structure, 23 detail, 12 verbose)"`

**Estimated effort:** 1-2 hours

---

## Phase 5: Testing & Validation

**5.1 Unit Tests**
- [ ] Test marker list operations (add, free)
- [ ] Test sample position tracking
- [ ] Verify marker categories assigned correctly

**5.2 Integration Tests**
- [ ] Convert small CAS file with markers
- [ ] Verify cue chunk format with hex dump
- [ ] Load in Audacity (should show cue labels)
- [ ] Test different profiles (verify timing differs)

**5.3 Edge Cases**
- [ ] Empty CAS file
- [ ] Single file CAS
- [ ] Multi-file CAS
- [ ] Large files (marker count reasonable?)

**Estimated effort:** 2-3 hours

---

## Phase 6: Playback Command (Future - Deferred)

**Note:** This phase implements `cast play` command. Can be done separately after Phase 1-5 complete.

- [ ] Parse WAV cue/adtl chunks
- [ ] Filter markers by category
- [ ] Real-time display during playback
- [ ] `--show` flag implementation

**Estimated effort:** 6-8 hours (deferred)

---

## Total Estimated Effort

**Phases 1-5 (Marker Generation):** 12-17 hours  
**Phase 6 (Playback - Optional):** 6-8 hours

---

## Success Criteria

✅ **Phase 1-5 Complete When:**
1. `cast convert game.cas game.wav --markers` generates valid WAV with cue points
2. Audacity shows markers in the timeline
3. Different profiles produce different marker timestamps
4. No markers generated without `--markers` flag
5. Backward compatible (existing WAVs work unchanged)

✅ **Phase 6 Complete When:**
6. `cast play game.wav` displays markers in real-time
7. `--show` flag filters correctly
8. `--no-markers` suppresses display

---

## Implementation Order

1. **Start with Phase 1** - Get infrastructure in place
2. **Then Phase 2** - Instrument one function at a time
3. **Then Phase 3** - Write cue chunks (test with Audacity)
4. **Then Phase 4** - Wire up CLI
5. **Then Phase 5** - Test thoroughly
6. **Defer Phase 6** - Can do later when needed

---

## Notes

- Reference: docs/PLAYBACK_MARKERS.md
- This is a working document, will be modified/deleted as needed
- Focus on Phases 1-5 first (marker generation)
- Phase 6 (playback) can wait
