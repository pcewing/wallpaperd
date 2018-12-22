
INC=-I./inc
SRC=./src

CFLAGS_GLIB= `pkg-config --cflags glib-2.0`
LDFLAGS_GLIB=`pkg-config --libs glib-2.0`

CFLAGS_XCB=`pkg-config --cflags xcb`
LDFLAGS_XCB=`pkg-config --libs xcb`

CFLAGS_XCB_IMAGE=`pkg-config --cflags xcb-image`
LDFLAGS_XCB_IMAGE=`pkg-config --libs xcb-image`

DEBUG_LEVEL=-g3

CFLAGS_BASE=$(DEBUG_LEVEL) -Wall -Wextra -lm
CFLAGS=$(CFLAGS_BASE) $(CFLAGS_XCB) $(CFLAGS_XCB_IMAGE)

LDFLAGS=$(LDFLAGS_XCB) $(LDFLAGS_XCB_IMAGE)

OBJECTS=build/core.o build/log.o build/enumerate.o build/wallpaper.o build/image.o

all: wallpaperd

build_dir:
	mkdir -p build

log.o: build_dir
	gcc -c -o build/log.o $(INC) $(SRC)/log.c

core.o: build_dir
	gcc -c -o build/core.o $(INC) $(SRC)/core.c

image.o: build_dir
	gcc -c -o build/image.o $(INC) $(SRC)/image.c

enumerate.o: build_dir
	gcc -c -o build/enumerate.o $(INC) $(SRC)/enumerate.c

wallpaper.o: build_dir
	gcc -c -o build/wallpaper.o $(INC) -g3 $(SRC)/wallpaper.c

wallpaperd: build_dir enumerate.o log.o core.o wallpaper.o image.o
	cc -o build/wallpaperd $(INC) $(SRC)/main.c $(OBJECTS) $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf ./build

