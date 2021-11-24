

#ifndef GROUP_H_
#define GROUP_H_

#include "bank.h"
#include "../wrappers.h"

#define TRRD_L	6
#define TCCD_L	8

typedef struct {
	bank_t**		bank; //Array of banks
	unsigned		numBanks; //Number of banks in group
	unsigned long long 	nextActivate; //Time available for next ACT command (tRRD_L)
	unsigned long long 	nextWrite; //Time available for next write(tCCD_L)
	unsigned long long 	nextRead; //Time available for next read(tWTR_L,tCCD_L)
}bGroup_t;

/**
 * @fn		group_init
 * @brief	Initializes a new bank group
 *
 * @param	banks	Number of banks in the group
 * @param	rows	Rows per bank
 * @return	Pointer to new group struct
 */
bGroup_t *group_init(unsigned banks, unsigned rows);

/**
 * @fn		group_deinit
 * @brief	Frees the memory allocated for the bank group
 *
 * @param	bankGroup	Target bankGroup to clean
 */
void group_deinit(bGroup_t *bankGroup);

/**
 * @fn		group_canActivate
 * @brief	Checks if the specified bank can start a row activation
 *
 * @details	Verifies that the bank is already precharged or is precharging and returns
 *		the time till completion.
 * @param	group	Pointer to the group struct
 * @param	bank	The bank number within the bank group.
 * @param	currentTime	The current simulation time (in CPU clock cycles)
 * @return	0 if the bank is precharged and ready for activation. A positive integer
 *		that is the time remaining if the bank is in the process of precharging.
 *		-2 if group or bank is out of bounds. -1 otherwise.
 */
int group_canActivate(bGroup_t *group, unsigned bank, unsigned long long currentTime);

/**
 * @fn		group_activate
 * @brief	Activates a row in a bank in a bank group
 *
 * @param	group	Pointer to a bank group
 * @param	bank	Bank number that will receive ACT command
 * @param	row	Row that will be activated
 * @param	currentTime	The current simulation time
 * @param	If ACT cmd issued successfully, returns time until ACT command is completed. 
 *		-2 if a bad argument is passed.-1 otherwise.
 */
int group_activate(bGroup_t *group, unsigned bank, unsigned row, unsigned long long currentTime);

/**
 * @fn		group_canPrecharge
 * @brief	Checks if the specified bank can start precharging
 *
 * @details	Verifies that the bank is already activated and then checks the time until a
 *		precharge command can be issued.
 * @param	group	Pointer to the group struct
 * @param	bank	Bank number to check
 * @param	currentTime	Current simulation time
 * @return	0 if the bank is ready to receive a precharge command. A positive integer showing
 *		how many CPU clock cycles remain until the bank is ready for a precharge command.
 *		-2 if a bad argument is passed. -1 if the bank isn't activated.
 */
int group_canPrecharge(bGroup_t *group, unsigned bank, unsigned long long currentTime);

/**
 * @fn		group_precharge
 * @brief	Precharges a bank in a bank group
 *
 * @param	group	Pointer to a bank group
 * @param	bank	Bank number that will receive PRE command
 * @param	currentTime	The current simulation time
 * @param	If PRE cmd issued successfully, returns time until PRE command is completed. 
 *		-2 if a bad argument is passed.-1 otherwise.
 */
int group_precharge(bGroup_t *group, unsigned bank, unsigned long long currentTime);


/**
 * @fn		group_canRead
 * @brief	Checks if the specified bank can perform a read
 *
 * @details	Verifies that the bank has activated the correct row and then checks the time 
 *		until a read command can be issued.
 * @param	group	Pointer to the group struct
 * @param	bank	Bank number to check
 * @param	row	Row that should be activated
 * @param	currentTime	Current simulation time
 * @return	0 if the bank is ready to receive a read command. A positive integer showing
 *		how many CPU clock cycles remain until the bank is ready for a read command.
 *		-2 if a bad argument is passed. -1 if the bank isn't activated.
 */
int group_canRead(bGroup_t *group, unsigned bank, unsigned row, unsigned long long currentTime);

/**
 * @fn		group_read
 * @brief	Reads a column in an activated row in a bank within a bank group
 *
 * @param	group	Pointer to a bank group
 * @param	bank	Bank number that will receive read command
 * @param	row	Row that will be read from
 * @param	currentTime	The current simulation time
 * @param	If read cmd issued successfully, returns time until command is completed. 
 *		-2 if a bad argument is passed.-1 otherwise.
 */
int group_read(bGroup_t *group, unsigned bank, unsigned row, unsigned long long currentTime);

/**
 * @fn		group_canWrite
 * @brief	Checks if the specified bank can perform a write
 *
 * @details	Verifies that the bank has activated the correct row and then checks the time 
 *		until a Write command can be issued.
 * @param	group	Pointer to the group struct
 * @param	bank	Bank number to check
 * @param	row	Row that should be activated
 * @param	currentTime	Current simulation time
 * @return	0 if the bank is ready to receive a write command. A positive integer showing
 *		how many CPU clock cycles remain until the bank is ready for a write command.
 *		-2 if a bad argument is passed. -1 if the bank isn't activated.
 */
int group_canWrite(bGroup_t *group, unsigned bank, unsigned row, unsigned long long currentTime);

#endif
