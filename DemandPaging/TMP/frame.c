/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

extern int page_replace_policy;
extern int debug_option;
/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
int getFrameUsingSC(void);
int getFrameUsingAging(void);

SYSCALL init_frm()
{
	STATWORD ps;
	disable(ps);
	int i = 0;
	for(i = 0; i < NFRAMES; i++){
		frm_tab[i].fr_status = FRM_UNMAPPED;
		frm_tab[i].fr_type = FR_PAGE;
		frm_tab[i].fr_pid = -1;
		frm_tab[i].fr_refcnt = 0;
		frm_tab[i].fr_vpno = 0;
		frm_tab[i].fr_dirty = 0;
	}
	//kprintf("To be implemented!\n");
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
	//kprintf("To be implemented!\n");
	STATWORD ps;
	disable(ps);
	int i = 0;
	*avail = -1;
	int frame_number;
	for(i = 0; i < NFRAMES; i++)
		{
		if(frm_tab[i].fr_status == FRM_UNMAPPED)
			{
			*avail = i;
		restore(ps);
			return OK;
			}
		}

	if(page_replace_policy == AGING){
		frame_number = getFrameUsingAging();
		if(frame_number==-1){
			return SYSERR;
		}
		if(debug_option==1){
			kprintf("Replacing Frame : %d",frame_number);
		}
		free_frm(frame_number);
		*avail = frame_number;
		restore(ps);
		return OK;
	}else if(page_replace_policy == SC){
		frame_number = getFrameUsingSC();
		if(frame_number==-1){
			return SYSERR;
		}
		if(debug_option==1){
			kprintf("Replacing Frame : %d",frame_number);
		}
		free_frm(frame_number);
		*avail = frame_number;
		restore(ps);
		return OK;
	}


	restore(ps);
	return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
	//kprintf("To be implemented!\n");
	STATWORD ps;
	disable(ps);
	int frameTableIterator=i;
	unsigned long virtualAddress;
	unsigned long pdbr;	
	unsigned int pageTableOffset;
	unsigned int pageDirectoryOffset;
	int pageTableFrame;
	pd_t *pageDirectoryEntry; 
	pt_t *pageTableEntry;
	int backingStoreRef, framePID, pageNumber;

	if(frm_tab[frameTableIterator].fr_type==FR_PAGE){
		framePID = frm_tab[frameTableIterator].fr_pid;
		pdbr = proctab[framePID].pdbr;
		virtualAddress = frm_tab[frameTableIterator].fr_vpno;
		pageTableOffset = virtualAddress&1023;
		pageDirectoryOffset = virtualAddress/1024;		
		pageDirectoryEntry = pdbr+(pageDirectoryOffset*sizeof(pd_t));
		pageTableEntry = (pageDirectoryEntry->pd_base*NBPG)+(pageTableOffset*sizeof(pt_t));
		backingStoreRef=proctab[frm_tab[frameTableIterator].fr_pid].store;
		pageNumber = frm_tab[frameTableIterator].fr_vpno-proctab[framePID].vhpno;
		write_bs((frameTableIterator+FRAME0)*NBPG, backingStoreRef, pageNumber);
		pageTableEntry->pt_pres = 0;
		pageTableFrame=pageDirectoryEntry->pd_base-FRAME0;
		frm_tab[pageTableFrame].fr_refcnt-=1;
		if(frm_tab[pageTableFrame].fr_refcnt==0){
			frm_tab[pageTableFrame].fr_pid = -1;
			frm_tab[pageTableFrame].fr_status = FRM_UNMAPPED;
			frm_tab[pageTableFrame].fr_type = FR_PAGE;
			frm_tab[pageTableFrame].fr_vpno = BACKING_STORE_VIRTUAL_BASE_PAGE;
			pageDirectoryEntry->pd_pres = 0;
		}
		restore(ps);
		return OK;
	}
	restore(ps);
	return SYSERR;
}



void evict_frame(int pid)
{
	int i = 0;
	for(i = 0; i < NFRAMES; i++)
		{
		if(frm_tab[i].fr_pid == pid)
			{
		  	frm_tab[i].fr_status = FRM_UNMAPPED;
		  	frm_tab[i].fr_vpno = BACKING_STORE_VIRTUAL_BASE_PAGE;
		  	frm_tab[i].fr_type = FR_PAGE;
		  	frm_tab[i].fr_pid = -1;
		  	frm_tab[i].fr_refcnt = 0;
		  	frm_tab[i].fr_dirty = 0;
			}
		}
}

void setPageDirectory(int pid){
	int frame_num = 0;
	int i;	
	pd_t *directoryEntry;

	if(get_frm(&frame_num)!=SYSERR){
		proctab[pid].pdbr = (FRAME0 + frame_num)*NBPG;
		//kprintf("Page Directory Frame: %d, pid: %d, pdbr: %d\n",frame_num,pid,proctab[pid].pdbr);
		frm_tab[frame_num].fr_pid = pid;
		frm_tab[frame_num].fr_status = FRM_MAPPED;
		frm_tab[frame_num].fr_type = FR_DIR;
		directoryEntry = (pd_t*)((FRAME0+ frame_num)*NBPG);
		for(i = 0; i < NFRAMES; i++){
			directoryEntry[i].pd_write = 1;	
			if(i<4){
				directoryEntry[i].pd_pres = 1;
				directoryEntry[i].pd_base = FRAME0+i;
				//directoryEntry[i].pd_user = 0;
			}else{
				directoryEntry[i].pd_pres = 0;
				//directoryEntry[i].pd_user = 0;
			}	
		}
	}
	else{
		kprintf("Error!!!\n");
	}
	
	
}

int getFrameUsingSC(){
	int frameTableIterator=0;
	unsigned long virtualAddress;
	unsigned long pdbr;	
	unsigned int pageTableOffset;
	unsigned int pageDirectoryOffset;
	
	pd_t *pageDirectoryEntry; 
	pt_t *pageTableEntry;
	int framePID, stopFlag=0;
	while(!stopFlag)
	{
		/* code */
		if(frm_tab[frameTableIterator].fr_type==FR_PAGE){
			virtualAddress = frm_tab[frameTableIterator].fr_vpno;
			framePID = frm_tab[frameTableIterator].fr_pid;
			pdbr = proctab[framePID].pdbr;			
			pageDirectoryOffset = virtualAddress/1024;
			pageTableOffset = virtualAddress&1023;
			pageDirectoryEntry = pdbr+(pageDirectoryOffset*sizeof(pd_t));
			pageTableEntry = (pageDirectoryEntry->pd_base*NBPG)+(pageTableOffset*sizeof(pt_t));
			if(pageTableEntry->pt_acc==0){
				stopFlag=1;
				return frameTableIterator;
			}
			else{
				pageTableEntry->pt_acc=0;
			}
		}
		frameTableIterator++;
		frameTableIterator=frameTableIterator%NFRAMES;
	}
}

int getFrameUsingAging(){
	int frameTableIterator=0;
	unsigned long virtualAddress;
	unsigned long pdbr;	
	unsigned int pageTableOffset;
	unsigned int pageDirectoryOffset;
	int minIndex=-1;
	int minAge=300;
	int tempVal;
	
	pd_t *pageDirectoryEntry; 
	pt_t *pageTableEntry;
	int framePID;
	while(frameTableIterator<NFRAMES)
	{
		/* code */
		if(frm_tab[frameTableIterator].fr_type==FR_PAGE){
			virtualAddress = frm_tab[frameTableIterator].fr_vpno;
			framePID = frm_tab[frameTableIterator].fr_pid;
			pdbr = proctab[framePID].pdbr;			
			pageDirectoryOffset = virtualAddress/1024;
			pageTableOffset = virtualAddress&1023;
			pageDirectoryEntry = pdbr+(pageDirectoryOffset*sizeof(pd_t));
			pageTableEntry = (pageDirectoryEntry->pd_base*NBPG)+(pageTableOffset*sizeof(pt_t));
			frm_tab[frameTableIterator].fr_age>>=1;
			if(pageTableEntry->pt_acc==1){
				tempVal=frm_tab[frameTableIterator].fr_age+128;
				frm_tab[frameTableIterator].fr_age=255<tempVal?255:tempVal;	
			}		
			if(frm_tab[frameTableIterator].fr_age<minAge){
				minAge=frm_tab[frameTableIterator].fr_age;
				minIndex=frameTableIterator;
			}
		}
		frameTableIterator++;
	}
	return minIndex;
}


int setPageTable()
{
	int i;
	int frame_number;
	unsigned int frame_addr;
	pt_t *pageTablePtr;
	if(get_frm(&frame_number)!=SYSERR){
		//kprintf("Page Table Frame: %d, pid: %d\n",frame_number,currpid);
		frame_addr = (FRAME0 + frame_number)*NBPG;

		 pageTablePtr = (pt_t*)frame_addr;


		for(i=0;i<NFRAMES;i++){
			pageTablePtr[i].pt_write = 1;
			pageTablePtr[i].pt_global = 0;
			pageTablePtr[i].pt_acc = 0;
			pageTablePtr[i].pt_mbz = 0;
			pageTablePtr[i].pt_pcd = 0;
			pageTablePtr[i].pt_avail = 0;
			pageTablePtr[i].pt_base = 0;
			pageTablePtr[i].pt_pres = 0;
			pageTablePtr[i].pt_pwt = 0;
			pageTablePtr[i].pt_dirty = 0;		
			pageTablePtr[i].pt_user = 0;
		}

		return frame_number;
	}
	kprintf("Error!!\n");
	return SYSERR;
	
}