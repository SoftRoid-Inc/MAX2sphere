CC = gcc
CFLAGS = -Wall -O3 
INCLUDES =
LFLAGS = 
LIBS = -ljpeg -lm 

OBJS = MAX2sphere.o bitmaplib.o

all: MAX2sphere

MAX2sphere: $(OBJS)
	$(CC) $(INCLUDES) $(CFLAGS) -o MAX2sphere $(OBJS) $(LFLAGS) $(LIBS)

MAX2sphere.o: MAX2sphere.c MAX2sphere.h
	$(CC) $(INCLUDES) $(CFLAGS) -c MAX2sphere.c
 
bitmaplib.o: bitmaplib.c bitmaplib.h
	$(CC) $(INCLUDES) $(CFLAGS) -c bitmaplib.c

clean:
	rm -rf core MAX2sphere $(OBJS) 
