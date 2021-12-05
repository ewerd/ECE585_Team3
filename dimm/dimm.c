/**
 * @file dimm.c 
 * @brief DIMM Level Memory Controller ADT source code file
 *
 * @detail	This is the source code file which contains the DIMM module level functionality.
 * 		It is the first part of the memory control simualtion and recursively reaches into
 * 		the group and bank levels of the simulator to determine the timing constraints
 * 		for various memory commands.
 * 		ECE 485/585 Final Project, Dr. Mark Faust
 * 		Portland State University, Fall 2021
 *
 * @date Presented December 6th, 2021
 *
 * @author	Stephen Short 	(steshort@pdx.edu)
 * @author	Drew Seidel	(dseidel@pdx.edu)
 * @author	Michael Weston	(miweston@pdx.edu)
 * @author	Braden Harwood 	(bharwood@pdx.edu)
 *
 *
 */

#include "dimm.h"

/*
 * Helper function declarations
 */
int dimm_checkArgs(dimm_t *dimm, unsigned group);

dimm_t *dimm_init(unsigned groups, unsigned banks, unsigned rows)
{
	// Allocate dimm_t object and subsequently its bank groups and banks
	dimm_t *newDimm = Malloc(sizeof(dimm_t));
	newDimm->group = Malloc(groups*sizeof(bGroup_t*));
	for (int i = 0; i < groups; i++)
	{
		newDimm->group[i]=group_init(banks, rows);	
	}
	newDimm->numGroups = groups;
	newDimm->nextWrite = 0;
	newDimm->nextRead = 0;
	newDimm->nextActivate = 0;
	return newDimm;
}

void dimm_deinit(dimm_t *dimm)
{
	// dimm isn't allocated, return
	if (dimm == NULL)
		return;
	
	// if dimm is allocated, deallocate or free() its groups first, then free() the actual dimm
	for (unsigned i = 0; i < dimm->numGroups; i++)
	{
		group_deinit(dimm->group[i]);
	}
	// free() in this order because groups are nested under dimm
	free(dimm->group);
	free(dimm);
}

uint8_t dimm_recoveryTime(memCmd_t firstOp, memCmd_t secOp)
{
	if (secOp == NONE || secOp == REMOVE)
	{
		return 0;
	}
	switch(firstOp)
	{
		case NONE:
		case REMOVE:
		return 0;
		
		case PRECHARGE:
		if (secOp == ACTIVATE)
			return D_PRECHARGE_TO_ACTIVATE;
		if (secOp == READ || secOp == WRITE)
			return D_PRECHARGE_TO_RW;
		if (secOp == PRECHARGE)
			return D_PRECHARGE_TO_PRECHARGE;
		case ACTIVATE:
		if (secOp == PRECHARGE)
			return D_ACTIVATE_TO_PRECHARGE;
		if (secOp == ACTIVATE)
			return D_ACTIVATE_TO_ACTIVATE;
		if (secOp == READ || secOp == WRITE)
			return D_ACTIVATE_TO_RW;
		case READ:
		if (secOp == PRECHARGE)
			return D_READ_TO_PRECHARGE;
		if (secOp == ACTIVATE)
			return D_READ_TO_ACTIVATE;
		if (secOp == READ)
			return D_READ_TO_READ;
		if (secOp == WRITE)
			return D_READ_TO_WRITE;
		case WRITE:
		if (secOp == PRECHARGE)
			return D_WRITE_TO_PRECHARGE;
		if (secOp == ACTIVATE)
			return D_WRITE_TO_ACTIVATE;
		if (secOp == READ)
			return D_WRITE_TO_READ;
		if (secOp == WRITE)
			return D_WRITE_TO_WRITE;
	}
	Fprintf(stderr, "Warning in dimm.dimm_recoveryTime(): Missing case for commands\n");
	return 0;
}

