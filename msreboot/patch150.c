/*
	reboot.bin fw1.50 patcher
*/

#include "common.h"

/****************************************************************************
	reboot.bin のパッチ
****************************************************************************/
void reboot150_patch(void)
{
	unsigned int *lp;
	unsigned int entry_addr;

	// 各種サブルーチンエントリ
	Kprintf    = (void *)0x88C01BB0;
	cache_clr1 = (void *)0x88C03F50;
	cache_clr2 = (void *)0x88C03B80;

	/////////////////////////////////////////////////////////////////////////
	// dbg_putcハンドラを設定
	entry_addr = (unsigned int)uart_dbg_putc;
	/////////////////////////////////////////////////////////////////////////

/*
  putc handler regist
*/
	*(unsigned int *)0x88c16d54 = entry_addr; // default putc handler
	lp = (unsigned int *)0x88C0023C;
	lp[0] = MIPS_LUI(4,entry_addr>>16);
	lp[2] = MIPS_ADDIU(4,4,entry_addr);

/*
	removeByDebugSecion フラグつぶし
	flag = 88C16d60 , bit 12 = Kprintf
*/
	lp = (unsigned int *)0x88C01D20;
	lp[0] = MIPS_JR(31);
	lp[1] = MIPS_ADDIU(2,0,0x0001); /* already 1 */

/*
	FLASHファイルアクセスフック
*/
	patch_jal(0x88C00084,hook_sceBootLfatOpen,&org_sceBootLfatOpen);
	patch_jal(0x88C000B4,hook_sceBootLfatRead,&org_sceBootLfatRead);
	patch_jal(0x88C000E0,hook_sceBootLfatClose,&org_sceBootLfatClose);
/*
  sysmem.prx load & RUN
*/
	lp = (unsigned int *)0x88C01014;
	lp[0] = MIPS_JAL((unsigned int)hook_start_sysmem);
	lp[1] = MIPS_ADDU(4,17,0);

/*
  loadcore load & RUN
*/
	lp = (unsigned int *)0x88C00FF8;
	lp[0] = MIPS_ADDU(7,2,0);
	lp[2] = MIPS_JAL((unsigned int)hook_start_loadcore);
	lp[3] = MIPS_NOP;

	cache_clear();
}
