CC = gcc
CFLAGS = -Wall -O3 
INCLUDES =
LFLAGS = 
LIBS = -ljpeg -lm 

OBJS = MAX2spherev2.o bitmaplib.o

all: MAX2sphere

MAX2sphere: $(OBJS)
	$(CC) $(INCLUDES) $(CFLAGS) -o MAX2spherev2 $(OBJS) $(LFLAGS) $(LIBS)

MAX2sphere.o: MAX2sphere.c MAX2sphere.h
	$(CC) $(INCLUDES) $(CFLAGS) -c MAX2spherev2.c
 
bitmaplib.o: bitmaplib.c bitmaplib.h
	$(CC) $(INCLUDES) $(CFLAGS) -c bitmaplib.c

clean:
	rm -rf core MAX2sphere $(OBJS) 

