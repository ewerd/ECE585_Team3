/**
 * @file	dimm.h
 * @brief	Abstraction of a DIMM
 *
 * @detail	DIMM contains 4 bank groups of 4 banks each and 256 rows per bank.
 *
 * @date	TODO
 * @authors	TODO
 */

//Track last write command time for tWTR_S
//Last access time for tCCD_S
//Track last ACT command for tRRD_S

typedef struct {
	//Track open row (-1 for no open row?)
	//Track if precharged
	//Last precharge time for tRP
	//Last ACT time for tRCD
	//Last write time for tWTR_L,CWL,tWR,tBURST
	//Last read command for CL,tRTP,tBURST
	//Current time so we know when commands finish for output printing
}bank_t;

typedef struct {
	//Array of banks
	//Last ACT command for tRRD_L
	//Last access time for tCCD_L
	//Last write time for tWTR_L
}bGroup_t;

//My thought is we just copy this format for a bank_group.h and bank.h. Then we'll recursively call
//those functions from these functions

/**
 * @fn		advTime
 * @brief	Method simulator uses to advance time.
 *
 * @details	For the DIMM and its banks, advancing time changes which possible commands are available,
 *		how much time remains until prior commands complete, and, upon completion, some commands
 *		will generate terminal output that is displayed after time is advanced.
 * @param	t	Amount of time advanced in this time step
 * @return	0 on success. -1 otherwise (I/O problem with output)
 */
int advTime(int t);

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
