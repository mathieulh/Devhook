#ifndef __PATCH_H__
#define __PATCH_H__
/*
  2006.4.18 fix ADDIU and ORI RT bug
*/

/////////////////////////////////////
// inline assemble code , do not test all
/////////////////////////////////////
#define MIPS_NOP 0x00000000

#define MIPS_AND(RD,RS,RT)      (0x00000024|(RD<<11)|(RT<<16)|(RS<<21))
#define MIPS_ADDU(RD,RS,RT)     (0x00000021|(RD<<11)|(RT<<16)|(RS<<21))
#define MIPS_LUI(RD,IMM)        (0x3c000000|(RD<<16)|((unsigned int)(IMM)&0xffff))

#define MIPS_ADDI(RT,RS,IMM)    (0x24000000|(RS<<21)|(RT<<16)|((unsigned int)(IMM)&0xffff))
#define MIPS_ADDIU(RT,RS,IMM)   (0x24000000|(RS<<21)|(RT<<16)|((unsigned int)(IMM)&0xffff))
#define MIPS_ORI(RT,RS,IMM)     (0x34000000|(RS<<21)|(RT<<16)|((unsigned int)(IMM)&0xffff))


#define MIPS_BNE(RS,RT,OFFSET)  (0x14000000|(RS<<21)|(RT<<16)|(OFFSET&0xFFFF))

#define MIPS_J(ADDR)            (0x08000000|((((unsigned int)(ADDR))&0x0ffffffc)>>2))
#define MIPS_JAL(ADDR)          (0x0c000000|(((unsigned int)(ADDR)>>2)&0x03ffffff))
#define MIPS_JR(R)              (0x00000008|((R)<<21))
#define MIPS_SYSCALL(NUM)       (0x0000000C|((NUM)<<6))

#define MIPS_SW(RT,BASE,OFFSET) (0xac000000|(BASE<<21)|(RT<<16)|(OFFSET&0xFFFF))
#define MIPS_SH(RT,BASE,OFFSET) (0xa4000000|(BASE<<21)|(RT<<16)|(OFFSET&0xFFFF))

/////////////////////////////////////
// get API / module segment
/////////////////////////////////////
u32 *getModuleSeg(void *api_stub_entry,int api_module_offset);
u32 *getAPIEntry(void *api_stub_entry);

/////////////////////////////////////
// hook/restore direct API entry point
// kernel API only
// do not supported J,JAL,Branch instruction
/////////////////////////////////////

#define HOOK_FUNC_SPACE() asm("NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP;")
typedef struct hookAPIdirect_struct
{
	u32 patch_data[7];
	u32 *patch_point;
}HOOKAPI_resotre;

void *hookAPIDirect(void *api_stub_entry,void *hook_entry,HOOKAPI_resotre *restore);
void restoreAPIDirect(HOOKAPI_resotre *restore);

/////////////////////////////////////
// call after patch
/////////////////////////////////////
void clear_cache(void);

#endif
