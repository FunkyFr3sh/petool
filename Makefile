.PHONY: clean all install

REV     ?= $(shell git rev-parse --short @{0})
STRIP   ?= strip
CFLAGS  ?= -std=c99 -pedantic -Wall -Wextra -DREV=\"$(REV)\"
TARGET  ?= petool

ifdef DEBUG
	CFLAGS  += -ggdb
else
	CFLAGS  += -O2
endif

ifeq ($(DESTDIR)$(PREFIX),)
	PREFIX := /usr/local
endif

all: $(TARGET)

$(TARGET): $(wildcard src/*.c) $(wildcard src/*.S)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) $(TARGET)

install: $(TARGET)
	install $(TARGET) $(DESTDIR)$(PREFIX)/bin/

uninstall: 
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(TARGET)
