
#include <stdio.h>
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sched.h>
#include <q.h>
#include <math.h>


schedClass=DEFAULTSCHEDULER;

void printque(int head){
	int qIterator = q[head].qnext;
	int count=0;
	while(qIterator<NPROC){
		kprintf(" %d ",q[qIterator].qkey);
		qIterator=q[qIterator].qnext;
		count++;
	}
	if(count==0){
		kprintf("Empty Queue\n");
	}else{
		kprintf("\n");
	}
}

int getschedclass(){
	return schedClass;
}

void setschedclass(int schedulerType){
	schedClass = schedulerType;
	//printf("Set schedClass to %d\n",schedClass );
}

int getNextProcess(int schedulerType,int currpid){
	if(schedulerType == EXPDISTSCHED){
		//printf("Hey\n");
		int expDistPrio;
		expDistPrio = (int)expdev(LAMDA);
		if(expDistPrio>100||expDistPrio<0){
			printf("Error\n");
			//shutdown();
		}
		//kprintf(" rand Prio %d ", expDistPrio );
		int qIterator = q[rdytail].qprev;
		int oldqIterator = qIterator;
		//printf("qIterator :%d , expDistPrio : %d\n",qIterator,expDistPrio );
		while(expDistPrio<q[qIterator].qkey && qIterator<NPROC){
			//kprintf("rand prio : %d, qIterator: %d, prio: %d\n",expDistPrio, qIterator,q[qIterator].qkey);
			if(q[oldqIterator].qkey!=q[qIterator].qkey) oldqIterator = qIterator;
			qIterator=q[qIterator].qprev;
		}
		if(qIterator>=NPROC){
			return NULLPROC;
		}			
		//printf("Dequeue PID: %d\n",qIterator );
		return (dequeue(oldqIterator));
		
		//return getlast(rdytail);
	}
	else if(schedulerType == LINUXSCHED){	
		if(preempt>=0){
			proctab[currpid].pcounter = preempt;
		}
		else{
			proctab[currpid].pcounter = 0;
		}	

		int maxGoodness;
		//int qPointer=q[rdytail].qprev;
		
		
		int qIterator = q[rdytail].qprev;
		if(qIterator>NPROC){
			//kprintf("\nHey 1\n");
			int i=0;
			int insertCount =0;
			register struct	pentry	*cptr;
			while(i<NPROC){
				if(proctab[i].pstate!=PRFREE){
					cptr=&proctab[i];
					proctab[i].pquantum = proctab[i].pcounter/2 + proctab[i].pprio;
					proctab[i].pgoodness = proctab[i].pcounter + proctab[i].pprio;
					//kprintf("Inserting Goodness %d\n",cptr->pgoodness);
					insert(i,rdyhead,cptr->pgoodness);
					//kprintf("Finished Inserting\n");
					insertCount++;
				}
				i++;
			}
			//kprintf("Finished While\n");
			if(insertCount!=0){
				//printque(rdyhead);
				int newQIterator = q[rdytail].qprev;
				//kprintf("\nProc %s, Counter %d, Quantum %d, Goodness %d, \n", proctab[newQIterator].pname, proctab[newQIterator].pcounter, proctab[newQIterator].pquantum, proctab[newQIterator].pgoodness);
				return(dequeue(newQIterator));
			}
			else{
				//kprintf("There 1\n");
				return NULLPROC;
			}
			
		}
		else{
			maxGoodness = q[qIterator].qkey;
			if(maxGoodness==0){
				//printque(rdyhead);
				dequeue(qIterator);
				return NULLPROC;
			}
			//kprintf("Hey 2\n");
			//printque(rdyhead);
			//kprintf("\nProc %s, Counter %d, Quantum %d, Goodness %d, \n", proctab[qIterator].pname, proctab[qIterator].pcounter, proctab[qIterator].pquantum, proctab[qIterator].pgoodness);
			return(dequeue(qIterator));
		}			
	}
	else {
		return getlast(rdytail);
	}
}

