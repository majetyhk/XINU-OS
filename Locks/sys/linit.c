#include <stdio.h>
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <lock.h>
extern struct lockTable;
extern int lockToken;

void lockInit(void){
	lockToken = 0;
	int iterator;
	for ( iterator = 0; iterator < NLOCKS; ++iterator)
	{
		lockTable[iterator].lockState = LMISSING;
		lockTable[iterator].lockToken = -1;
		//lockTable[iterator].highestProcPrio =0;
	}
}
