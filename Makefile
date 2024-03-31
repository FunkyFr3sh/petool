REV     ?= $(shell git rev-parse --short @{0})
STRIP   ?= strip
CFLAGS  ?= -std=c99 -pedantic -Wall -Wextra -DREV=\"$(REV)\"
TARGET  ?= petool

ifdef DEBUG
CFLAGS  += -ggdb
else
CFLAGS  += -O2
endif

all: $(TARGET)

$(TARGET): $(wildcard src/*.c) $(wildcard src/*.S)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	$(RM) $(TARGET)
