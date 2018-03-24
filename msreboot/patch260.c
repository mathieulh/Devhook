/*
	reboot.bin fw2.50 patcher
*/

#include "common.h"

/****************************************************************************
	reboot.bin のパッチ
****************************************************************************/
void reboot260_patch(void)
{
	unsigned int *lp;
	unsigned int entry_addr;

	// 各種サブルーチンエントリ
	Kprintf    = (void *)0x88C01B4C;
	cache_clr1 = (void *)0x88C03EB8;
	cache_clr2 = (void *)0x88C039DC;
#if 1
/*
plain TXTの許可
*/
		lp = (unsigned int *)0x88C04FF0;
//		lp[0] = MIPS_SW(0,29,0x0000);
		lp[1] = MIPS_SW(5,29,0x0000);
		lp[2] = MIPS_ADDI(2,5,0);
#endif

	/////////////////////////////////////////////////////////////////////////
	// dbg_putcハンドラを設定
	entry_addr = (unsigned int)uart_dbg_putc;
	/////////////////////////////////////////////////////////////////////////

/*
  putc handler regist
*/
	*(unsigned int *)0x88c16f94 = entry_addr; // default putc handler
	lp = (unsigned int *)0x88C001BC;
	lp[0] = MIPS_LUI(4,entry_addr>>16);
	lp[2] = MIPS_ADDIU(4,4,entry_addr);

/*
	removeByDebugSecion フラグつぶし
*/
	lp = (unsigned int *)0x88C01CE8;
	lp[0] = MIPS_JR(31);
	lp[1] = MIPS_ADDIU(2,0,0x0001); /* already 1 */

/*
	FLASHファイルアクセスフック
*/
	patch_jal(0x88C0008C,hook_sceBootLfatOpen,&org_sceBootLfatOpen);
	patch_jal(0x88C000BC,hook_sceBootLfatRead,&org_sceBootLfatRead);
	patch_jal(0x88C000E8,hook_sceBootLfatClose,&org_sceBootLfatClose);

/*
bypass decrypr error enable plain txt
*/
	// non decrypt txt , KILL decrypt call
	lp = (unsigned int *)0x88C00448;
	lp[0] = MIPS_LUI(4,0);

	lp = (unsigned int *)0x88C06694;
	lp[0] = MIPS_LUI(4,0);

/*
  sysmem.prx load & RUN
*/
	lp = (unsigned int *)0x88C01010;
	lp[0] = MIPS_JAL((unsigned int)hook_start_sysmem);
	lp[1] = MIPS_ADDU(4,18,0);

/*
  loadcore load & RUN
*/
	lp = (unsigned int *)0x88C00FF4;
	lp[0] = MIPS_ADDU(7,17,0);
	lp[2] = MIPS_JAL((unsigned int)hook_start_loadcore);
	lp[3] = MIPS_NOP;

	cache_clear();
}
