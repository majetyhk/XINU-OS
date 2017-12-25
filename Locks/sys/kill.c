/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);

	int lIter=0;
	for(lIter=0;lIter<NLOCKS;lIter++){
		if(proctab[pid].lockList[lIter]==1){
			releaseLock(pid,lIter, 1);
			//updatePrioOnKill(pid,lIter);
			/*lptr->lockUserType[pid]=NOTREL;
			lptr->lockPriority[pid] = -9999;
			proctab[pid].lockList[ldes]=0;*/
		}
	}

	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;


	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}

/*void updatePrioOnKill(int pid, int lockDescriptor){
	struct pentry *pptr,*npptr;
	pptr=&proctab[pid];
	
	struct lockTableEntry *lptr;
	lptr=&lockTable[lockDescriptor];
	if(lptr->lockUserType[pid] == WAIT){
		int pIterator;
		int highWaitProcPrio=getHighestWaitProcessPriority(pid,lockDescriptor);
		kprintf("Waiting Process %s %d being pinherited in %d\n",proctab[pid].pname,pid,lockDescriptor);
		for (pIterator = 0; pIterator < NPROC; pIterator++){
			if(proctab[pIterator].pinh!=-1){
				if(proctab[pIterator].pinh<highWaitProcPrio){
					chprio(pIterator,highWaitProcPrio);
				}else{
					chprio(pIterator,proctab[pIterator].pinh);
					proctab[pIterator].pinh = -1;
				}
				int lockIterator;
				npptr=&proctab[pIterator];
				for(lockIterator=0;lockIterator<NLOCKS;lockIterator++){
					if(npptr->lockList[lockIterator]==1)
					updatePrioritiesOnKill(pIterator,lockIterator);
				}
			}
		}
	}
}

int getHighestWaitProcessPriority(int pid, int lockDescriptor){
	int retVal=-9999;
	int i;
	for (i = 0; i < NPROC; ++i)
	{
		if(lockTable[lockDescriptor].lockUserType[i]==WAIT  && lockTable[lockDescriptor].lockPriority[i]>retVal && i!=pid){
			retVal= proctab[i].pprio;
		}
	}
	return retVal;
}*/