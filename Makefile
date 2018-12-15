DEBUG_LEVEL=-g3

INC=-I./inc
SRC=./src

CFLAGS=$(DEBUG_LEVEL) -Wall -Wextra `pkg-config --cflags glib-2.0`
LDFLAGS=`pkg-config --libs glib-2.0`

all: wallpaperd


log:
	gcc -c -o build/log.o $(INC) $(SRC)/log.c

util: log
	gcc -c -o build/util.o $(INC) $(SRC)/util.c

enumerate: util
	gcc -c -o build/enumerate.o $(INC) $(SRC)/enumerate.c

wallpaperd: enumerate
	cc -o build/wallpaperd $(INC) $(SRC)/main.c build/util.o build/log.o build/enumerate.o $(CFLAGS) $(LDFLAGS)

clean:
	rm -f build/wallpaperd build/util.o build/enumerate.o build/log.o

