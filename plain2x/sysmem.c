/*
	enable plain prx for PSP

	disable clear rebot area from init.prx
*/
#include "common.h"

extern void SysMemForKernel_2f808748(void *dst,int n,int size);

static u32 *pp;
static u32 od[2];
static u32 memcpy_cnt;

/*****************************************************************************
	memset hook
*****************************************************************************/
void hook_SysMemForKernel_2f808748(u8 *dst,int n,int size)
{
//	Kprintf("SysMemForKernel_2f808748(%08X,%02X,%08X)\n",dst,n,size);

	// disable memclr

	switch((u32)dst)
	{
	case 0x89000000:
		return;
	case 0x88c00000: 
		return;
	}

	if(--memcpy_cnt==0)
	{
		// save org data
		pp[0] = od[0];
		pp[1] = od[1];
		clear_cache();
	}
	// memclr func

	while(size--)
	{
		*dst++ = n;
	}
}

/*****************************************************************************
   sysmem‚Ìƒpƒbƒ`
*****************************************************************************/
void patchSysMem(void)
{
	// maximum wathc count
	memcpy_cnt = 32;

	// stub entry
	pp = (u32 *)SysMemForKernel_2f808748; // stub entry
	// true entry
	pp = (u32 *)(0x80000000 + ((pp[0]&0x03ffffff)<<2) );

	// save org data
	od[0] = pp[0];
	od[1] = pp[1];

	// patch data
	pp[0] = MIPS_J(hook_SysMemForKernel_2f808748);
	pp[1] = MIPS_LUI(2,0);
	clear_cache();
}
