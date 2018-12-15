
DEBUG_LEVEL=-g3
INCLUDE=-I./inc/ 
CFLAGS=$(DEBUG_LEVEL) -Wall -Wextra `pkg-config --cflags glib-2.0`
LDFLAGS=`pkg-config --libs glib-2.0`

log:
	gcc -c -o build/log.o $(INCLUDE) src/log.c

util: log
	gcc -c -o build/util.o $(INCLUDE) src/util.c

enumerate: util
	gcc -c -o build/enumerate.o $(INCLUDE) src/enumerate.c

wpr: enumerate
	cc -o build/wpr $(INCLUDE) src/wpr.c build/util.o build/log.o build/enumerate.o $(CFLAGS) $(LDFLAGS)

clean:
	rm -f build/wpr build/util.o build/enumerate.o build/log.o

