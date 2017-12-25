#include <stdio.h>
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>

int releaseall (int numlocks, ...){
	STATWORD ps;
	disable(ps);
	unsigned long *a;
	int lockCount=numlocks;
    a = (unsigned long *)(&numlocks) + (numlocks); 
    int releaseArray[NLOCKS];
    int count=0;
    for ( ; numlocks > 0 ; numlocks--){
         releaseArray[count]=*a--;
         count++;
    }
    int error = 0;
    int i;
    for(i=0;i<lockCount;i++){
    	if(releaseLock(currpid, releaseArray[i],0)==-1){
    		error = 1;
    	}
	}
	if(error ==1){
		restore(ps);
    	return SYSERR;
	}
	restore(ps);
	return OK;

}

int releaseLock(int pid, int ldes,int beingKilled){
	struct lockTableEntry *lptr;
	//int highestWritePrio;
	lptr=&lockTable[ldes];
	if(proctab[pid].lockList[ldes]==0){
		return -1;
	}
	if(lptr->lockUserType[pid]==WAIT){
		
		//kprintf("Waiting process releasing the lock\n");
		lptr->lockUserType[pid]=NOTREL;
		lptr->lockPriority[pid] = -9999;
		proctab[pid].lockList[ldes]=0;
		updatePrioritiesOnRelease(pid,ldes,beingKilled);

		//kprintf("Waiting process releasing the lock\n");
		return 1;
	}
	if(lockTable[ldes].lockType[pid]==READ){
		//see any other readers still running.
		//kprintf("Its reader releasing the lock\n");
		int pIter;
		for (pIter = 0; pIter < NPROC; ++pIter)
		{
			if(lptr->lockType[pIter]==READ && lptr->lockUserType[pIter]==CURR && pIter!=pid){
				lptr->lockUserType[pid]=NOTREL;
				lptr->lockPriority[pid] = -9999;
				proctab[pid].lockList[ldes]=0;
				updatePrioritiesOnRelease(pid,ldes,beingKilled);
				//kprintf("Left the responsibility to other readers\n");
				return 1;//return as the last reader to release the lock will handle waking writes
			}
			/* code */
		}
	}
	//No other curr reader found. Search for a writer;
	//kprintf("Searching for waiting writers\n");
	int writerFound=0;
	int writerPID = -1;
	int writerPrio=-9999;
	int highestPrio = -9999;
	int highPrioPID = -1;
	int pIter;
	for (pIter = 0; pIter < NPROC; ++pIter)
	{
		if(lptr->lockType[pIter]==WRITE && lptr->lockUserType[pIter]==WAIT){
			if(writerPrio < lptr->lockPriority[pIter]){
				writerFound = 1;
				writerPID = pIter;
				writerPrio = lptr->lockPriority[pIter];
			}
			else if(writerPrio == lptr->lockPriority[pIter]&&writerPrio!=-9999){
				if(lptr->lockWaitTime[writerPID]>lptr->lockWaitTime[pIter]){
					writerPID=pIter;
				}
			}
		}
		if(lptr->lockPriority[pIter]>highestPrio && lptr->lockUserType[pIter]==WAIT){
			highestPrio=lptr->lockPriority[pIter];
			highPrioPID = pIter;
		}
		/* code */
	}
	if(writerFound ==0){
		//No Writer Found.
		//kprintf("No Writer Found\n");
		if(highPrioPID == -1){
			//No Waiting Readers. Release and Chill
			lptr->lockState=LAVAILABLE;
			lptr->lockUserType[pid]=NOTREL;
			lptr->lockPriority[pid] = -9999;
			proctab[pid].lockList[ldes]=0;
			updatePrioritiesOnRelease(pid,ldes,beingKilled);
			//kprintf("Release and Chill\n");
		}
		else{
			//Release and Wake all readers
			lptr->lockUserType[pid]=NOTREL;
			lptr->lockPriority[pid] = -9999;
			proctab[pid].lockList[ldes]=0;
			updatePrioritiesOnRelease(pid,ldes,beingKilled);
			int pIter;
			for (pIter = 0; pIter < NPROC; ++pIter){
				if(lptr->lockType[pIter]==READ && lptr->lockUserType[pIter]==WAIT){
					proctab[pIter].pstate = PRREADY;
					lptr->lockState=LBLOCKED;
					lptr->lockUserType[pIter]=CURR;
					lptr->lockWaitTime[pIter] = 0;
					if(!beingKilled){

					}
					ready(pIter, RESCHNO);
				}
			}
			//kprintf("Woke up all readers\n");
			/*proctab[highPrioPID].pstate = PRREADY;
			lptr->lockState=LBLOCKED;
			lptr->lockUserType[highPrioPID]=CURR;
			lptr->lockWaitTime[highPrioPID] = 0;
			//updatePrioritiesOnLockAcquire(highPrioPID,ldes);
			ready(highPrioPID,RESCHNO);*/
		}
	}
	else{
		//kprintf("Writer Found\n");
		if(highPrioPID==writerPID){
			//kprintf("Writer has High Priority\n");
			lptr->lockUserType[pid]=NOTREL;
			lptr->lockPriority[pid] = -9999;
			proctab[pid].lockList[ldes]=0;
			updatePrioritiesOnRelease(pid,ldes,beingKilled);
			lptr->lockState=LBLOCKED;
			lptr->lockUserType[writerPID]=CURR;
			lptr->lockWaitTime[writerPID] = 0;
			updatePrioritiesOnLockWait(writerPID,ldes,beingKilled);
			proctab[writerPID].pstate = PRREADY;
			ready(writerPID, RESCHNO);
			//kprintf("Woke up the Writer\n");
		}
		else{
			//kprintf("Readers have higher priority than writer\n");
			lptr->lockUserType[pid]=NOTREL;
			lptr->lockPriority[pid] = -9999;
			proctab[pid].lockList[ldes]=0;
			int highestWaitProcPrio = proctab[writerPID].pprio;
			int highestWaitProcPID = writerPID;
			updatePrioritiesOnRelease(pid,ldes,beingKilled);
			int pIter;
			for (pIter = 0; pIter < NPROC; ++pIter){
				if(lptr->lockType[pIter]==READ && lptr->lockUserType[pIter]==WAIT && lptr->lockPriority[pIter]>=writerPrio){
					proctab[pIter].pstate = PRREADY;
					lptr->lockState=LBLOCKED;
					lptr->lockUserType[pIter]=CURR;
					lptr->lockWaitTime[pIter] = 0;
					ready(pIter, RESCHNO);
				}
				else if(lptr->lockType[pIter]==READ && lptr->lockUserType[pIter]==WAIT && lptr->lockPriority[pIter]<writerPrio){
					if(lptr->lockPriority[pIter]>highestWaitProcPrio){
						highestWaitProcPrio = lptr->lockPriority[pIter];
						highestWaitProcPID = pIter;
					}
				}
			}
			updatePrioritiesOnLockWait(highestWaitProcPID,ldes,beingKilled); 
			kprintf("Woke up all the required Readers\n");				
		}
	}

	return 1;
	
	/*else{
		//Write Release
		int writerFound=0;
		int writerPID = -1;
		int writerPrio=-9999;
		
		for (int pIter = 0; pIter < NPROC; ++pIter)
		{
			if(lptr->lockType[pIter]==WRITE && lptr->lockUserType[pIter]==WAIT){
				if(writerPrio > lptr->lockPriority[pIter]){
					writerFound = 1;
					writerPID = pIter;
					writerPrio = lptr->lockPriority[pIter];
				}
				else if(writerPrio == lptr->lockPriority[pIter]&&writerPrio!=-9999){
					if(lptr->lockWaitTime[writerPID]>lptr->lockWaitTime[pIter]){
						writerPID=pIter;
					}
				}
			}
		}
		if(writerFound == 0){

		}
		else{
			lptr->lockUserType[pid]=NOTREL;
			proctab[pid].lockList[ldes]=0;
			updatePrioritiesOnRelease(pid,ldes);
			lptr->lockState=LBLOCKED;
			lptr->lockUserType[writerPID]=CURR;
			lptr->lockWaitTime = 0;
			updatePrioritiesOnLock(writerPID,ldes);

			proctab[writerPID].pstate = PRREADY;

			ready(writerPID, RESCHNO);
		}
	}*/
}

