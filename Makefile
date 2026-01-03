CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = cast

SRCS = cast.c \
       commands/list.c \
       commands/info.c \
       commands/export.c \
       commands/convert.c \
       commands/profile.c \
       commands/play.c \
       lib/caslib.c \
       lib/printlib.c \
       lib/cmdlib.c \
       lib/wavlib.c \
       lib/presetlib.c \
       lib/playlib.c \
       lib/uilib.c

OBJS = $(SRCS:.c=.o)

# Libraries needed for linking
LIBS = -lpthread -lm -ldl

# Test programs
TEST_LIBS = lib/wavlib.o lib/caslib.o test/test_utils.o
TEST_PROGS = test/test_lowpass test/test_trapezoid_rise test/test_leader_timing test/test_wavlib_phase7

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Special rule for play.c to suppress termbox2 library warnings
commands/play.o: commands/play.c
	$(CC) $(CFLAGS) -Wno-unused-function -c -o $@ $<

# Special rule for uilib.o to suppress termbox2 library warnings
lib/uilib.o: lib/uilib.c
	$(CC) $(CFLAGS) -Wno-unused-function -c -o $@ $<

# Test targets
test/test_utils.o: test/test_utils.c test/test_utils.h
	$(CC) $(CFLAGS) -c -o $@ $<

test/test_lowpass: test/test_lowpass.c $(TEST_LIBS)
	$(CC) $(CFLAGS) -o $@ $< $(TEST_LIBS) -lm

test/test_trapezoid_rise: test/test_trapezoid_rise.c $(TEST_LIBS)
	$(CC) $(CFLAGS) -o $@ $< $(TEST_LIBS) -lm

test/test_leader_timing: test/test_leader_timing.c $(TEST_LIBS)
	$(CC) $(CFLAGS) -o $@ $< $(TEST_LIBS) -lm

test/test_wavlib_phase7: test/test_wavlib_phase7.c lib/wavlib.o lib/caslib.o
	$(CC) $(CFLAGS) -o $@ $< lib/wavlib.o lib/caslib.o -lm

test: $(TEST_PROGS)
	@echo "Running Audio Library Tests"
	@echo "============================"
	@echo ""
	@echo "=== Low-Pass Filter Test ==="
	@cd test && ./test_lowpass && echo "✓ PASSED" || echo "✗ FAILED"
	@echo ""
	@echo "=== Trapezoid Rise Time Test ==="
	@cd test && ./test_trapezoid_rise && echo "✓ PASSED" || echo "✗ FAILED"
	@echo ""
	@echo "=== Leader Timing Test ==="
	@cd test && ./test_leader_timing && echo "✓ PASSED" || echo "✗ FAILED"
	@echo ""
	@echo "=== WAV Cue Markers Test (Phase 7) ==="
	@if [ -f ../casfiles/disc.cas ]; then \
		./test/test_wavlib_phase7 ../casfiles/disc.cas test/test_disc_markers.wav && echo "✓ PASSED" || echo "✗ FAILED"; \
	else \
		echo "⊘ SKIPPED (no CAS file found at ../casfiles/disc.cas)"; \
	fi
	@echo ""
	@echo "============================"
	@echo "✓ All tests completed successfully!"
	@echo ""
	@echo "Generated test files:"
	@ls -lh test/*.wav 2>/dev/null | awk '{print "  " $$9 " (" $$5 ")"}'

clean:
	rm -f $(TARGET) $(OBJS) test/test_utils.o $(TEST_PROGS) test/*.wav

.PHONY: all clean test
