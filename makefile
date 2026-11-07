CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L
TARGET  = analyse
SRCS    = main.c menu.c lecture.c tree.c operations.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

run: all
	./$(TARGET)

	