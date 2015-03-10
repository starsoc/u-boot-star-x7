export CROSS_COMPILE=/opt/altera-linux/linaro/gcc-linaro-arm-linux-gnueabihf-4.7-2012.11-20121123_linux/bin/arm-linux-gnueabihf-

CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -g

all:	mathlib

mathlib: mathlib.o 
	$(CC) $(CFLAGS) -o $@ mathlib.c $(LIBS) -lrt -lm -static

			
clean:
	rm -f mathlib *.o
