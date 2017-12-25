/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
	STATWORD ps;
	disable(ps);
    int i = 0;
	for(i = 0; i < MAXBS; i++){
		bsm_tab[i].bs_status = BSM_UNMAPPED;
		bsm_tab[i].bs_pid = -1;
		bsm_tab[i].bs_npages = 0;
		bsm_tab[i].bs_ispriv = 0;
		bsm_tab[i].bs_sem = 0;
		bsm_tab[i].bs_vpno = 4096;	//default BS VPNO
	}
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
	STATWORD ps;
	disable(ps);
	int i = 0;
	for(i = 0; i < MAXBS; i++){
		if(bsm_tab[i].bs_status == BSM_UNMAPPED){
			*avail = i;
			restore(ps);
			return OK;		
		}
	}
	kprintf("Error: Backing Store Unavailable");
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
	STATWORD ps;
	disable(ps);
	bsm_tab[i].bs_status = BSM_UNMAPPED;
  	bsm_tab[i].bs_pid = -1;
  	bsm_tab[i].bs_npages = 0;
	restore(ps);
	return (OK);
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
	STATWORD ps;
	disable(ps);
	int i = 0;
	for (i = 0; i < MAXBS; i++){
		if(bsm_tab[i].bs_pid == pid){
			*store = i;
			*pageth = (vaddr/NBPG) - bsm_tab[i].bs_vpno;
			restore(ps);
			return OK;
		}
	}
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
	bsm_tab[source].bs_status = BSM_MAPPED;
	bsm_tab[source].bs_pid = pid;
	bsm_tab[source].bs_npages = npages;	
	bsm_tab[source].bs_ispriv = 0;
	bsm_tab[source].bs_vpno = vpno;
	bsm_tab[source].bs_sem = 1;
	proctab[currpid].vhpno=vpno;
	proctab[currpid].store=source;
	//kprintf("Mapped BS %d to %d\n",source,pid);
	return OK;
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
	int i = 0;
	int bsRef,pageOff;
	unsigned long vaddr = vpno*NBPG;
	for(i = 0; i < NFRAMES; i++){
		if(frm_tab[i].fr_pid == pid && frm_tab[i].fr_type == FR_PAGE){
			bsm_lookup(pid,vaddr,&bsRef,&pageOff);
			write_bs( (i+NFRAMES)*NBPG, bsRef, pageOff);
		}
	}
	bsm_tab[bsRef].bs_status = BSM_UNMAPPED;
	bsm_tab[bsRef].bs_pid = -1;
	bsm_tab[bsRef].bs_npages = 0;
	bsm_tab[bsRef].bs_ispriv = 0;
	bsm_tab[bsRef].bs_sem = 0;
	bsm_tab[bsRef].bs_vpno = 4096; //setting back default BS VPNO
	return OK;
}


