TARGETS = hw_malloc.o
CC ?= gcc
CFLAGS += -std=gnu99 -Wall -g
OBJS = $(TARGETS)

all: $(TARGETS)

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o

AS:
	astyle --style=kr --indent=spaces=4 hw_malloc.c
