/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
  //kprintf("xmmap - to be implemented!\n");
  STATWORD ps;
  disable(ps);
  if ( (virtpage > 4096) && (npages >= 1) && ( npages <256) && ( source > 0 ) && ( source <= MAXBS) ){
    bsm_map(currpid, virtpage, source, npages);
    restore(ps);
    return OK;
  }
  else{
    kprintf("XMMAP Error! \n");
    disable(ps);
    return SYSERR;
  } 
  //return SYSERR;
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
  //kprintf("To be implemented!");
  STATWORD ps;
  disable(ps);
  if ( (virtpage > 4096) ){ 
    bsm_unmap(currpid, virtpage);
    restore(ps);
    return OK;
  }
  else{
    kprintf("XMUNMAP Error!");
    return SYSERR;
  } 
  //return SYSERR;
}

