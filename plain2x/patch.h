/*
  2006.4.18 fix ADDIU and ORI RT bug
*/
#define MIPS_LUI(R,IMM) 0x3c000000|(R<<16)|((unsigned int)(IMM)&0xffff)

#define MIPS_ADDI(RT,RS,IMM)  0x24000000|(RS<<21)|(RT<<16)|((unsigned int)(IMM)&0xffff)
#define MIPS_ADDIU(RT,RS,IMM) 0x24000000|(RS<<21)|(RT<<16)|((unsigned int)(IMM)&0xffff)
#define MIPS_ORI(RT,RS,IMM) 0x34000000|(RS<<21)|(RT<<16)|((unsigned int)(IMM)&0xffff)
#define MIPS_NOP 0x00000000
#define MIPS_J(ADDR) (0x08000000 + ((((unsigned int)(ADDR))&0x0ffffffc)>>2))
#define MIPS_JR(R) (0x00000008 + ((R)<<21))
#define MIPS_JAL(ADDR) (0x0c000000 + (((unsigned int)(ADDR)>>2)&0x03ffffff))
#define MIPS_SYSCALL(NUM) (0x0000000C+((NUM)<<6))
#define MIPS_AND(RD,RS,RT)  (0x00000024|(RD<<11)|(RT<<16)|(RS<<21))
#define MIPS_ADDU(RD,RS,RT) (0x00000021|(RD<<11)|(RT<<16)|(RS<<21))

#define MIPS_SH(RT,BASE,OFFSET) (0xa4000000|(BASE<<21)|(RT<<16)|(OFFSET&0xFFFF))

#define MIPS_BNE(RS,RT,OFFSET) (0x14000000|(RS<<21)|(RT<<16)|(OFFSET&0xFFFF))

/* get kernel function entry from kernel mode stub */
#define EXTERN_KERNEL_FUNC_ENTRY(FUNC_NAME) extern unsigned int *_##FUNC_NAME
#define KERNEL_FUNC_ENTRY(FUNC_NAME) (void *)((((unsigned int)(_##FUNC_NAME)&0x03ffffff)<<2)|0x80000000)

/* hook kernel API from kernel memory space */
#define HOOK_API_KK(FUNC_NAME,USER_FUNC) \
{\
extern unsigned int *_##FUNC_NAME;\
unsigned int *pp = (unsigned int *)((((unsigned int)(_##FUNC_NAME)&0x03ffffff)<<2)|0x80000000);\
pp[0]=MIPS_J(USER_FUNC);\
pp[1]=MIPS_NOP;\
clear_cache();\
}\

/* hook kernel API from user memory space */
#define HOOK_API_KU(FUNC_NAME,USER_FUNC) \
{\
unsigned int *pp=KERNEL_FUNC_ENTRY(FUNC_NAME);\
pp[0]=MIPS_LUI(2,USER_FUNC>>16);\
pp[1]=MIPS_ORI(2,2,USER_FUNC>>16);\
pp[2]=MIPS_JR(2);\
pp[3]=MIPS_NOP;\
}\

int patch_kernel_version(unsigned int devkit_ver);
void clear_cache(void);

/* syscall patch */
int SetSystemcallVector(void);

unsigned int hook_user_api(void *stub_func_entry,void *hook_func_entry);

u32 *getModuleSeg(void *entry,int offset);
u32 *getAPIEntry(void *entry);
u32 *hookAPIDirect(void *entry,void *hook_entry);
