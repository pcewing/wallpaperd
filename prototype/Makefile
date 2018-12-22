
CFLAGS_XCB=`pkg-config --cflags xcb`
LDFLAGS_XCB=`pkg-config --libs xcb`

CFLAGS_XCB_IMAGE=`pkg-config --cflags xcb-image`
LDFLAGS_XCB_IMAGE=`pkg-config --libs xcb-image`

DEBUG_LEVEL=-g3

CFLAGS_BASE=$(DEBUG_LEVEL) -Wall -Wextra -lm
CFLAGS=$(CFLAGS_BASE) $(CFLAGS_XCB) $(CFLAGS_XCB_IMAGE)

LDFLAGS=$(LDFLAGS_XCB) $(LDFLAGS_XCB_IMAGE)

all: test

test: clean
	cc -o test main.c $(CFLAGS) $(LDFLAGS)

test2: clean
	gcc -Wall -lxcb-icccm -lxcb -lxcb-image -o test2 test2.c

clean:
	rm -rf ./build

