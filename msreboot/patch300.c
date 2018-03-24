/*
	reboot.bin 3.00-3.03 patcher
*/

#include "common.h"

#define PUTC_ENTRY ((unsigned int)uart_dbg_putc)

// key seed : bfc00200
#define XD 0x53

#if 0
static const unsigned char key_param[16] = {
0xdd^XD,0xa3^XD,0x70^XD,0x6f^XD,
0x01^XD,0xee^XD,0xc3^XD,0xb9^XD,
0x94^XD,0xc3^XD,0x14^XD,0x25^XD,
0xbe^XD,0x1c^XD,0xab^XD,0x9f^XD
};
#endif

#if 0
// key base : reboot.bin[88C132D4]
static const unsigned char key_seed[16] = {
0x42,0xc4,0x6a,0x15,
0x23,0x1d,0x9a,0xb2,
0x3e,0xae,0xb0,0xe3,
0x35,0xcc,0xa8,0xe8};
#endif

// final key : reboot.bin[88C15180] const data == dummy!
static const u8 key_data_300[16] = {
	0x9F^XD, 0x67^XD, 0x1A^XD, 0x7A^XD,0x22^XD, 0xF3^XD, 0x59^XD, 0x0B^XD, 
	0xAA^XD, 0x6D^XD, 0xA4^XD, 0xC6^XD,0x8B^XD, 0xD0^XD, 0x03^XD, 0x77^XD};

static const u8 key_data_303[16] = {
	0x7b^XD, 0xa1^XD, 0xe2^XD, 0x5a^XD, 0x91^XD, 0xb9^XD, 0xd3^XD, 0x13^XD,
	0x77^XD, 0x65^XD, 0x4a^XD, 0xb7^XD, 0xc2^XD, 0x8a^XD, 0x10^XD, 0xaf^XD};

/****************************************************************************
	build key data
****************************************************************************/
static void buld_key_param(const u8 *key_real)
{
	int i;
	u8 *dst  = (u8 *)(0xbfc00200 - 0x35231452); // KEY param
	u8 *src  = (u8 *)(0x88c132d4 - 0x35231452); // KEY seed
	// key seed
	for(i=0;i<0x10;i++)
	{
		dst[0x35231452+i] = src[0x35231452+i] ^ key_real[i] ^ XD;
	}
}


/****************************************************************************
	patch reboot.bin
****************************************************************************/
const char *reboot300_patch(void)
{
	unsigned int *lp;
	const char *name;

	// sunroutine entry
	Kprintf    = (void *)0x88C00D58;
	cache_clr2 = (void *)0x88c02118; // 0x88C0210C;
	cache_clr1 = (void *)0x88C0274C;

	// patch point : enable plain TXT
		lp = (unsigned int *)0x88C049C8;
//		lp[0] = MIPS_SW(0,29,0x0000); // sw     r7,$0(r29)
		lp[1] = MIPS_SW(5,29,0x0000); // sw     r7,$0(r29)
		lp[2] = MIPS_ADDI(3,5,0);     // addiu  r3,0,-$1

	//patch point : putc handler regist2
	//   lui    r1,$xxxx
	//   sw     r4,$xxxx(r1)
	*(unsigned int *)0x88c15404 = PUTC_ENTRY; // default putc handler

	// patch point : putc handler regist1
	lp = (unsigned int *)0x88C0018C;
	lp[0] = MIPS_LUI(4,PUTC_ENTRY>>16); // lui    r4,$88c0
	                                    // jal    $xxxxxxxx
	lp[2] = MIPS_ADDIU(4,4,PUTC_ENTRY); // addiu  r4,r4,$xxxx

	// patch point : removeByDebugSecion
	// KDebugForKernel_Unkonow_24c32559
	lp = (unsigned int *)0x88C00F3C;
	lp[0] = MIPS_JR(31);            //  lui    r2,$8002                           ;88C00F3C[3C028002,'...<']
	lp[1] = MIPS_ADDIU(2,0,0x0001); //  sltiu  r6,r4,$40                          ;88C00F40[2C860040,'@..,']

	// patch point : init error
	lp = (unsigned int *)0x88C00084;
	lp[0] = MIPS_NOP; //  bltz   r2,$xxxxxxxx

	// patch point : open
	patch_jal(0x88C0008C,hook_sceBootLfatOpen,&org_sceBootLfatOpen);
	// patch point : read
	patch_jal(0x88C000B8,hook_sceBootLfatRead,&org_sceBootLfatRead);
	// patch point : close
	patch_jal(0x88C000E4,hook_sceBootLfatClose,&org_sceBootLfatClose);

#if 0
	// bypass decrypr error enable plain txt
	lp = (unsigned int *)0x88C038F8;
	lp[0] = MIPS_LUI(4,0);       // addiu  r4,r4,$xxxx
#endif

	// patch point : sysmem.prx load & RUN
	lp = (unsigned int *)0x88C04654;
	lp[0] = MIPS_JAL((unsigned int)hook_start_sysmem); // jalr   r18
	lp[1] = MIPS_ADDU(4,18,0);                         // addiu  r4,0,$4

	// loadcore load & RUN
	lp = (unsigned int *)0x88C04638;
	lp[0] = MIPS_ADDU(7,17,0);                           // jr     r17
	                                                     // addu   r29,r21,0
	lp[2] = MIPS_JAL((unsigned int)hook_start_loadcore); // nop
	lp[3] = MIPS_NOP;                                    // j $xxxxxxxx

	// patch point : check hash error1
	lp = (unsigned int *)0x88C03E08;
	lp[0] = MIPS_NOP;
	// patch point : check hash error2
	lp = (unsigned int *)0x88C03E60;
	lp[0] = MIPS_NOP;

	// make decrypt key param from decrypt TAG
	name = "unknown";
	lp = (u32 *)(0x88c132e4 - 0x16fe7250);
	switch(lp[0x16fe7250/4]-0x45897245)
	{
	case 0xCFEF06F0-0x45897245: // 3.00 - 3.02
		buld_key_param(key_data_300);
		name = "3.02";
		break;
	case 0xCFEF07F0-0x45897245: // 3.03
		buld_key_param(key_data_303);
		name = "3.03";
		break;
	}
	return name;
	
	cache_clear();
}
