# An admittedly primitive Makefile
# To compile, type "make" or make "all"
# To remove files, type "make clean"

CC = gcc
CFLAGS = -Wall -pthread
OBJS = pzip.o pzip1.o io_helper.o 

.SUFFIXES: .c .o 

all: pzip pzip1

pzip: pzip.o io_helper.o 
	$(CC) $(CFLAGS) -o pzip pzip.o io_helper.o

pzip1: pzip1.o io_helper.o
	$(CC) $(CFLAGS) -o pzip1 pzip1.o io_helper.o

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm -f $(OBJS) pzip pzip1
