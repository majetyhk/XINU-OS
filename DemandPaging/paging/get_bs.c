#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

  /* requests a new mapping of npages with ID map_id */
    //kprintf("To be implemented!\n");
    STATWORD ps;
	disable(ps);
	if (npages < 0 || npages > 256 || bs_id < 0 || bs_id > MAXBS ) {
		restore(ps);
		return SYSERR;
	}
	else{
		if(bsm_tab[bs_id].bs_status == BSM_MAPPED){
			if (bsm_tab[bs_id].bs_ispriv == 1 || bsm_tab[bs_id].bs_sem == 1){
				restore(ps);
				return SYSERR;
			}
			else{
				bsm_tab[bs_id].bs_pid = currpid;
				restore(ps);
				return bsm_tab[bs_id].bs_npages;
			}	
		}
		else{
			bsm_tab[bs_id].bs_status = BSM_MAPPED;
			bsm_tab[bs_id].bs_pid = currpid;
			bsm_tab[bs_id].bs_npages = npages;
			//kprintf("Assigned Backing Store\n");
		}
	}
	restore(ps);
    return npages;

}


