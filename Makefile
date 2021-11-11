#### Makefile for preliminary_parser.c 
#### ECE585 - Memory Controller Simulation Project
#### Michael Weston - Braden Harwood - Stephen Short - Drew Seidel

C = gcc
CFLAGS = -Wall -std=c99 -g
OBJS = memoryController.o mem_queue.o
HDRS = memoryController.h mem_queue.h

memoryController : $(OBJS)
	$(C) -o memoryController $(OBJS)

memoryController.o : memoryController.c
	$(C) $(CFLAGS) -c $(HDRS) memoryController.c

mem_queue.o : mem_queue.c
	$(C) $(CFLAGS) -c $(HDRS) mem_queue.c

clean:
	rm $(OBJS) memoryController.exe
