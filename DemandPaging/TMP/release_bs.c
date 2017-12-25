#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

  /* release the backing store with ID bs_id */
	//kprintf("To be implemented!\n");
	bsm_tab[bs_id].bs_status = BSM_UNMAPPED;
	bsm_tab[bs_id].bs_pid = -1;
	bsm_tab[bs_id].bs_npages = 0;
	bsm_tab[bs_id].bs_vpno = BACKING_STORE_VIRTUAL_BASE_PAGE;
	bsm_tab[bs_id].bs_sem = 0;
	bsm_tab[bs_id].bs_ispriv = 0;
	
	return OK;

}
