REV     ?= $(shell git rev-parse --short @{0})
STRIP   ?= strip
CFLAGS  ?= -std=c99 -pedantic -Wall -Wextra -DREV=\"$(REV)\"
TARGET  ?= petool
WINDRES ?= windres

ifdef DEBUG
CFLAGS  += -ggdb
else
CFLAGS  += -O2 -march=i486
endif

all: $(TARGET)

$(TARGET): $(wildcard src/*.c)
	$(WINDRES) -J rc petool.rc petool.rc.o
	$(CC) $(CFLAGS) -o $@ $^ petool.rc.o
	$(STRIP) -s $@

.PHONY: clean
clean:
	$(RM) $(TARGET)
