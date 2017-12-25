#include <stdio.h>
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <lock.h>
#include <proc.h>

extern struct lockTable;
extern int lockToken;
extern int ctr1000;


int lock (int ldes1, int type, int priority){
	STATWORD ps;
	disable(ps);
	struct pentry *pptr;
	pptr=&proctab[currpid];
	struct lockTableEntry *lptr;
	lptr=&lockTable[ldes1];
	int isWrite,highestWritePrio;
	if(lockTable[ldes1].lockState==LMISSING){
		restore(ps);
		return DELETED;
	}
	else if(pptr->lockTokenList[ldes1]!=-1 && pptr->lockTokenList[ldes1]!=lptr->lockToken){
		return SYSERR;
	}
	else{
		if(type==WRITE){
			//kprintf("Write Lock Req\n");
			if(lptr->lockState==LAVAILABLE){
				//Give the lock and chill
				//kprintf("Lock Free. Give and Chill\n");
				lptr->lockState=LBLOCKED;
				lptr->lockType[currpid] = WRITE;
				lptr->lockUserType[currpid]=CURR;
				lptr->lockPriority[currpid] = priority;
				pptr->lockList[ldes1] = 1;
				pptr->lockTokenList[ldes1] = lptr->lockToken;
				updatePrioritiesOnLockWait(currpid,ldes1,0);
				
				//lptr->lockWaitTime = ctr1000;
				//lptr->lockWaitlist[currpid] = 0;
				//lptr->lockUsersPriority[currpid] = pptr->pprio;
				//lptr->lockUserMode = type;
				//lptr->lockWaitType[currpid] = type;
				
				restore(ps);
				return OK;
			}
			if(lptr->lockState==LBLOCKED){
				//insert into waitlist
				//Update Priority
				//kprintf("Lock Busy. Wait\n");
				lptr->lockType[currpid] = WRITE;
				lptr->lockUserType[currpid]=WAIT;
				lptr->lockPriority[currpid] = priority;
				lptr->lockWaitTime[currpid] = ctr1000;
				//resched
				
				pptr->pstate=PRWAIT;
				pptr->lockList[ldes1] = 1;
				pptr->lockTokenList[ldes1] = lptr->lockToken;
				updatePrioritiesOnLockWait(currpid,ldes1,0);

				resched();
				
				restore(ps);
				return pptr->pwaitret;
			}
		}
		else if(type==READ){
			//kprintf("Read Lock Req\n");
			if(lptr->lockState == LAVAILABLE){
				//give lock and chill
				//kprintf("Lock Free. Give and Chill\n");
				lptr->lockState=LBLOCKED;
				lptr->lockType[currpid] = READ;
				lptr->lockUserType[currpid] = CURR;
				lptr->lockPriority[currpid] = priority;
				pptr->lockList[ldes1] = 1;
				pptr->lockTokenList[ldes1] = lptr->lockToken;
				
				updatePrioritiesOnLockWait(currpid,ldes1,0);
				//updatePrioritiesOnLock(currpid,ldes1);
				restore(ps);
				return OK;
			}
			else{
				//give lock if being used as read lock and update priorities transitively
				isWrite=isWriteLock(ldes1);
				if(isWrite>0){
					//add in wait list and update priorities
					//kprintf("Lock Busy due to Writer. Wait %s, p %d, l %d\n",proctab[currpid].pname,currpid,ldes1);
					lptr->lockType[currpid] = READ;
					lptr->lockUserType[currpid] = WAIT;
					lptr->lockPriority[currpid] = priority;
					lptr->lockWaitTime[currpid] = ctr1000;
					//resched
					pptr->pstate=PRWAIT;
					pptr->lockList[ldes1] = 1;
					pptr->lockTokenList[ldes1] = lptr->lockToken;
					
					updatePrioritiesOnLockWait(currpid,ldes1,0);
					
					resched();
					
					restore(ps);
					return pptr->pwaitret;
				}
				else{
					
					highestWritePrio = getHighestWaitWritePriority(ldes1);
					if(priority>=highestWritePrio){
						//give the lock and update priorities
						//kprintf("Lock Busy. Priority Greater than waiting Writers. Acquired\n");
						lptr->lockType[currpid] = READ;
						lptr->lockUserType[currpid] = CURR;
						lptr->lockPriority[currpid] = priority;

						//pptr->pstate=PRWAIT;
						pptr->lockList[ldes1] = 1;
						pptr->lockTokenList[ldes1] = lptr->lockToken;
						
						updatePrioritiesOnLockWait(currpid,ldes1,0);
						//updatePrioritiesOnLock(currpid,ldes1);
						
						restore(ps);
						return OK;
					}
					else{
						//kprintf("Lock busy. But less than waiting writer. Wait \n");
						lptr->lockType[currpid] = READ;
						lptr->lockUserType[currpid] = WAIT;
						lptr->lockPriority[currpid] = priority;
						lptr->lockWaitTime[currpid] = ctr1000;
						//resched
						pptr->pstate=PRWAIT;
						pptr->lockList[ldes1] = 1;
						pptr->lockTokenList[ldes1] = lptr->lockToken;
						updatePrioritiesOnLockWait(currpid,ldes1,0);
						
						resched();
						
						restore(ps);
						return pptr->pwaitret;
					}
				}
			}
		}
		else{
			restore(ps);
			return SYSERR;
		}
		
	}
	restore(ps);
	return SYSERR;
}

