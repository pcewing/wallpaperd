
DEBUG_LEVEL=-g3
CFLAGS=$(DEBUG_LEVEL) -Wall -Wextra `pkg-config --cflags glib-2.0`
LDFLAGS=`pkg-config --libs glib-2.0`

log:
	gcc -c -o log.o log.c

util: log
	gcc -c -o util.o util.c

enumerate: util
	gcc -c -o enumerate.o enumerate.c

wpr: enumerate
	cc -o wpr wpr.c util.o log.o enumerate.o $(CFLAGS) $(LDFLAGS)

clean:
	rm -f wpr util.o enumerate.o log.o

