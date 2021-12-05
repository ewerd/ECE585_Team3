#### Makefile
#### ECE585 - Memory Controller Simulation Project
#### Michael Weston - Braden Harwood - Stephen Short - Drew Seidel

CC = gcc
CFLAGS = -Wall -std=c99 -g
OBJS = parser.o mem_queue.o mem_sim.o
LDFLAGS = -Wall -lm
SRC = parser/parser.c queueADT/mem_queue.c mem_sim.c wrappers.c dimm/dimm.c dimm/group.c dimm/bank.c stats/stats.c stats/sListADT.c
HDRS = parser/parser.h queueADT/mem_queue.h wrappers.h dimm/dimm.h dimm/group.h dimm/bank.h stats/stats.h stats/sListADT.h
EXE  = sim.exe

#.PHONY to inform Make to not associate all with a file named all
.PHONY: all
all: sim

sim : $(OBJS)
	$(CC) $(LDFLAGS) -o $(EXE) $(SRC)

mem_sim.o : mem_sim.c
	$(CC) $(CFLAGS) -c mem_sim.c

mem_queue.o : queueADT/mem_queue.c queueADT/mem_queue.h
	$(CC) $(CFLAGS) -c queueADT/mem_queue.c
	
parser.o : parser/parser.c parser/parser.h
	$(CC) $(CFLAGS) -c parser/parser.c

verbose : $(OBJS)
	$(CC) $(CFLAGS) -DVERBOSE -o $(EXE) $(SRC)

# -DDEBUG will define DEBUG and recompile everything with DEBUG symbols enabled
debug : $(OBJS)
	$(CC) $(CFLAGS) -DDEBUG -DVERBOSE -o $(EXE) $(SRC)

#.PHONY to inform Make to not associate test with a file named test
.PHONY: test
# test runs a script that runs through all of the outputs and checks if the test case results have changed
test: sim
	./testing/test_output.sh

#.PHONY to inform Make to not associate clean with a file named clean
.PHONY: clean
clean:
	@rm -f $(OBJS) sim.exe
