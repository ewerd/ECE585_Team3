

#ifndef BANK_H_
#define BANK_H_

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
 */
void bank_init(unsigned rows, bank_t *newBank);

#endif
