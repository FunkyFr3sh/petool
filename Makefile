-include config.mk

TARGET  ?= petool
REV     := $(shell git describe --match=NeVeRmAtCh --always --dirty)
CFLAGS  ?= -O2 -std=c99 -pedantic -Wall -Wextra -DREV=\"$(REV)\"
LDFLAGS ?= -s

ifeq ($(DESTDIR)$(PREFIX),)
	PREFIX := /usr/local
endif

ifneq ($(CROSS_COMPILE),)
CC       = $(CROSS_COMPILE)gcc
AS       = $(CROSS_COMPILE)as
STRIP    = $(CROSS_COMPILE)strip
endif

SRCS   := $(wildcard src/*.c) src/incbin.S
OBJS   := $(addsuffix .o, $(basename $(SRCS)))

.PHONY: clean all install
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	$(RM) $(TARGET) $(OBJS) || del $(TARGET) $(subst /,\\,$(OBJS))

install: $(TARGET)
	install $(TARGET) $(DESTDIR)$(PREFIX)/bin/

uninstall: 
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(TARGET)
