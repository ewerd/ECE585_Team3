/**
 * @file	dimm.h
 * @brief	Abstraction of a DIMM
 *
 * @detail	TODO DIMM contains 4 bank groups of 4 banks each and 256 rows per bank.
 *
 * @date	TODO
 * @authors	TODO
 */

#ifndef DIMM_H_
#define DIMM_H_

#include "group.h"
#include "bank.h"
#include "../wrappers.h"

#define TRRD_S		4 //Time between row activations in different bank groups
#define SCALE_FACTOR	2 //Ratio of CPU clock speed to MEM clock speed

typedef struct {
	bGroup_t**		group; //Array of groups
	unsigned 		numGroups; //Number of groups in the dimm
	unsigned long long	nextWrite; //Time available for next write(tCCD_S)
	unsigned long long	nextRead; //Time available for next read(tCCD_S,tWTR_S)
	unsigned long long	nextActivate; //Time available for next ACT command(tRRD_S)
} dimm_t;

/**
 * @fn		dimm_init
 * @brief	Initializes a DIMM and returns a pointer to it
 *
 * @detail	The DIMM will start with all banks precharged and ready for activation
 * @param	groups	Number of bank groups
 * @param	banks	Number of banks per group
 * @param	rows	Number of rows per bank
 * @returns	Pointer to new dimm_t struct
 */
dimm_t *dimm_init(unsigned groups, unsigned banks, unsigned rows);


/**
 * @fn		dimm_deinit
 * @brief	Frees all dynamic memory used by a dimm_t
 *
 * @param	dimm	Target dimm to free
 */
void dimm_deinit(dimm_t *dimm);

/**
 * @fn		dimm_canActivate
 * @brief	Checks if the specified bank can start a row activation
 *
 * @details	Verifies that the bank is already precharged or is precharging and returns
 *		the time till completion.
 * @param	dimm	Pointer to the dimm struct
 * @param	bGroup	The bank group that contains the bank.
 * @param	bank	The bank number within the bank group.
 * @param	currentTime	The current simulation time (in CPU clock cycles)
 * @return	0 if the bank is precharged and ready for activation. A positive integer
 *		that is the time remaining if the bank is in the process of precharging.
 *		-2 if group or bank is out of bounds. -1 otherwise.
 */
int dimm_canActivate(dimm_t *dimm, unsigned group, unsigned bank, unsigned long long currentTime);

/**
 * @fn		dimm_activate
 * @brief	Issues an activate command to a bank
 *
 * @details	Verifies that bank is available for an activate command and then issues it.
 * @param	dimm	Pointer to dimm receiving command
 * @param	group	Bank group
 * @param	bank	The bank being activated
 * @param	row	The row to activate
 * @return	If the command is issued successfully, returns time(in CPU cycles) until the 
 *		activate command is completed. -1 otherwise.
 */
int dimm_activate(dimm_t *dimm, unsigned group, unsigned bank, unsigned row, unsigned long long currentTime);

/**
 * @fn		dimm_canPrecharge
 * @brief	Checks if the specified bank can receive a precharge command
 *
 * @detail	Verifies that the bank is idle or calculates how much tRTP since the last 
 *		read, tWR since the last write, or tRAS time remains.
 * @param	dimm	Pointer to dimm struct
 * @param	bGroup	The bank group that contains the bank
 * @param	bank	The bank number within the group.
 * @param	currentTime	The current simulation time
 * @return	0 if the precharge command can be issued. A positive integer if some definite
 *		time remains until precharge can be issued. -1 otherwise.
 */
int dimm_canPrecharge(dimm_t *dimm, unsigned group, unsigned bank, unsigned long long currentTime);

/**
 * @fn		dimm_precharge
 * @brief	Precharges a bank
 *
 * @detail	Verifies the bank is available for precharging and issues the command.
 * @param	dimm	Pointer to dimm struct
 * @param	group	Bank group
 * @param	bank	Target bank
 * @param	currentTime	Current simulation time
 * @return	If the command is issued, some positive integer that is the time in CPU cycles
 *		until the command is completed. -2 if bad arguments are passed. -1 otherwise
 */
int dimm_precharge(dimm_t *dimm, unsigned group, unsigned bank, unsigned long long currentTime);

/**
 * @fn		dimm_canRead
 * @brief	Checks if the specified bank can receive a read command
 *
 * @detail	Verifies that the bank has activated the correct row or calculates how much 
 *		time remains until that row completes activation. Also will check that the
 *		appopriate amount of time has elapsed since the previous read (CL) or 
 *		write(tWR&tWTR).
 * @param	bGroup	The bank group that contains the bank
 * @param	bank	The bank number within the group.
 * @param	row	The row to be read from.
 * @return	0 if the read command can be issued. A positive integer if some definite
 *		time remains until read can be issued. -1 otherwise.
 */
int dimm_canRead(int bGroup, int bank, int row);

/**
 * @fn		dimm_read
 * @brief	Issues a read command to a bank
 *
 * @detail	Verifies that the read command is legal and issues it to the target bank.
 * @param	bGroup	Bank group
 * @param	bank	Target bank
 * @param	row	Target row
 * @param	col	Target column
 * @return	0 if the command is issued. A positive integer if some definite time remains
 *		until it can be issued. -1 otherwise.
 */
int dimm_read(int bGroup, int bank, int row, int col);

/**
 * @fn		dimm_canWrite
 * @brief	Checks if the specified bank can receive a write command
 *
 * @detail	Verifies that the bank has activated the correct row and can write. If not,
 *		calculates the time till activation finishes or previous write completes.
 * @param	bGroup	The bank group that contains the bank
 * @param	bank	The bank number within the group.
 * @param	row	The row to be written to.
 * @return	0 if the write command can be issued. A positive integer if some definite
 *		time remains until write can be issued. -1 otherwise.
 */
int dimm_canWrite(int bGroup, int bank, int row);

/**
 * @fn		dimm_write
 * @brief	Issues a write command to a bank
 *
 * @detail	Verifies that the write command is legal and issues it to the target bank.
 * @param	bGroup	Bank group
 * @param	bank	Target bank
 * @param	row	Target row
 * @param	col	Target column
 * @return	0 if the command is issued. A positive integer if some definite time remains
 *		until it can be issued. -1 otherwise.
 */
int dimm_write(int bGroup, int bank, int row, int col);

#endif
