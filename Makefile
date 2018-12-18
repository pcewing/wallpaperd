INC=-I./inc
SRC=./src

CFLAGS_GLIB= `pkg-config --cflags glib-2.0`
LDFLAGS_GLIB=`pkg-config --libs glib-2.0`

CFLAGS_XLIB=`pkg-config --cflags x11`
LDFLAGS_XLIB=`pkg-config --libs x11`

CFLAGS_IMLIB2=`pkg-config --cflags imlib2`
LDFLAGS_IMLIB2=`pkg-config --libs imlib2`

DEBUG_LEVEL=-g3

CFLAGS_BASE=$(DEBUG_LEVEL) -Wall -Wextra
CFLAGS=$(CFLAGS_BASE) $(CFLAGS_XLIB) $(CFLAGS_IMLIB2)

LDFLAGS=$(LDFLAGS_XLIB) $(LDFLAGS_IMLIB2)

OBJECTS=build/core.o build/log.o build/enumerate.o build/wallpaper.o

all: wallpaperd

build_dir:
	mkdir -p build

log.o: build_dir
	gcc -c -o build/log.o $(INC) $(SRC)/log.c

core.o: build_dir
	gcc -c -o build/core.o $(INC) $(SRC)/core.c

enumerate.o: build_dir
	gcc -c -o build/enumerate.o $(INC) $(SRC)/enumerate.c

wallpaper.o: build_dir
	gcc -c -o build/wallpaper.o $(INC) -g3 $(SRC)/wallpaper.c

wallpaperd: build_dir enumerate.o log.o core.o wallpaper.o
	cc -o build/wallpaperd $(INC) $(SRC)/main.c $(OBJECTS) $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf ./build

