#make file for the server
OBJS = multiplexer.o  connection.o filetask.o
CC = gcc
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

all: $(OBJS)
	$(CC) $(OBJS) -o a1

multiplexer.o: multiplexer.c multiplexer.h boot.h
	$(CC) $(CFLAGS) multiplexer.c 

connection.o: connection.c connection.h boot.h
	$(CC) $(CFLAGS) connection.c 

filetask.o: filetask.c filetask.h boot.h
	$(CC) $(CFLAGS) filetask.c 
clean:
	rm -rf *~ *.o a1
