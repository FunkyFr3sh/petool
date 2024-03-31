REV     ?= $(shell git rev-parse --short @{0})
UNAME   ?= $(shell uname)
STRIP   ?= strip
CFLAGS  ?= -std=c99 -pedantic -Wall -Wextra -DREV=\"$(REV)\"
TARGET  ?= petool

ifeq ($(UNAME), Darwin)
CFLAGS  += -target x86_64-apple-darwin
endif

ifdef DEBUG
CFLAGS  += -ggdb
else
CFLAGS  += -O2
endif

all: $(TARGET)

$(TARGET): $(wildcard src/*.c)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	$(RM) $(TARGET)