void updatePrioritiesOnLockWait(int pid,int lockDescriptor, int beingKilled){
	
	struct pentry *pptr,*npptr;
	pptr=&proctab[pid];
	
	struct lockTableEntry *lptr;
	lptr=&lockTable[lockDescriptor];
	if(lptr->lockUserType[pid] == WAIT){
		int pIterator;
		//kprintf("Waiting Process %s %d being pinherited in %d\n",proctab[pid].pname,pid,lockDescriptor);
		for (pIterator = 0; pIterator < NPROC; pIterator++)
		{
			if(lptr->lockUserType[pIterator]==CURR && pptr->pprio>proctab[pIterator].pprio){
				if(proctab[pIterator].pinh==-1){
					proctab[pIterator].pinh=proctab[pIterator].pprio;
				}
				
				chprio(pIterator,pptr->pprio);
				int lockIterator;
				for(lockIterator=0;lockIterator<NLOCKS;lockIterator++){
					npptr=&proctab[pIterator];
					if(npptr->lockList[lockIterator]==1)
					updatePrioritiesOnLockWait(pIterator,lockIterator,beingKilled);
				}
			}
			/*else if(pptr->pprio<proctab[pIterator].pprio){
				kprintf("Being Killed or released \n");
				if(proctab[pIterator].pinh!=-1){
					int highWaitProcPrio=getHighestWaitProcPriority(lockDescriptor);
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
						updatePrioritiesOnLockWait(pIterator,lockIterator,beingKilled);
					}
				}
			}*/
		}
	}
	else{
		//kprintf("Curr Process %s %d being pinherited in %d\n",proctab[pid].pname, pid,lockDescriptor);
		int highWaitProcPrio=getHighestWaitProcPriority(lockDescriptor);

		if(pptr->pprio<highWaitProcPrio){
			if(proctab[pid].pinh==-1){
				proctab[pid].pinh=proctab[pid].pprio;
			}
			chprio(pid,highWaitProcPrio);
			int lockIterator;
			for(lockIterator=0;lockIterator<NLOCKS;lockIterator++){
				npptr=&proctab[pid];
				if(npptr->lockList[lockIterator]==1)
				updatePrioritiesOnLockWait(pid,lockIterator,0);
			}
		}
	}

}

/*void updatePrioritiesOnLockAcquire(int pid,int lockDescriptor){
	
	struct pentry *pptr,*npptr;
	pptr=&proctab[pid];
	
	struct lockTableEntry *lptr;
	lptr=&lockTable[lockDescriptor];
	if(pptr->pprio<lptr->highestProcPrio){
		if(proctab[pid].pinh==-1){
			proctab[pid].pinh=proctab[pid].pprio;
		}
		int newPrio=getHighestWaitProcPriority(lockDescriptor);
		chprio(pid,newPrio);
		for(int lockIterator=0;lockIterator<NLOCKS;lockIterator++){
			npptr=&proctab[pid];
			if(npptr->lockList[lockIterator]==1)
			updatePrioritiesOnLockWait(pid,lockIterator);
		}
	}
}*/


int isWriteLock(int lockDescriptor){
	int i ;
	for (i= 0; i < NPROC; ++i)
	{
		if(lockTable[lockDescriptor].lockUserType[i]==CURR&&lockTable[lockDescriptor].lockType[i]==WRITE){
			return lockTable[lockDescriptor].lockPriority[i];
		}
	}
	return 0;
}

int getHighestWaitWritePriority(int lockDescriptor){
	int retVal=-9999;
	int i;
	for (i = 0; i < NPROC; ++i)
	{
		if(lockTable[lockDescriptor].lockUserType[i]==WAIT && lockTable[lockDescriptor].lockType[i]==WRITE && lockTable[lockDescriptor].lockPriority[i]>retVal){
			retVal= lockTable[lockDescriptor].lockPriority[i];
		}
	}
	return retVal;
}

int getHighestWaitProcPriority(int lockDescriptor){
	int retVal=-9999;
	int i;
	for (i = 0; i < NPROC; ++i)
	{
		if(lockTable[lockDescriptor].lockUserType[i]==WAIT  && lockTable[lockDescriptor].lockPriority[i]>retVal){
			retVal= proctab[i].pprio;
		}
	}
	return retVal;
}