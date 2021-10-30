#### Makefile for preliminary_parser.c 
#### ECE585 - Memory Controller Simulation Project
#### Michael Weston - Braden Harwood - Stephen Short - Drew Seidel

C = gcc
CFLAGS = -Wall -std=c99 -g
OBJS = preliminary_parser.o
#### HDRS = 

preliminary_parser : $(OBJS)
	$(C) -o preliminary_parser $(OBJS)

preliminary_parser.o : preliminary_parser.c
	$(C) $(CFLAGS) -c preliminary_parser.c

clean:
	rm $(OBJS) preliminary_parser.exe
