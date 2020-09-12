# An admittedly primitive Makefile
# To compile, type "make" or make "all"
# To remove files, type "make clean"

CC = gcc
CFLAGS = -Wall -pthread
OBJS = mapreduce.o io_helper.o 

.SUFFIXES: .c .o 

all: mapreduce

pzip: mapreduce.o io_helper.o 
	$(CC) $(CFLAGS) -o mapreduce mapreduce.o io_helper.o

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm -f $(OBJS) mapreduce x_intermediate.*
