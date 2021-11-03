/**
 * @file	mem_sim.c
 * @brief	Top level memory simulator for ECE 585 final project.
 *
 * @detail	TODO
 *
 * @date	TODO
 * @authors	TODO
 */



int main(int argc, char** argv)
{
	//Parse arguments if we're passing them on the command line
	
	//Init parser

	//Init queue

	//Init DIMM

	//Main operating loop

	//While queue isn't empty, ping parser for available requests at current time
	//Add each line to back of queue

	//For basic implementation, get bank group, bank, row of top request

	//Ask DIMM if that bank group and bank are ready for a command
	//Determine what next command should be and issue it to DIMM
	//If data is ready, remove request from queue and...print message? Check project description for what needs to happen

	//Determine time until DIMM is ready for next command
	//Determine time until next request arrives from parser
	//Advance time by the smaller of the two times

	//Loop back to start of MOL
}