int dimm_canActivate(dimm_t *dimm, unsigned group, unsigned bank, unsigned long long currentTime)
{
	
	if (dimm_checkArgs(dimm, group) < 0)
	{
		Fprintf(stderr, "Error in dimm.dimm_canActivate(): Bad arguments.\n");
		return -2;
	}

	int groupActivateTime = group_canActivate(dimm->group[group],bank,currentTime); //Get time till group ready for ACT
	if (groupActivateTime < 0) //If group cannot ACT, return error code
	{
		return groupActivateTime;
	}

	int dimmActivateTime = (dimm->nextActivate > currentTime) ? dimm->nextActivate - currentTime : 0;
	//Return the larger of the two wait times, dimmActivateTime or groupActivate time
	return (dimmActivateTime > groupActivateTime) ? dimmActivateTime : groupActivateTime;
}

int dimm_activate(dimm_t *dimm, unsigned group, unsigned bank, unsigned row, unsigned long long currentTime)
{
	// checks for a valid dimm pointer and group number, will return error -2 if bad arguments
	if (dimm_checkArgs(dimm, group) < 0)
	{
		Fprintf(stderr, "Error in dimm.dimm_activate(): Bad arguments.\n");
		return -2;
	}
	
	if (currentTime < dimm->nextActivate)
	{
		Fprintf(stderr, "Error in dimm.dimm_activate(): Dimm not ready to activate. %llu cycles remain.\n",dimm->nextActivate - currentTime);
		return -1;
	}
	
	// dimm_activate calls group_activate which then calls bank_activate to check that a particular bank can be activated
	int activateTime = group_activate(dimm->group[group], bank, row, currentTime);
	if (activateTime > 0)
	{
        // minimum amount of time until dimm can be activated (assumes different bank group being activated)
		dimm->nextActivate = currentTime + TRRD_S * SCALE_FACTOR;
	}
	else
	{
		Fprintf(stderr, "Error in dimm.dimm_activate(): Could not activate group %u.\n", group);
	}
	return activateTime;
}

int dimm_canPrecharge(dimm_t *dimm, unsigned group, unsigned bank, unsigned long long currentTime)
{
	if (dimm_checkArgs(dimm, group) < 0)
	{
		Fprintf(stderr, "Error in dimm.dimm_canPrecharge(): Bad arguments.\n");
		return -2;
	}
    // calls group and bank "canPrecharge" functions to determine if desired bank can precharge
    // returns the largest of the next time a group of desired bank can precharge
	return group_canPrecharge(dimm->group[group], bank, currentTime);
}

int dimm_precharge(dimm_t *dimm, unsigned group, unsigned bank, unsigned long long currentTime)
{
	if (dimm_checkArgs(dimm, group < 0))
	{
		Fprintf(stderr, "Error in dimm.dimm_precharge(): Bad arguments.\n");
		return -2;	
	}

	// minimum time until the desired bank is done precharging
	int prechargeTime = group_precharge(dimm->group[group], bank, currentTime);
	if (prechargeTime < 0)
	{
		Fprintf(stderr, "Error in dimm.dimm_precharge(): Unable to precharge group %u.\n", group);
	}
	return prechargeTime; // return either error code or minimum time until desired bank is precharged
}

int dimm_canRead(dimm_t *dimm, unsigned group, unsigned bank, unsigned row, unsigned long long currentTime)
{
	if (dimm_checkArgs(dimm, group) < 0)
	{
		Fprintf(stderr, "Error in dimm.dimm_canRead(): Bad arguments.\n");
		return -2;	
	}
    
    // calls group and bank "canRead" functions to determine if desired bank row can be read from
	int groupReadTime = group_canRead(dimm->group[group], bank, row, currentTime);
	if (groupReadTime < 0)
	{
		return groupReadTime; // returns error code or minimum time until bank row can be read from
	}
	
    // minimum time until this dimm can perform another read command
	int dimmReadTime = (dimm->nextRead > currentTime) ? dimm->nextRead - currentTime : 0;
    // compares minimum time dimm can read to minimum time desired bank row can be read and returns the larger
	return (dimmReadTime < groupReadTime) ? groupReadTime : dimmReadTime;
}

