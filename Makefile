#### Makefile
#### ECE585 - Memory Controller Simulation Project
#### Michael Weston - Braden Harwood - Stephen Short - Drew Seidel

CC = gcc
CFLAGS = -Wall -std=c99 -g
OBJS = parser.o mem_queue.o mem_sim.o wrappers.o
LDFLAGS = -Wall -lm
SRC = src/parser/parser.c src/queueADT/mem_queue.c src/mem_sim.c src/wrappers/wrappers.c src/dimm/dimm.c src/dimm/group.c src/dimm/bank.c src/stats/stats.c src/stats/sListADT.c
HDRS = src/parser/parser.h src/queueADT/mem_queue.h src/wrappers/wrappers.h src/dimm/dimm.h src/dimm/group.h src/dimm/bank.h src/stats/stats.h src/stats/sListADT.h
EXE  = sim.exe

#.PHONY to inform Make to not associate all with a file named all
.PHONY: all
all: sim

sim : $(OBJS)
	$(CC) $(LDFLAGS) -o $(EXE) $(SRC)

mem_sim.o : src/mem_sim.c
	$(CC) $(CFLAGS) -c src/mem_sim.c

mem_queue.o : src/queueADT/mem_queue.c src/queueADT/mem_queue.h
	$(CC) $(CFLAGS) -c src/queueADT/mem_queue.c
	
parser.o : src/parser/parser.c src/parser/parser.h
	$(CC) $(CFLAGS) -c src/parser/parser.c

wrappers.o : src/wrappers/wrappers.c src/wrappers/wrappers.h
	$(CC) $(CFLAGS) -c src/wrappers/wrappers.c

verbose : $(OBJS)
	$(CC) $(CFLAGS) -DVERBOSE -o $(EXE) $(SRC)

# -DDEBUG will define DEBUG and recompile everything with DEBUG symbols enabled
debug : $(OBJS)
	$(CC) $(CFLAGS) -DDEBUG -DVERBOSE -o $(EXE) $(SRC)

#.PHONY to inform Make to not associate test with a file named test
.PHONY: test
# test runs a script that runs through all of the outputs and checks if the test case results have changed
test : sim
	./testing/test_output.sh

stat_comparison : testing/stat_comparison/stat_compare.c
	$(CC) $(CFLAGS) -o testing/stat_comparison/stat_compare testing/stat_comparison/stat_compare.c

#.PHONY to inform Make to not associate statistics with a file named statistics
.PHONY: statistics
statistics: sim stat_comparison
	 ./testing/stat_comparison/test_stat.sh

#.PHONY to inform Make to not associate help with a file named help
.PHONY: help
help:
	@echo "Targets available:"
	@echo "all       : Compiles the project. Run with ''./sim.exe -o outputfilename inputFilename'." 
	@echo "             Add '-opt' flag for optimization. Add '-stat' flag to show performance statistics."
	@echo "verbose   : Compiles the project the same as all, but with extra print statements during run time"
	@echo "debug     : Compiles the project the same as verbose, but with even more print statements during run time."
	@echo "test      : Compiles the project and then runs a test script that runs all of our test stimulus."
	@echo "             It then reports back what differences there are in the output versus the manually checked ground truth."
	@echo "statistics: Compiles the project and then runs a function to compute statistics comparisons given input cases"
	@echo "clean     : Deletes all of the object files and executable file to clean the directory"

#.PHONY to inform Make to not associate clean with a file named clean
.PHONY: clean
clean:
	@rm -f $(OBJS) sim.exe testing/stat_comparison/stat_compare.exe
