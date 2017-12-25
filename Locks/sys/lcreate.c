#include <stdio.h>
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>

extern int lockToken;
extern struct lockTable;
int lcreate (void){
	STATWORD ps;
	disable(ps);
	int nextLock = lockToken%NLOCKS;
	struct lockTableEntry *lptr;
	int i;
	for(i=0;i<NLOCKS;i++){
		if(lockTable[nextLock].lockState==LMISSING){
			lockTable[nextLock].lockState = LAVAILABLE;
			lockTable[nextLock].lockToken = lockToken;
			proctab[currpid].lockTokenList[nextLock]=lockToken;
			lptr=&lockTable[nextLock];
			int j;
			for (j = 0; j < NPROC; ++j)
			{
				lptr->lockUserType[j]=NOTREL;
				lptr->lockPriority[j]=-9999;
			}
			restore(ps);
			return nextLock;
		}
		nextLock = (nextLock+1)%NLOCKS;
	}
	restore(ps);
	return SYSERR;
}