//lock.h
#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef	NLOCKS
#define	NLOCKS 50		/* number of locks */
#endif

#ifndef	NPROC				/* set the number of processes	*/
#define	NPROC		30		/*  allowed if not already done	*/
#endif

#ifndef	DELETED				
#define	DELETED		-6		
#endif

#define LMISSING  212
#define LAVAILABLE 210
#define LBLOCKED 211

#define READ 300
#define WRITE 301

#define CURR -1
#define WAIT 1
#define NOTREL 0

struct lockTableEntry{
	int lockState; //Missing or Available or Blocked
	int lockToken;
	int lockUserType[NPROC];// CURR -> Curr Process, WAIT -> Waiting Process, NOTREL - No Relation
	int lockType[NPROC]; //READ or WRITE
	int lockPriority[NPROC];
	unsigned long lockWaitTime[NPROC];
	//int highestProcPrio;
	//int lockUsersPriority[NPROC];
};

extern int lockToken;

extern struct lockTableEntry lockTable[];


void lockInit(void);
int lcreate (void);
int ldelete (int);
int lock (int, int, int);
int releaseall (int, ...);
int getHighestWaitWritePriority(int);
int getHighestWaitProcPriority(int);
int releaseLock(int,int,int);
int getNextHighestWaitProcPriority(int, int);
int isWriteLock(int);
void updatePrioritiesOnLockWait(int,int,int);
//void updatePrioritiesOnLockAcquire(int,int);
void updatePrioritiesOnRelease(int,int,int);
#endif