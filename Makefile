CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = cast

SRCS = cast.c \
       commands/list.c \
       commands/export.c \
       commands/doctor.c \
       commands/convert.c \
       lib/caslib.c \
       lib/printlib.c \
       lib/cmdlib.c \
       lib/wavlib.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean
