/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

extern int page_replace_policy;
extern int setPageTable(void);


/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
	STATWORD ps;
	unsigned long requestedVirtualAddress;
	virt_addr_t *virtualAddressStruct; 
	unsigned long pageDirectoryBaseRegister; 
	unsigned int pageOffset,pageTableOffset,pageDirectoryOffset;
	pd_t *pageDirectoryEntry;
	pt_t *pageTableEntry; 
	int newPageTableFrameIndex; 
	int newEmptyFrameIndex; 
	int backingStoreRef; 
	int backingStorePageOffset; 
	disable(ps);

	requestedVirtualAddress = read_cr2();
	//kprintf("Enter Page Fault ISR. requestedVirtualAddress: 0x%08x, pid: %d\n",requestedVirtualAddress,currpid);

	virtualAddressStruct = (virt_addr_t*)&requestedVirtualAddress;

	pageOffset = virtualAddressStruct->pg_offset;
	pageTableOffset = virtualAddressStruct->pt_offset;
	pageDirectoryOffset = virtualAddressStruct->pd_offset;

	pageDirectoryBaseRegister = proctab[currpid].pdbr;

	pageDirectoryEntry = pageDirectoryBaseRegister+pageDirectoryOffset*sizeof(pd_t);

	if(pageDirectoryEntry->pd_pres ==0)
	{
		newPageTableFrameIndex = setPageTable();

		pageDirectoryEntry->pd_pres = 1;
		pageDirectoryEntry->pd_write = 1;
		pageDirectoryEntry->pd_pcd = 0;
		pageDirectoryEntry->pd_acc = 0;
		pageDirectoryEntry->pd_global = 0;
		pageDirectoryEntry->pd_pwt = 0;
		pageDirectoryEntry->pd_mbz = 0;
		pageDirectoryEntry->pd_user = 1;
		pageDirectoryEntry->pd_fmb = 0;
		pageDirectoryEntry->pd_avail = 0;
		pageDirectoryEntry->pd_base = newPageTableFrameIndex+FRAME0;

		frm_tab[newPageTableFrameIndex].fr_status = FRM_MAPPED;
		frm_tab[newPageTableFrameIndex].fr_pid = currpid;
		frm_tab[newPageTableFrameIndex].fr_type = FR_TBL;
		//kprintf("Page Directory Entry Page Table base: %d, pid: %s\n",pageDirectoryEntry->pd_base,proctab[currpid].pname);		
	}

	pageTableEntry = (pt_t*)(pageDirectoryEntry->pd_base*NBPG+pageTableOffset*sizeof(pt_t));

	if(pageTableEntry->pt_pres == 0)
	{
		get_frm(&newEmptyFrameIndex);

		pageTableEntry->pt_pres = 1;
		pageTableEntry->pt_write = 1;
		pageTableEntry->pt_user = 1;
		pageTableEntry->pt_base = (FRAME0+newEmptyFrameIndex);

		frm_tab[newEmptyFrameIndex].fr_status = FRM_MAPPED;
		frm_tab[newEmptyFrameIndex].fr_pid = currpid;
		frm_tab[newEmptyFrameIndex].fr_vpno = requestedVirtualAddress/NBPG;
		frm_tab[newEmptyFrameIndex].fr_type = FR_PAGE;
		frm_tab[newEmptyFrameIndex].fr_dirty=0;

		frm_tab[pageDirectoryEntry->pd_base-FRAME0].fr_refcnt++;
		
		bsm_lookup(currpid,requestedVirtualAddress,&backingStoreRef,&backingStorePageOffset);
		//kprintf("BSM Looked Up BS: %d, pg_offset: %d\n", backingStoreRef, backingStorePageOffset);
		read_bs((char*)((FRAME0+newEmptyFrameIndex)*NBPG),backingStoreRef,backingStorePageOffset);
		//kprintf("Page Frame: %d, pid: %s\n",newEmptyFrameIndex,proctab[currpid].pname);
		
	}

	write_cr3(pageDirectoryBaseRegister);
	//pageTranslationDump(requestedVirtualAddress);
	//kprintf("Wrote PDBR:0x%08x, %d, %d.\nExiting Page Fault ISR.\n",pageDirectoryBaseRegister,pageDirectoryBaseRegister,pageDirectoryBaseRegister/NBPG);
	restore(ps);
	return OK;
	
}
