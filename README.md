# ECE585_Team3
Team 3's final project for ECE 485/585
Portland State University, Fall 2021

Braden Harwood, Stephen Short, Michael Weston, Drew Seidel

*\*Information sourced from Dr. Faust's Final Project Description and Group Discussion*
## Table of Contents
- Objective
- Directory Organization
- Getting Started
- Usage
- Test Plan

## Objective
The purpose of this project was to simulate a memory controller as a part of a four core 3.2GHz processor employing a single channel memory. The memory controller communicates with a single rank 8GB PC4-25600 DIMM. For its most basic implementation, the memory controller utilizes bank parallelism and an open-page policy. Given an input text file of CPU requests (specifying time of request, type of request, and address needed of request), the simulator produces an output text file of the corresponding DRAM actions (precharge, activate, read or write), the CPU time at which they occurred, and corresponding DRAM location data when applicable (bank, bank group, row, column). 

The memory controller will be implemented using a queue that can hold up to 16 outstanding requests. For the basic implementation, the memory controller’s main objective is to prioritize the oldest request in the queue, while exploiting bank parallelism to peak at the remaining outstanding requests to determine what valid commands can be issued without violating timing constraints imposed by the DIMM. In order to make our memory controller the most efficient provided the given policies, we first had to make a few design choices. Then, in making sure our simulator was servicing CPU requests as efficiently as possible, we took into account DIMM timing constraints and tested methodically to ensure our memory controller was not violating constraints. 

## Directory Organization
- docs
  - Contains documentation relating to the project including our specifications document, edge cases tracking, test plan, and table of timing constraints.
- src
  - Contains all of our source files including the main program (mem_sim.c) and all of the various supporting ADTs, wrappers, and functions.
- testing
  - Contains the scripts and trace files used for testing the operation of our program.
  -   test_output.sh, a bash script used to run all of the test traces through our simulator and compare them to ground truth outputs.
  -   generate_ground_truth.sh, a bash script used to update these ground truth outputs.
  -   writeTrace, a utility program used to generate new trace files
  -   stat_comparison, a testing program used to compare the statistics of various test traces
  -   test_traces, the directory that holds all of our test case traces
  -   traces, the general purpose directory that holds all other traces used to test the output.

## Getting Started
1. Clone the repository to your computer
   ```sh
   git clone https://github.com/your_username_/ECE585_Team3.git
   ```
2. Run make to compile the project
   ```sh
   make
   ```
3. For a basic simulation, run the following:
   ```sh
   ./sim.exe testing/traces/trace.txt
   ```
   Output will be generated and sent to stdout
   
   For more advanced use, see the Usage section for different flags that can be set during runtime.
   A few examples:
   
   - Will run the simulator using strict in order execution.
   ```sh
   ./sim.exe -strict testing/traces/trace.txt
   ```
   
   - Will run the simulator using our optimized priority and out of order execution policies.
   ```sh
   ./sim.exe -opt testing/traces/trace.txt
   ```
  
   - Will run the simulator using the default scheduling policy.
   ```sh
   ./sim.exe testing/traces/trace.txt
   ```
   
   - Will run the simulator as described above with the optimized policy and will also print out statistics on reads, writes, and instruction fetches at the end of the output.
   ```sh
   ./sim.exe -stat -opt testing/traces/trace.txt
   ```
   
   
## Usage
### sim.exe
The simulation program sim.exe has the following flags:
- -o <output_file>  Sends output to a .txt file. If output_file is blank, will default to output.txt
- -stat             Displays statistics after completing the simulation. Output will be send to stdout. Statistics include min, max, average, and median time in queue for each type of operation as well as for the aggregate total for all commands.
- -\<policy>
  - strict  : The memory controller sticks to a strict in order scheduling fetches, reads, and writes will be serviced in the exact order that they arrive.
  - opt     : Our optimized policy. This algorithm prioritizes fetches, reads, and writes (in that order) to open rows, then fetches, reads, and writes(in that order) to other rows. To prevent starvation, each operation type has an upper threshold for time in queue and, once a request passes that threshold, it moves to the highest priority. Lower priority requests may be serviced earlier than higher priority requests but only if doing so does not slow down higher priority requests in any way. The thresholds can be set by the user with the -<fch/rd/wr> <threshold> flags or the default ones will be used.
  - \<none> : This is the default policy which gives priority to the oldest requests in the queue. Newer requests can still be serviced sooner but only if processing those requests does not slow down any older request.
- Other than these flags, an input file must be provided and must meet the specifications described in docs/specifications.md.

### make
make also has additional targets:
- \<none>    : No target provided defaults to the basic compilation of all
- all        : Compiles the simulator sim.exe
- verbose    : Compiles the simulator sim.exe but adds additional text to the output to give more information about what is happening.
- debug      : Compiles the simulator sim.exe and adds all possible additional text to the output for debugging purposes.
- test       : Runs the simulator sim.exe with the test script (test_output.sh) that runs all of the test traces through the simulator and compares their output to the ground truth traces. No output other than the description of the test means that the test output was the same as the ground truth. This is useful when a change is made to the code as it will check to see what tests generated different results.
- statistics : Runs the simulator sim.exe with the statistics test script (test_stat.sh) that runs multiple traces through the simulator and prints out just their statistics for comparison.
- clean      : Removes all object files and the executable generated by make to clean the directory.

## Test Plan
To thoroughly test our simulator, we came up with a large number of simple trace files that are meant to test one or more memory timing constraints at a time. After confirming that no timing constraints were being violated, we then automated this testing process so that additional changes to the program could be tested to ensure that previously confirmed constraints were still not being violated.

In the docs/test_plan.md file, you will see the various tests we decided to do, a few we chose not to based on other assumptions, and a summary of what parameters are tested by each trace file.
