/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sched.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	register struct	pentry	*optr;	// pointer to old process entry 
	register struct	pentry	*nptr;	// pointer to new process entry 
	int schedulerType = getschedclass();

	if(schedulerType==DEFAULTSCHEDULER){
		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
		   (lastkey(rdytail)<optr->pprio)) {
			return(OK);
		}
	}

	optr= &proctab[currpid];
	/* force context switch */
	if(schedulerType!=LINUXSCHED){
		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
			//kprintf("Inserting into queue : %s\n", optr->pname);
		}
	}
	
	int nextpid = getNextProcess(getschedclass(),currpid);
	/* remove highest priority process at end of ready list */
	//kprintf("Old PID = %d, priority = %d :-:",currpid, proctab[currpid].pprio);
	if(schedulerType==EXPDISTSCHED){
		if(currpid == nextpid){
			dequeue(currpid);
			#ifdef	RTCLOCK
				preempt = QUANTUM;		 //reset preemption counter	
			#endif
			optr->pstate = PRCURR;
			return OK;
		}
	}
	
	nptr = &proctab[ (currpid = nextpid) ];
	/*if(nextpid!=0){
		//kprintf("New PID = %d, priority = %d \n",currpid, proctab[currpid].pprio );
	}*/
	if(schedulerType!=LINUXSCHED){
		nptr->pstate = PRCURR;		/* mark it currently running	*/
		#ifdef	RTCLOCK
			preempt = QUANTUM;		 //reset preemption counter	
		#endif
	}
	else{
		nptr->pstate = PRCURR;
		#ifdef	RTCLOCK
			preempt = nptr->pquantum;		 //reset preemption counter	
		#endif
	}
	
	//printf("reset preemption outside\n");
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}
	
