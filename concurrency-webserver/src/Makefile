# An admittedly primitive Makefile
# To compile, type "make" or make "all"
# To remove files, type "make clean"

CC = gcc
CFLAGS = -Wall -pthread
OBJS = wserver.o wclient.o wclient_m.o request.o io_helper.o 

.SUFFIXES: .c .o 

all: wserver wclient wclient_m spin.cgi

wserver: wserver.o request.o io_helper.o 
	$(CC) $(CFLAGS) -o wserver wserver.o request.o io_helper.o

wclient: wclient.o io_helper.o
	$(CC) $(CFLAGS) -o wclient wclient.o io_helper.o

wclient_m: wclient_m.o io_helper.o
	$(CC) $(CFLAGS) -o wclient_m wclient_m.o io_helper.o

spin.cgi: spin.c
	$(CC) $(CFLAGS) -o spin.cgi spin.c

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm -f $(OBJS) wserver wclient spin.cgi
