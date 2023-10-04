REV     ?= $(shell git rev-parse --short @{0})
STRIP   ?= strip
CFLAGS  ?= -std=c99 -pedantic -Wall -Wextra -DREV=\"$(REV)\"
TARGET  ?= petool

ifdef DEBUG
CFLAGS  += -ggdb
else
CFLAGS  += -O2 -march=i486
endif

all: $(TARGET)

$(TARGET): $(wildcard src/*.c)
ifneq (,$(WINDRES))
	$(WINDRES) -J rc petool.rc petool.rc.o
	$(CC) $(CFLAGS) -o $@ $^ petool.rc.o
else
	$(CC) $(CFLAGS) -o $@ $^
endif
	$(STRIP) -s $@

.PHONY: clean
clean:
	$(RM) $(TARGET)
