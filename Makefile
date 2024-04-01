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

all: $(TARGET)

$(TARGET): $(wildcard src/*.c) $(wildcard src/*.S)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) $(TARGET)

install: $(TARGET)
	install -Dt $(DESTDIR)$(PREFIX)/bin/ $(TARGET)

uninstall: 
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(TARGET)
