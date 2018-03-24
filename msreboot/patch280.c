/*
	reboot.bin fw2.80 patcher
*/

#include "common.h"

#define PUTC_ENTRY ((unsigned int)uart_dbg_putc)

/****************************************************************************
	patch reboot.bin
****************************************************************************/
void reboot280_patch(void)
{
	unsigned int *lp;

	// sunroutine entry
	Kprintf    = (void *)0x88C00DF4;
	cache_clr2 = (void *)0x88C02C10;
	cache_clr1 = (void *)0x88C03244;

	// patch point : enable plain TXT
	lp = (unsigned int *)0x88C05438;
//	lp[0] = MIPS_SW(0,29,0x0000);
	lp[1] = MIPS_SW(5,29,0x0000);
	lp[2] = MIPS_ADDI(3,5,0);

/*
  putc handler regist
*/
	*(unsigned int *)0x88c17548 = PUTC_ENTRY; // default putc handler
	lp = (unsigned int *)0x88C001B8;
	lp[0] = MIPS_LUI(4,PUTC_ENTRY>>16);
	lp[2] = MIPS_ADDIU(4,4,PUTC_ENTRY);

	// patch point : removeByDebugSecion
	// KDebugForKernel_Unkonow_24c32559
	lp = (unsigned int *)0x88C00fcc;
	lp[0] = MIPS_JR(31);
	lp[1] = MIPS_ADDIU(2,0,0x0001); /* already 1 */

	// patch point : open
	patch_jal(0x88C0008C,hook_sceBootLfatOpen,&org_sceBootLfatOpen);
	// patch point : read
	patch_jal(0x88C000B8,hook_sceBootLfatRead,&org_sceBootLfatRead);
	// patch point : close
	patch_jal(0x88C000E4,hook_sceBootLfatClose,&org_sceBootLfatClose);

	// patch point : sysmem.prx load & RUN
	lp = (unsigned int *)0x88C050c4;
	lp[0] = MIPS_JAL((unsigned int)hook_start_sysmem);
	lp[1] = MIPS_ADDU(4,18,0);

	// loadcore load & RUN
	lp = (unsigned int *)0x88C050a8;
	lp[0] = MIPS_ADDU(7,17,0);
	lp[2] = MIPS_JAL((unsigned int)hook_start_loadcore);
	lp[3] = MIPS_NOP;

	// patch point : check hash error1
	lp = (unsigned int *)0x88C04878;
	lp[0] = MIPS_NOP;
	// patch point : check hash error2
	lp = (unsigned int *)0x88C048d0;
	lp[0] = MIPS_NOP;

	cache_clear();
}
