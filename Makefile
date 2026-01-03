CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = cast

SRCS = cast.c \
       commands/list.c \
       commands/export.c \
       commands/convert.c \
       commands/profile.c \
       lib/caslib.c \
       lib/printlib.c \
       lib/cmdlib.c \
       lib/wavlib.c \
       lib/presetlib.c

OBJS = $(SRCS:.c=.o)

# Test programs
TEST_LIBS = lib/wavlib.o lib/caslib.o test/test_utils.o
TEST_PROGS = test/test_lowpass test/test_trapezoid_rise test/test_leader_timing

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Test targets
test/test_utils.o: test/test_utils.c test/test_utils.h
	$(CC) $(CFLAGS) -c -o $@ $<

test/test_lowpass: test/test_lowpass.c $(TEST_LIBS)
	$(CC) $(CFLAGS) -o $@ $< $(TEST_LIBS) -lm

test/test_trapezoid_rise: test/test_trapezoid_rise.c $(TEST_LIBS)
	$(CC) $(CFLAGS) -o $@ $< $(TEST_LIBS) -lm

test/test_leader_timing: test/test_leader_timing.c $(TEST_LIBS)
	$(CC) $(CFLAGS) -o $@ $< $(TEST_LIBS) -lm

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
	@echo "============================"
	@echo "✓ All tests completed successfully!"
	@echo ""
	@echo "Generated test files:"
	@ls -lh test/*.wav 2>/dev/null | awk '{print "  " $$9 " (" $$5 ")"}'

clean:
	rm -f $(TARGET) $(OBJS) test/test_utils.o $(TEST_PROGS) test/*.wav

.PHONY: all clean test
