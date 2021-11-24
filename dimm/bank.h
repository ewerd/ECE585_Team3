

#ifndef BANK_H_
#define BANK_H_

#define TRCD	24
#define TRAS	52
#define TCAS	24
#define TRTP	12
#define TRP	24
#define TBURST	4
#define CWL	20

typedef enum {PRECHARGED,ACTIVE} bankState_t;

typedef struct {
	unsigned int		row; //Track open row 
	unsigned int		maxRows;
	bankState_t		state; //Current or target bank state
	unsigned long long	nextPrecharge; //Time available for next precharge cmd (tRP)
	unsigned long long	nextActivate; //Time available for next activate cmd (tRCD)
	unsigned long long	nextWrite; //Time available for next write cmd (tWTR_L,CWL,tWR,tBURST)
	unsigned long long	nextRead; //Time available for next read cmd(CL,tRTP,tBURST)
}bank_t;

/**
 * @fn		bank_init
 * @brief	Initializes a new bank
 *
 * @param	rows	Number of rows in bank
 * @param	newBank	Target bank
 * @return	Pointer to new bank struct
 */
bank_t *bank_init(unsigned rows);

/**
 * @fn		bank_deinit
 * @brief	Cleans and frees a bank struct
 *
 * @param	bank	Target bank to clean
 */
void bank_deinit(bank_t *bank);

/**
 * @fn		bank_canActivate
 * @brief	Checks if the specified bank can start a row activation
 *
 * @details	Verifies that the bank is already precharged or is precharging and returns
 *		the time till completion.
 * @param	bank	Pointer to the bank struct
 * @param	currentTime	The current simulation time (in CPU clock cycles)
 * @return	0 if the bank is precharged and ready for activation. A positive integer
 *		that is the time remaining if the bank is in the process of precharging.
 *		-2 if the bank is NULL. -1 otherwise.
 */
int bank_canActivate(bank_t *bank, unsigned long long currentTime);

/**
 * @fn		bank_activate
 * @brief	Issues an activate command to the bank
 *
 * @param	bank	Pointer to a bank
 * @param	row	Row to be activated
 * @param	currentTime	Current simulation time
 * @return	If successful, time until the ACT cmd is completed. -2 if bad arguments
 *		are passed. -1 otherwise
 */
int bank_activate(bank_t *bank, unsigned row, unsigned long long currentTime);

/**
 * @fn		bank_canPrecharge
 * @brief	Checks if the specified bank can start a precharge
 *
 * @details	Verifies that the bank is already activated and returns the time till 
 *		a precharge command can be issued.
 * @param	bank	Pointer to the bank struct
 * @param	currentTime	The current simulation time (in CPU clock cycles)
 * @return	0 if the bank is activated and ready for precharge. A positive integer
 *		that is the time remaining if the bank is in the process of activating,
 *		reading or writing.-2 if the bank is NULL. -1 otherwise.
 */
int bank_canPrecharge(bank_t *bank, unsigned long long currentTime);

/**
 * @fn		bank_precharge
 * @brief	Issues a precharge command to the bank
 *
 * @param	bank	Pointer to a bank
 * @param	currentTime	Current simulation time
 * @return	If successful, time until the PRE cmd is completed. -2 if bad arguments
 *		are passed. -1 otherwise
 */
int bank_precharge(bank_t *bank, unsigned long long currentTime);

/**
 * @fn		bank_canRead
 * @brief	Checks if the bank can start a read
 *
 * @details	Verifies that the specified row is already activated and returns time
 *		until the next read can occur.
 * @param	bank	Pointer to the bank struct
 * @param	row	Row that will be read from
 * @param	currentTime	The current simulation time (in CPU clock cycles)
 * @return	0 if the row is activated and ready to be read from. A positive integer
 *		that is the time remaining if the bank is in the process of precharging.
 *		-2 if the bank is NULL. -1 otherwise.
 */
int bank_canRead(bank_t *bank, unsigned row, unsigned long long currentTime);

/**
 * @fn		bank_read
 * @brief	Issues a read command to the bank
 *
 * @param	bank	Pointer to a bank
 * @param	row	Row to be read from
 * @param	currentTime	Current simulation time
 * @return	If successful, time until the read cmd is completed. -2 if bad arguments
 *		are passed. -1 otherwise
 */
int bank_read(bank_t *bank, unsigned row, unsigned long long currentTime);

/**
 * @fn		bank_canWrite
 * @brief	Checks if the bank can start a Write
 *
 * @details	Verifies that the specified row is already activated and returns time
 *		until the next write can occur.
 * @param	bank	Pointer to the bank struct
 * @param	row	Row that will be written to
 * @param	currentTime	The current simulation time (in CPU clock cycles)
 * @return	0 if the row is activated and ready to be written to. A positive integer
 *		that is the time remaining if the bank is in the process of precharging.
 *		-2 if the bank is NULL. -1 otherwise.
 */
int bank_canWrite(bank_t *bank, unsigned row, unsigned long long currentTime);

#endif
