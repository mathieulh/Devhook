/*
	reboot.bin fw2.71 patcher
*/

#include "common.h"

#define PUTC_ENTRY ((unsigned int)uart_dbg_putc)

/****************************************************************************
	reboot.bin ‚Ìƒpƒbƒ`
****************************************************************************/
void reboot271_patch(void)
{
	unsigned int *lp;
	unsigned int entry_addr;

	// sunroutine entry
	Kprintf    = (void *)0x88C01B9C;
	cache_clr2 = (void *)0x88C03954;
	cache_clr1 = (void *)0x88C03F88;

	// patch point : enable plain TXT
		lp = (unsigned int *)0x88C050B4;
//		lp[0] = MIPS_SW(0,29,0x0000);
		lp[1] = MIPS_SW(5,29,0x0000);
		lp[2] = MIPS_ADDI(2,5,0);

/*
  putc handler regist
*/
	*(unsigned int *)0x88c17094 = PUTC_ENTRY; // default putc handler
	lp = (unsigned int *)0x88C001B4;
	lp[0] = MIPS_LUI(4,PUTC_ENTRY>>16);
	lp[2] = MIPS_ADDIU(4,4,PUTC_ENTRY);

	// patch point : removeByDebugSecion
	// KDebugForKernel_Unkonow_24c32559
	lp = (unsigned int *)0x88C01D6C;
	lp[0] = MIPS_JR(31);
	lp[1] = MIPS_ADDIU(2,0,0x0001); /* already 1 */

	// patch point : open
	patch_jal(0x88C0008C,hook_sceBootLfatOpen,&org_sceBootLfatOpen);
	// patch point : read
	patch_jal(0x88C000B8,hook_sceBootLfatRead,&org_sceBootLfatRead);
	// patch point : close
	patch_jal(0x88C000E4,hook_sceBootLfatClose,&org_sceBootLfatClose);

/*
bypass decrypr error enable plain txt
*/
	// non decrypt txt file , KILL decrypt entry point
	lp = (unsigned int *)0x88C0041C;
	lp[0] = MIPS_LUI(4,0);

	// clear error code
	lp = (unsigned int *)0x88C06728;
	lp[0] = MIPS_LUI(4,0);

	// patch point : sysmem.prx load & RUN
	lp = (unsigned int *)0x88C01020;
	lp[0] = MIPS_JAL((unsigned int)hook_start_sysmem);
	lp[1] = MIPS_ADDU(4,18,0);

	// loadcore load & RUN
	lp = (unsigned int *)0x88C01004;
	lp[0] = MIPS_ADDU(7,17,0);
	lp[2] = MIPS_JAL((unsigned int)hook_start_loadcore);
	lp[3] = MIPS_NOP;

	cache_clear();
}
