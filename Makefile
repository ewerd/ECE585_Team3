#### Makefile
#### ECE585 - Memory Controller Simulation Project
#### Michael Weston - Braden Harwood - Stephen Short - Drew Seidel

CC = gcc
CFLAGS = -Wall -std=c99 -g
OBJS = parser.o mem_queue.o mem_sim.o
LDFLAGS = -lm
SRC = parser/parser.c queueADT/mem_queue.c mem_sim.c wrappers.c
HDRS = parser/parser.h queueADT/mem_queue.h wrappers.h
EXE  = sim.exe

sim : $(OBJS)
	$(CC) $(LDFLAGS) -o $(EXE) $(SRC)

mem_sim.o : mem_sim.c
	$(CC) $(CFLAGS) -c mem_sim.c

mem_queue.o : queueADT/mem_queue.c queueADT/mem_queue.h
	$(CC) $(CFLAGS) -c queueADT/mem_queue.c
	
parser.o : parser/parser.c parser/parser.h
	$(CC) $(CFLAGS) -c parser/parser.c

# -DDEBUG will define DEBUG and recompile everything with DEBUG symbols enabled
debug :
	$(CC) $(CFLAGS) -DDEBUG -o $(EXE) $(SRC)

clean:
	rm $(OBJS) sim.exe
