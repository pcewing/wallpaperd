
INC=-I./inc
SRC=./src

CFLAGS_SQLITE=`pkg-config --cflags sqlite3`
LDFLAGS_SQLITE=`pkg-config --libs sqlite3`

CFLAGS_XCB=`pkg-config --cflags xcb`
LDFLAGS_XCB=`pkg-config --libs xcb`

CFLAGS_XCB_IMAGE=`pkg-config --cflags xcb-image`
LDFLAGS_XCB_IMAGE=`pkg-config --libs xcb-image`

# TODO: Is there a better way to do this? pkg-config doesn't work
LDFLAGS_YAML=-lyaml

CFLAGS_BASE=-g3 -Wall -Wextra
CFLAGS=$(CFLAGS_BASE) $(CFLAGS_XCB) $(CFLAGS_XCB_IMAGE) $(CFLAGS_SQLITE)

LDFLAGS=-lm $(LDFLAGS_XCB) $(LDFLAGS_XCB_IMAGE) $(LDFLAGS_SQLITE) $(LDFLAGS_YAML)

all: wallpaperd

build_dir:
	mkdir -p build

config.o: build_dir
	gcc -c -o build/config.o $(INC) $(CFLAGS_BASE) $(SRC)/config.c

log.o: build_dir
	gcc -c -o build/log.o $(INC) $(CFLAGS_BASE) $(SRC)/log.c

core.o: build_dir
	gcc -c -o build/core.o $(INC) $(CFLAGS_BASE) $(SRC)/core.c

image.o: build_dir
	gcc -c -o build/image.o $(INC) $(CFLAGS_BASE) $(SRC)/image.c

ftw.o: build_dir
	gcc -c -o build/ftw.o $(INC) $(CFLAGS_BASE) $(SRC)/ftw.c 

wallpaper.o: build_dir
	gcc -c -o build/wallpaper.o $(INC) $(CFLAGS_BASE) $(SRC)/wallpaper.c

data.o: build_dir
	gcc -c -o build/data.o $(INC) $(CFLAGS_BASE) $(SRC)/data.c

wallpaperd: build_dir ftw.o log.o core.o wallpaper.o image.o data.o config.o
	cc -o build/wallpaperd $(INC) $(SRC)/main.c build/*.o $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf ./build

