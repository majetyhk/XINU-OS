#include <stdio.h>
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>

int ldelete (int lockdescriptor){
	STATWORD ps;
	disable(ps);

	lockTable[lockdescriptor].lockState = LMISSING;
	lockTable[lockdescriptor].lockToken = -1;
	int pIter;
	for (pIter = 0; pIter < NPROC; ++pIter)
	{
		if(lockTable[lockdescriptor].lockUserType[pIter]!=NOTREL){
			lockTable[lockdescriptor].lockUserType[pIter]=NOTREL;
			lockTable[lockdescriptor].lockPriority[pIter]=-9999;
			proctab[pIter].lockList[lockdescriptor]=0;
			proctab[pIter].pwaitret = DELETED;
			updatePrioritiesOnRelease(pIter,lockdescriptor,0);
		}
		if(proctab[pIter].pstate == PRWAIT){
			ready(pIter,RESCHNO);
		}
	}
	resched();

	restore(ps);
	return OK;
}