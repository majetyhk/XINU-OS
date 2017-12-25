/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct	mblock	*block;
	unsigned size;
{
	//kprintf("To be implemented!\n");
	STATWORD ps;
	struct mblock *memListPtr, *nextPtr, *prevPtr;
	if (block < BACKING_STORE_VIRTUAL_BASE_PAGE * NBPG || size == 0) 
		return SYSERR;
	disable(ps);
	
	memListPtr = proctab[currpid].vmemlist;
	prevPtr = memListPtr;
	nextPtr = prevPtr->mnext;
	while (nextPtr < block && nextPtr != NULL) {
		prevPtr= nextPtr;
		nextPtr = nextPtr->mnext;
	}
	block->mnext = nextPtr;
	prevPtr->mnext = block;
	block->mlen = size;

	restore(ps);
	return(OK);
}
