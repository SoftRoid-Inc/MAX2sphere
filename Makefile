CC = gcc
CFLAGS = -Wall -O3 
INCLUDES =
LFLAGS = 
LIBS = -ljpeg -lm 

OBJS = MAX2sphere.o bitmaplib.o
OBJS_precalc = MAX2sphere_precalc.o bitmaplib.o
OBJS_test = testc.o bitmaplib.o

all: MAX2sphere_precalc MAX2sphere

MAX2sphere_precalc: $(OBJS_precalc)
	$(CC) $(INCLUDES) $(CFLAGS) -o MAX2sphere_precalc $(OBJS_precalc) $(LFLAGS) $(LIBS)

MAX2sphere_precalc.o: MAX2sphere_precalc.c MAX2sphere.h
	$(CC) $(INCLUDES) $(CFLAGS) -c MAX2sphere_precalc.c
 
bitmaplib.o: bitmaplib.c bitmaplib.h
	$(CC) $(INCLUDES) $(CFLAGS) -c bitmaplib.c

clean:
	rm -rf core MAX2sphere_precalc $(OBJS_precalc)
	rm -rf core MAX2sphere $(OBJS)
	rm -rf core testc $(OBJS_test)

MAX2sphere: $(OBJS)
	$(CC) $(INCLUDES) $(CFLAGS) -o MAX2sphere $(OBJS) $(LFLAGS) $(LIBS)

MAX2sphere.o: MAX2sphere.c MAX2sphere.h
	$(CC) $(INCLUDES) $(CFLAGS) -c MAX2sphere.c

testc: $(OBJS_test)
	$(CC) $(INCLUDES) -fopenmp $(CFLAGS) -o testc $(OBJS_test) $(LFLAGS) $(LIBS)
testc.o: testc.c MAX2sphere.h
	$(CC) $(INCLUDES)  -fopenmp $(CFLAGS) -c testc.c
