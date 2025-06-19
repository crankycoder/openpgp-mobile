# Examples Makefile

CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c99 -g
LDFLAGS ?= -L../lib -lopenpgp -ldl
INCLUDES = -I../include

# Example programs
EXAMPLES = key_operations

all: $(EXAMPLES)

key_operations: key_operations.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(EXAMPLES)

.PHONY: all clean