

#ifndef STATS_H_
#define STATS_H_

#include "sListADT.h"
#include <stdint.h>
#include <stdio.h>

#ifndef OPERATION_T_
#define OPERATION_T_
typedef enum _operation_e { RD, WR, IFETCH } operation_t;
#endif

typedef struct
{
	uint16_t 	timeInQueue;
	operation_t	type;
}request_t;

sList_t *data;
unsigned long totalRequests, fchRequests, rdRequests, wrRequests;

/**
 * @fn		init_Stats
 * @brief	Initialize the stat tracker
 *
 * @return	0 if successful. -1 otherwise.
 */
uint8_t init_Stats(void);

/**
 * @fn		clean_Stats
 * @brief	Clean allocated memory from stats.c
 */
void clean_Stats(void);

/**
 * @fn		addRequest
 * @brief	Adds the completed requests to the data
 *
 * @param	request	Request to be added
 */
void addRequest(request_t *request);

/**
 * @fn		displayStats
 * @brief	Calculates final statistics and displays them
 *
 * @param	output	File to print statistics to
 */
void displayStats(FILE* output);

#endif