int dimm_read(dimm_t *dimm, unsigned group, unsigned bank, unsigned row, unsigned long long currentTime)
{
	if (dimm_checkArgs(dimm, group < 0))
	{
		Fprintf(stderr, "Error in dimm.dimm_read(): Bad arguments.\n");
		return -2;	
	}

	
	if (currentTime < dimm->nextRead)
	{
		Fprintf(stderr, "Error in dimm.dimm_read(): Dimm not ready to read. %llu cycles remain.\n",dimm->nextRead - currentTime);
		return -1;
	}
    // time that it will take to perform read to desired bank row if possible
	int readTime = group_read(dimm->group[group], bank, row, currentTime);
	if (readTime > 0)
	{
        	// if row can be read right now, this is minimum time that dimm can possibly perform subsequent read
		dimm->nextRead = currentTime + D_READ_TO_READ;
		dimm->nextWrite = currentTime + D_READ_TO_WRITE;
	}
	else
	{
		Fprintf(stderr, "Error in dimm.dimm_read(): Could not read from group %u.\n", group);
	}

	return readTime; // returns error code
}

int dimm_canWrite(dimm_t *dimm, unsigned group, unsigned bank, unsigned row, unsigned long long currentTime)
{
	if (dimm_checkArgs(dimm, group < 0))
	{
		Fprintf(stderr, "Error in dimm.dimm_canWrite(): Bad arguments.\n");
		return -2;	
	}
    // calls group and desired bank "canWrite" functions to determine if/minimum time until group and bank can be written
	int groupWriteTime = group_canWrite(dimm->group[group], bank, row, currentTime);
	if (groupWriteTime < 0)
	{
		return groupWriteTime;
	}
	
    // time until this dimm can be written to again
	int dimmWriteTime = (dimm->nextWrite > currentTime) ? dimm->nextWrite - currentTime : 0;
    // compares the minimum time until group or dimm can be written to and returns the larger
	return (dimmWriteTime < groupWriteTime) ? groupWriteTime : dimmWriteTime;
}

int dimm_write(dimm_t *dimm, unsigned group, unsigned bank, unsigned row, unsigned long long currentTime)
{
	if (dimm_checkArgs(dimm, group < 0))
	{
		Fprintf(stderr, "Error in dimm.dimm_write(): Bad arguments.\n");
		return -2;	
	}

	if (currentTime < dimm->nextWrite)
	{
		Fprintf(stderr, "Error in dimm.dimm_write(): Dimm not ready to write. %llu cycles remain.\n",dimm->nextWrite - currentTime);
		return -1;
	}
    // time that a write to a desired bank row will take if possible
	int writeTime = group_write(dimm->group[group], bank, row, currentTime);
	if (writeTime > 0)
	{
        // write commenced, minimum time stamp that dimm can be written to again set
		dimm->nextWrite = currentTime + TCCD_S * SCALE_FACTOR;
        // minimum time stamp that dimm can be read from again set
		dimm->nextRead = currentTime + (CWL + TBURST + TWTR_S) * SCALE_FACTOR;
	}
	else
	{
		Fprintf(stderr, "Error in dimm.dimm_write(): Could not write to group %u.\n", group);
	}
	return writeTime; // returns time until data written to dimm / desired bank row
}


// ------------------------------------------------------Helper Functions-------------------------------------------------------------

/**
 * @fn		dimm_checkArgs
 * @brief	Checks for NULL pointers and out of bound group numbers
 *
 * @param	dimm	Pointer to a dimm struct
 * @param	group	Group number in that dimm struct
 * @return	-1 if an argument is bad. 0 otherwise.
 */
int dimm_checkArgs(dimm_t *dimm, unsigned group)
{
	if (dimm == NULL)
	{
		Fprintf(stderr, "Error in dimm.dimm_checkArgs(): NULL pointer passed.\n");
		return -1;
	}

	//Error checking that bGroup is in range
	if (group >= dimm->numGroups)
	{
		Fprintf(stderr, "Error in dimm.dimm_checkArgs(): Group index %u out of bounds.Last group is %u.\n",group,dimm->numGroups-1);
		return -1;
	}
	return 0;
}
