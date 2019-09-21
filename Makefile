
SRC_DIR=./src
LIB_DIR=$(SRC_DIR)/lib
CONTRIB_DIR=$(SRC_DIR)/contrib

CFLAGS_SQLITE=`pkg-config --cflags sqlite3`
LDFLAGS_SQLITE=`pkg-config --libs sqlite3`

CFLAGS_XCB=`pkg-config --cflags xcb`
LDFLAGS_XCB=`pkg-config --libs xcb`

CFLAGS_XCB_IMAGE=`pkg-config --cflags xcb-image`
LDFLAGS_XCB_IMAGE=`pkg-config --libs xcb-image`

# TODO: Is there a better way to do this? pkg-config doesn't work
LDFLAGS_YAML=-lyaml

INCLUDES=-I$(LIB_DIR) -I$(CONTRIB_DIR)
CFLAGS_BASE=-g3 -Wall -Wextra $(INCLUDES)
CFLAGS=$(CFLAGS_BASE) $(CFLAGS_XCB) $(CFLAGS_XCB_IMAGE) $(CFLAGS_SQLITE)

LDFLAGS=-lm $(LDFLAGS_XCB) $(LDFLAGS_XCB_IMAGE) $(LDFLAGS_SQLITE) $(LDFLAGS_YAML)

all: wallpaperd

build_dir:
	mkdir -p build

objects: build_dir
	gcc -c -o build/error.o $(CFLAGS_BASE) $(LIB_DIR)/error.g.c
	gcc -c -o build/config.o $(CFLAGS_BASE) $(LIB_DIR)/config.c
	gcc -c -o build/log.o $(CFLAGS_BASE) $(LIB_DIR)/log.c
	gcc -c -o build/parse.o $(CFLAGS_BASE) $(LIB_DIR)/parse.c
	gcc -c -o build/core.o $(CFLAGS_BASE) $(LIB_DIR)/core.c
	gcc -c -o build/image.o $(CFLAGS_BASE) $(LIB_DIR)/image.c
	gcc -c -o build/ftw.o $(CFLAGS_BASE) $(LIB_DIR)/ftw.c 
	gcc -c -o build/wallpaper.o $(CFLAGS_BASE) $(LIB_DIR)/wallpaper.c
	gcc -c -o build/data.o $(CFLAGS_BASE) $(LIB_DIR)/data.c

wallpaperd: objects
	cc -o build/i3bgd $(SRC_DIR)/i3bgd/main.c build/*.o $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf ./build

