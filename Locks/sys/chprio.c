/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	//kprintf("Changing %s's priority to %d",pptr->pname,newprio);
	pptr->pprio = newprio;
	if(pptr->pstate == PRREADY){
		dequeue(currpid);
		insert(currpid,rdyhead,pptr->pprio);
	}
	restore(ps);
	return(newprio);
}
