/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{
	STATWORD ps;
	disable(ps);
	//kprintf("Entering vgetmem\n");
	struct mblock *vMemListPtr,*nextPtr,*currPtr,*remPtr;
	vMemListPtr=proctab[currpid].vmemlist;
	if(nbytes==0||vMemListPtr->mnext==(struct mblock *)NULL){
		//kprintf("Exiting vgetmem1\n");
		restore(ps);
		//kprintf("Exiting vgetmem1\n");
		return( (WORD *)SYSERR);
	}
	nbytes=roundmb(nbytes);
	nextPtr=vMemListPtr->mnext;
	currPtr=vMemListPtr;
	//kprintf("Entering vgetmem while loop\n");
	while(nextPtr!=NULL){
		//kprintf("Entered\n");
		if(nextPtr->mlen==nbytes){
			currPtr->mnext=nextPtr->mnext;
			//kprintf("Exiting vgetmem2\n");
			restore(ps);
			//kprintf("Exiting vgetmem2\n");
			return ((WORD *)nextPtr);
		}
		else if(nextPtr->mlen>nbytes){
			remPtr=(struct mblock *)((unsigned)nextPtr+nbytes);
			remPtr->mlen=nextPtr->mlen-nbytes;
			remPtr->mnext=nextPtr->mnext;
			currPtr->mnext=remPtr;
			//kprintf("Exiting vgetmem3\n");
			restore(ps);
			//kprintf("Exiting vgetmem3\n");
			return ((WORD *)nextPtr);
		}
		currPtr=nextPtr;
		nextPtr=nextPtr->mnext;

	}
	//kprintf("Exiting vgetmem4\n");
	restore(ps);

	
	return( (WORD *)SYSERR);
}


