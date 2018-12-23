
# Call with GTKVERSION=3 to generate a GTK3 version
# Not really tested. Don't be afraid of warning list when building
ifeq ($(GTKVERSION),3)
	GTKLDFLAGS=$(shell pkg-config --libs gtk+-3.0)
	GTKCFLAGS=$(shell pkg-config --cflags gtk+-3.0)
else
	GTKLDFLAGS=$(shell pkg-config --libs gtk+-2.0)
	GTKCFLAGS=$(shell pkg-config --cflags gtk+-2.0)
endif

CFLAGS:=-Wall -Wextra -Wconversion -O2
LDFLAGS:=-s -lm

CC=gcc
LD=gcc

default: bin/dprpwg-gtk

all: bin/dprpwg-gtk

clean distclean:
	rm -rf bin build

bin/dprpwg-gtk: build/dprpwg-gtk.o build/dprpwg_lib.o
	mkdir -p bin
	$(LD) -o $@ $^ $(LDFLAGS) $(GTKLDFLAGS)

build/dprpwg-gtk.o: src/dprpwg-gtk.c
	mkdir -p build
	$(CC) -c $(CFLAGS) $(GTKCFLAGS) -o $@ $^

build/dprpwg_lib.o: src/dprpwg_lib.c
	mkdir -p build
	$(CC) -c $(CFLAGS) -o $@ $^
