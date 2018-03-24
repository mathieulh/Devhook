/*
	PSP patch
*/
#include "common.h"

#define PATCH_LOG 0

/////////////////////////////////////////////////////////////////////////////
// cache clear after patch
/////////////////////////////////////////////////////////////////////////////
void clear_cache(void)
{
  sceKernelDcacheWritebackAll();
  sceKernelIcacheClearAll();
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static u32 *get_j_addr(u32 inst)
{
	return (u32 *)( ((inst&0x03ffffff)<<2)|0x80000000 );
}

static int is_branch(u32 inst)
{
	// not supported
	return 0;
}

static int is_j(u32 inst)
{
	return (inst & 0xfc000000) == MIPS_J(0) ? 1 : 0;
}

static int is_jal(u32 inst)
{
	return (inst & 0xfc000000) == MIPS_JAL(0) ? 1 : 0;
}

static int is_sycall(u32 inst)
{
	return (inst & 0xfc00003f) == MIPS_SYSCALL(0);
}

/////////////////////////////////////////////////////////////////////////////
// get syscall vector
/////////////////////////////////////////////////////////////////////////////
typedef struct ModuleSyscallHeader
{
 void *unk;
 u32 basenum;
 u32 topnum;
 u32 size;
} ModuleSyscallHeader;

typedef struct ModuleFunc
{
 u32 addr;
 u32 *sysaddr;
} ModuleFunc;

/////////////////////////////////////////////////////////////////////////////
// get syscall vector base
/////////////////////////////////////////////////////////////////////////////
static unsigned int *sysvector = 0;

static int SetSystemcallVector(void)
{
	if(sysvector) return 1; // OK

#if 1
	u8 **syscall;
	ModuleSyscallHeader *sysheader;
	u32 *systable;
//	int size;

 // Get syscall table
 asm("cfc0 %0, $12\n" : "=r"(syscall));

 // Exit if failed
 if (!(syscall)) return 0;

 // Get syscall header
 sysheader = (ModuleSyscallHeader *) *syscall;

 // Get syscall table
 systable = (u32 *) ((*syscall) + sizeof(ModuleSyscallHeader));

	 // Get syscall size
	 // size = (sysheader->size - sizeof(ModuleSyscallHeader)) / sizeof(u32);

	// SysVector Base
	sysvector = (unsigned int *)( ((u32)systable) - sysheader->basenum );
	return 1;

//Kprintf("syscall    = %08X\n",syscall);
//Kprintf("sysheader  = %08X\n",sysheader);
//Kprintf("sysheader[]= %08X %08X %08X %08X\n",sysheader->unk,sysheader->basenum,sysheader->topnum,sysheader->size);
//Kprintf("systable   = %08X\n",systable);
//Kprintf("sysvector  = %08X\n",sysvector);

#endif
}

/////////////////////////////////////////////////////////////////////////////
// get module segment from API entry & offset
/////////////////////////////////////////////////////////////////////////////
u32 *getModuleSeg(void *entry,int offset)
{
	u32 op1,op0;
	unsigned char *segbase = 0;

//Kprintf("getModuleSeg(%08X,%X)\n",(int)entry,offset);

	op0 = ((u32 *)entry)[0];
	op1 = ((u32 *)entry)[1];

	if(op1==MIPS_NOP)
	{
		if(is_j(op0))
		{
			// kernel API
			segbase = (unsigned char *)(  ((op0&0x03ffffff)<<2) | 0x80000000 );
//Kprintf("Kernel API at %08X\n",segbase);
		}
	} else if(is_sycall(op0))
	{
//Kprintf("User API at %08X\n",segbase);
		if(SetSystemcallVector())
			segbase = (unsigned char *)sysvector[(op1>>6)];
	}
	if(segbase) return (u32 *)(segbase-offset);

#if PATCH_LOG
	Kprintf("getModuleSeg: API entry not found (%08X,%X)\n",(int)entry,offset);
#endif
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
u32 *getAPIEntry(void *api_stub_entry)
{
	return getModuleSeg(api_stub_entry,0);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void *hookAPIDirect(void *api_stub_entry,void *hook_entry,HOOKAPI_resotre *restore)
{
	u32 inst;
	u32 *lp = getAPIEntry(api_stub_entry);
	if(lp==0) return (void *)lp;

	if(restore)
	{
		restore->patch_point   = lp;

		inst = lp[0];
#if 0
		// 1st instruction
		if(is_branch(inst))
		{
#if 1
			return NULL;
#else
			// jmp branch address
			restore->patch_data[5] = MIPS_J(&lp[(inst&0xffff)+1]);
			restore->patch_data[6] = MIPS_NOP;
			// branch to patch_data[5];
			inst = (inst&0xffff0000)|(5-1);
#endif
		}
#endif
		restore->patch_data[0] = inst;

		// 2nd instruction
		inst = lp[1];
#if 0
		if(is_branch(inst))
		{
#if 1
			return NULL;
#else
			// jmp branch address
			restore->patch_data[5] = MIPS_J(&lp[(inst&0xffff)+2]);
			restore->patch_data[6] = MIPS_NOP;
			// branch to patch_data[5];
			inst = (inst&0xffff0000)|(5-2);
#endif
		}
#endif
		restore->patch_data[1] = inst;
#if 0
		if(is_j(inst) || is_jal(inst))
		{
			restore->patch_data[2] = lp[2]; // delay code
			restore->patch_data[3] = MIPS_J(&lp[3]);
			restore->patch_data[4] = MIPS_NOP;
		}
		else
#endif
		{
			restore->patch_data[2] = MIPS_J(&lp[2]);
			restore->patch_data[3] = MIPS_NOP;
		}
	}
	
	// hook to my function
	lp[0] = MIPS_J(hook_entry);
	lp[1] = MIPS_NOP;

	clear_cache();

	return (void *)lp;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void restoreAPIDirect(HOOKAPI_resotre *restore)
{
	u32 *lp;
	u32 inst;

	// restore point
	lp = restore->patch_point;

	// 1st op
	inst = restore->patch_data[0];
#if 0
	if(is_branch(inst))
	{
		u32 offset = get_j_addr(inst) - (&lp[1]);
		inst = (inst&0xffff0000)|offset;
	}
#endif
	lp[0] = inst;

	// 2nd op
	inst = restore->patch_data[1];
#if 0
	if(is_branch(restore->patch_data[1]))
	{
		u32 offset = get_j_addr(inst) - (&lp[2]);
		inst = (inst&0xffff0000)|offset;
	}
#endif
	lp[1] = inst;
	clear_cache();
}
