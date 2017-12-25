/* ready.c - ready */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sched.h>

/*------------------------------------------------------------------------
 * ready  --  make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
int ready(int pid, int resch)
{
	register struct	pentry	*pptr;

	if (isbadpid(pid))
		return(SYSERR);
	pptr = &proctab[pid];
	pptr->pstate = PRREADY;
	
	if(schedClass == LINUXSCHED){
		insert(pid,rdyhead,pptr->pgoodness);
		//kprintf("Pushed Goodness\n");
	}
	else{
		insert(pid,rdyhead,pptr->pprio);
		//kprintf("Pushed Priority\n");
	}
	if (resch)
		resched();
	return(OK);
}