void updatePrioritiesOnRelease(int pid, int lockDescriptor, int beingKilled){
	
	struct pentry *pptr,*npptr;
	pptr=&proctab[pid];
	
	struct lockTableEntry *lptr;
	lptr=&lockTable[lockDescriptor];
	if(beingKilled==1){
		int pIterator;
		int highWaitProcPrio=getNextHighestWaitProcPriority(pid,lockDescriptor);
		//kprintf("Waiting Process %s %d being pinherited in %d\n",proctab[pid].pname,pid,lockDescriptor);
		if(pptr->pprio > highWaitProcPrio){
			for (pIterator = 0; pIterator < NPROC; pIterator++){
				if(lptr->lockUserType[pIterator]!=NOTREL && proctab[pIterator].pinh!=-1){
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
						updatePrioritiesOnRelease(pIterator,lockIterator,1);
					}
				}
			}
		}
		
	}else{
		if(pptr->pinh!=pptr->pprio){
			chprio(pid,pptr->pinh);
			pptr->pinh=-1;
		}
		/*struct lockTableEntry *lptr;
		lptr=&lockTable[lockDescriptor];
		if(lptr->lockUserType[pid] == WAIT){

		}
		else if(lptr->lockUserType[pid] == CURR){

		}*/
		//updatePrioritiesOnLockWait(pid,lockDescriptor,beingKilled);
		
		//int lock =-1;
		int lIterator;
		for (lIterator = 0; lIterator < NLOCKS; ++lIterator){
			if(pptr->lockList[lIterator]==1)
			{
				/*lock=lIterator;
				kprintf("Found %d",lIterator);
				break;*/
				updatePrioritiesOnLockWait(pid,lIterator,beingKilled);
			}
		}	
	}

	

	/*for (int pIterator = 0; pIterator < NPROC; pIterator++)
	{
		if(lptr->lockUserType[i]==CURR && pptr->pprio<proctab[i].pprio){
			//proctab[pIterator].pinh=proctab[pIterator].pprio;
			

			//set the next priority else reset pprio to pinh

			chprio(pIterator,pptr->pprio);
			for(int lockIterator=0;lockIterator<NLOCKS;lockIterator++){
				npptr=&proctab[pIterator];
				if(npptr->lockList[lockIterator]==1)
				updatePrioritiesOnRelease(pIterator,lockIterator);
			}
		}
	}*/
	/*if(pptr->pprio<proctab[pIterator].pprio && beingKilled == 1){
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
	}
*/}
int getNextHighestWaitProcPriority(int pid, int lockDescriptor){
	int retVal=-9999;
	int i;
	for (i = 0; i < NPROC; ++i)
	{
		if(lockTable[lockDescriptor].lockUserType[i]==WAIT  && lockTable[lockDescriptor].lockPriority[i]>retVal && i!=pid){
			retVal= proctab[i].pprio;
		}
	}
	return retVal;
}
