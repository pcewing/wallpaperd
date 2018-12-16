INC=-I./inc
SRC=./src

CFLAGS_GLIB= `pkg-config --cflags glib-2.0`
LDFLAGS_GLIB=`pkg-config --libs glib-2.0`

CFLAGS_MAGICK_WAND=`pkg-config --cflags MagickWand`
LDFLAGS_MAGICK_WAND=`pkg-config --libs MagickWand`

CFLAGS_XLIB=`pkg-config --cflags x11`
LDFLAGS_XLIB=`pkg-config --libs x11`

DEBUG_LEVEL=-g3

CFLAGS_BASE=$(DEBUG_LEVEL) -Wall -Wextra
CFLAGS=$(CFLAGS_BASE) $(CFLAGS_MAGICK_WAND) $(CFLAGS_XLIB)

LDFLAGS=$(LDFLAGS_MAGICK_WAND) $(LDFLAGS_XLIB)

OBJECTS=build/util.o build/log.o build/enumerate.o build/wallpaper.o build/image.o

all: wallpaperd

build_dir:
	mkdir -p build

log.o: build_dir
	gcc -c -o build/log.o $(INC) $(SRC)/log.c

util.o: build_dir
	gcc -c -o build/util.o $(INC) $(SRC)/util.c

enumerate.o: build_dir
	gcc -c -o build/enumerate.o $(INC) $(SRC)/enumerate.c

wallpaper.o: build_dir
	gcc -c -o build/wallpaper.o $(INC) -g3 $(SRC)/wallpaper.c

image.o: build_dir
	gcc -c -o build/image.o $(SRC)/image.c $(INC) $(CFLAGS) $(LDFLAGS)

wallpaperd: build_dir enumerate.o log.o util.o wallpaper.o image.o
	cc -o build/wallpaperd $(INC) $(SRC)/main.c $(OBJECTS) $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf ./build

