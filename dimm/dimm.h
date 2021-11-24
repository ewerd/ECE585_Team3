/**
 * @file	dimm.h
 * @brief	Abstraction of a DIMM
 *
 * @detail	DIMM contains 4 bank groups of 4 banks each and 256 rows per bank.
 *
 * @date	TODO
 * @authors	TODO
 */

#ifndef DIMM_H_
#define DIMM_H_

#include "group.h"
#include "bank.h"
#include "../wrappers.h"


typedef struct {
	bGroup_t*		group; //Array of groups
	unsigned 		numGroups; //Number of groups in the dimm
	unsigned long long	nextWrite; //Time available for next write(tCCD_S)
	unsigned long long	nextRead; //Time available for next read(tCCD_S,tWTR_S)
	unsigned long long	nextActivate; //Time available for next ACT command(tRRD_S)
} dimm_t;

//My thought is we just copy this format for a bank_group.h and bank.h. Then we'll recursively call
//those functions from these functions

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
dimm_t *dimm_init(int groups, int banks, int rows);


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
 * @param	bGroup	The bank group that contains the bank.
 * @param	bank	The bank number within the bank group.
 * @return	0 if the bank is precharged and ready for activation. A positive integer
 *		that is the time remaining if the bank is in the process of precharging.
 *		-1 otherwise.
 */
int dimm_canActivate(int bGroup, int bank);

/**
 * @fn		dimm_activate
 * @brief	Issues an activate command to a bank
 *
 * @details	Verifies that bank is available for an activate command and then issues it.
 * @param	bGroup	Bank group
 * @param	bank	The bank being activated
 * @param	row	The row to activate
 * @return	0 if the command was issued successfully. Some positive integer if the
 *		command can't be issued yet (see dimm_canActivate()). -1 otherwise.
 */
int dimm_activate(int bGroup, int bank, int row);

/**
 * @fn		dimm_canPrecharge
 * @brief	Checks if the specified bank can receive a precharge command
 *
 * @detail	Verifies that the bank is idle or calculates how much tRTP since the last 
 *		read, tWR since the last write, or tRAS time remains.
 * @param	bGroup	The bank group that contains the bank
 * @param	bank	The bank number within the group.
 * @return	0 if the precharge command can be issued. A positive integer if some definite
 *		time remains until precharge can be issued. -1 otherwise.
 */
int dimm_canPrecharge(int bGroup, int bank);

/**
 * @fn		dimm_precharge
 * @brief	Precharges a bank
 *
 * @detail	Verifies the bank is available for precharging and issues the command.
 * @param	bGroup	Bank group
 * @param	bank	Target bank
 * @return	0 if the command was issued. Some positive integer if the command can't be
 *		issued yet (see dimm_canPrecharge()). -1 otherwise
 */
int dimm_precharge(int bGroup, int bank);

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
