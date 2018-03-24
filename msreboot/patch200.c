/*
	reboot.bin fw2.00 patcher
*/

#include "common.h"

/****************************************************************************
	reboot.bin のパッチ
****************************************************************************/
void reboot200_patch(void)
{
	unsigned int *lp;
	unsigned int entry_addr;

	// 各種サブルーチンエントリ
	Kprintf    = (void *)0x88C01C5C;
	cache_clr1 = (void *)0x88C040D8;
	cache_clr2 = (void *)0x88C03BFC;

/*
plain TXTの許可
*/
		lp = (unsigned int *)0x88C05058;
//		lp[0] = MIPS_SW(0,29,0x0000);
		lp[1] = MIPS_SW(5,29,0x0000);
		lp[2] = MIPS_ADDI(2,5,0);

	/////////////////////////////////////////////////////////////////////////
	// dbg_putcハンドラを設定
	entry_addr = (unsigned int)uart_dbg_putc;
	/////////////////////////////////////////////////////////////////////////

/*
  putc handler regist
*/
	*(unsigned int *)0x88c17554 = entry_addr; // default putc handler
	lp = (unsigned int *)0x88C0024C;
	lp[0] = MIPS_LUI(4,entry_addr>>16);
	lp[2] = MIPS_ADDIU(4,4,entry_addr);

/*
	removeByDebugSecion フラグつぶし
*/
	lp = (unsigned int *)0x88C01DE4;
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
#if 1
	// non decrypt txt , KILL decrypt call
	lp = (unsigned int *)0x88C004CC;
	lp[0] = MIPS_LUI(4,0);
#endif
	lp = (unsigned int *)0x88C06604;
	lp[0] = MIPS_LUI(4,0);

/*
  sysmem.prx load & RUN
*/
	lp = (unsigned int *)0x88C010EC;
	lp[0] = MIPS_JAL((unsigned int)hook_start_sysmem);
	lp[1] = MIPS_ADDU(4,18,0);

/*
  loadcore load & RUN
*/
	lp = (unsigned int *)0x88C010D0;
	lp[0] = MIPS_ADDU(7,17,0);
	lp[2] = MIPS_JAL((unsigned int)hook_start_loadcore);
	lp[3] = MIPS_NOP;

	cache_clear();
}
