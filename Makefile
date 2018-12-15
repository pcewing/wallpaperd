DEBUG_LEVEL=-g3

INC=-I./inc
SRC=./src

CFLAGS=$(DEBUG_LEVEL) -Wall -Wextra `pkg-config --cflags glib-2.0`
LDFLAGS=`pkg-config --libs glib-2.0`

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
	gcc -c -o build/wallpaper.o $(INC) $(SRC)/wallpaper.c

image.o: build_dir
	gcc -c -o build/image.o $(INC) $(SRC)/image.c

wallpaperd: build_dir enumerate.o log.o util.o wallpaper.o image.o
	cc -o build/wallpaperd $(INC) $(SRC)/main.c $(OBJECTS) $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf ./build

