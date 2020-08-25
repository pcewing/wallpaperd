
SRC_DIR=./src
LIB_DIR=$(SRC_DIR)/lib
CONTRIB_DIR=$(SRC_DIR)/contrib
GEN_DIR=$(SRC_DIR)/schema/generated
BUILD_DIR=./build

CFLAGS_SQLITE=`pkg-config --cflags sqlite3`
LDFLAGS_SQLITE=`pkg-config --libs sqlite3`

CFLAGS_XCB=`pkg-config --cflags xcb`
LDFLAGS_XCB=`pkg-config --libs xcb`

CFLAGS_XCB_IMAGE=`pkg-config --cflags xcb-image`
LDFLAGS_XCB_IMAGE=`pkg-config --libs xcb-image`

# TODO: This should be either committed or made a sub-repository
CFLAGS_FLATCC=-I$(BUILD_DIR)/flatcc/include
LDFLAGS_FLATCC=-L$(BUILD_DIR)/flatcc/lib -lflatcc -lflatccrt

LDFLAGS_YAML=-lyaml

INCLUDES=-I$(LIB_DIR) -I$(CONTRIB_DIR) -I$(GEN_DIR)
CFLAGS_BASE=-g3 -Wall -Wextra -pthread $(INCLUDES)
CFLAGS=$(CFLAGS_BASE) $(CFLAGS_SQLITE) $(CFLAGS_XCB) $(CFLAGS_XCB_IMAGE) $(CFLAGS_FLATCC)

LDFLAGS=-lm $(LDFLAGS_SQLITE) $(LDFLAGS_XCB) $(LDFLAGS_XCB_IMAGE) $(LDFLAGS_FLATCC) $(LDFLAGS_YAML)

all: wallpaperd

build_dir:
	mkdir -p $(BUILD_DIR)

codegen: build_dir
	tool/codegen.py
	tool/flatbuffers.sh

objects: codegen
	gcc -c -o $(BUILD_DIR)/error.o $(CFLAGS) $(LIB_DIR)/error.g.c
	gcc -c -o $(BUILD_DIR)/config.o $(CFLAGS) $(LIB_DIR)/config.c
	gcc -c -o $(BUILD_DIR)/log.o $(CFLAGS) $(LIB_DIR)/log.c
	gcc -c -o $(BUILD_DIR)/parse.o $(CFLAGS) $(LIB_DIR)/parse.c
	gcc -c -o $(BUILD_DIR)/core.o $(CFLAGS) $(LIB_DIR)/core.c
	gcc -c -o $(BUILD_DIR)/image.o $(CFLAGS) $(LIB_DIR)/image.c
	gcc -c -o $(BUILD_DIR)/ftw.o $(CFLAGS) $(LIB_DIR)/ftw.c 
	gcc -c -o $(BUILD_DIR)/wallpaper.o $(CFLAGS) $(LIB_DIR)/wallpaper.c
	gcc -c -o $(BUILD_DIR)/data.o $(CFLAGS) $(LIB_DIR)/data.c
	gcc -c -o $(BUILD_DIR)/ipc.o $(CFLAGS) $(LIB_DIR)/ipc.c
	gcc -c -o $(BUILD_DIR)/rpc.o $(CFLAGS) $(LIB_DIR)/rpc.c

wallpaperd: objects
	cc -o $(BUILD_DIR)/i3bgd $(SRC_DIR)/i3bgd/main.c $(BUILD_DIR)/*.o $(CFLAGS) $(LDFLAGS)
	cc -o $(BUILD_DIR)/i3bg $(SRC_DIR)/i3bg/main.c $(BUILD_DIR)/*.o $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)

