CC = gcc
CFLAGS = -Wall -O3 
INCLUDES =
LFLAGS = 
LIBS = -ljpeg -lm 

OBJS = MAX2sphere.o bitmaplib.o
OBJS_precalc = MAX2sphere_precalc.o bitmaplib.o

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

MAX2sphere: $(OBJS)
	$(CC) $(INCLUDES) $(CFLAGS) -o MAX2sphere $(OBJS) $(LFLAGS) $(LIBS)

MAX2sphere.o: MAX2sphere.c MAX2sphere.h
	$(CC) $(INCLUDES) $(CFLAGS) -c MAX2sphere.c

