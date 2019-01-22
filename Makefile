
INC=-I./inc
SRC=./src

CFLAGS_SQLITE= `pkg-config --cflags sqlite3`
LDFLAGS_SQLITE=`pkg-config --libs sqlite3`

CFLAGS_XCB=`pkg-config --cflags xcb`
LDFLAGS_XCB=`pkg-config --libs xcb`

CFLAGS_XCB_IMAGE=`pkg-config --cflags xcb-image`
LDFLAGS_XCB_IMAGE=`pkg-config --libs xcb-image`

DEBUG_LEVEL=-g3

CFLAGS_BASE=$(DEBUG_LEVEL) -Wall -Wextra -lm
CFLAGS=$(CFLAGS_BASE) $(CFLAGS_XCB) $(CFLAGS_XCB_IMAGE) $(CFLAGS_SQLITE)

LDFLAGS=$(LDFLAGS_XCB) $(LDFLAGS_XCB_IMAGE) $(LDFLAGS_SQLITE)

OBJECTS=build/core.o build/log.o build/ftw.o build/wallpaper.o build/image.o build/data.o

all: wallpaperd

build_dir:
	mkdir -p build

log.o: build_dir
	gcc -c -o build/log.o $(INC) -g3 $(SRC)/log.c

core.o: build_dir
	gcc -c -o build/core.o $(INC) -g3 $(SRC)/core.c

image.o: build_dir
	gcc -c -o build/image.o $(INC) -g3 $(SRC)/image.c

ftw.o: build_dir
	gcc -c -o build/ftw.o $(INC) -g3 $(SRC)/ftw.c 

wallpaper.o: build_dir
	gcc -c -o build/wallpaper.o $(INC) -g3 $(SRC)/wallpaper.c

data.o: build_dir
	gcc -c -o build/data.o $(INC) -g3 $(SRC)/data.c

wallpaperd: build_dir ftw.o log.o core.o wallpaper.o image.o data.o
	cc -o build/wallpaperd $(INC) $(SRC)/main.c $(OBJECTS) $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf ./build

