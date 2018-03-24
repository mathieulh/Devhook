/*
  PSP reboot.bin / sysmem.prx / loadcore.prx patcher

	note:

	Kprintf broken (in registry.c)
	registry func hangup?

*/

#include "common.h"

#define MSREBOOT_MODE
#include "../plain2x/loadcore.h"

typedef int SceUID;
typedef int SceMode;
typedef long long  SceOff;

#include "../include/devhook.h"
#include "../src/ramdisk.h"
#include "../src/msboot.h"

// PARAMETER from msboot.c
//#define DH_CONFIG_PARAM   0x88bff000
//#define RAMDISK_TOP_PARAM 0x88bffff8

// direct Kprintf hook
#define HOOK_KPRINT_DIRTY 1

// load registry from memory
#define LOAD_REGISTRY 1 // error hangup

// debug message
#define LOG_CALL  0
#define SHOW_ARGS 0

//#define log_rd Kprintf
#define log_rd(text,...) {}

///////////////////////////////////////////////////////////////////////////////

static RD_HDR *ramdisk_top;
#ifdef RAMDISK_NOLOAD_LIST
static RD_HDR *noload_top;
#endif

/****************************************************************************
****************************************************************************/
//Kprintf Flasjh
int Kprintf_Hook_Flag = 0;

/****************************************************************************
****************************************************************************/
static void dummy_func(void) {}

void (*Kprintf)(const char *format,...) = (void *)dummy_func;
int (*cache_clr1)(void) = (void *)dummy_func;
void(*cache_clr2)(int) = (void *)dummy_func;

/****************************************************************************
****************************************************************************/
int strcmp2(const char *str1,const char *str2)
{
  int ret;
  while(1)
  {
    ret=(*str1)-(*str2);
    if(ret || *str1==0 ) break;
    str1++;
    str2++;
  }
  return ret;
}

/****************************************************************************
****************************************************************************/
void halt(void)
{
	while(1);
}

/*****************************************************************************
clear cache
*****************************************************************************/
void cache_clear(void)
{
  int cache_flag;
  cache_flag = cache_clr1();
  cache_clr2(cache_flag);
}

/*****************************************************************************
put string before Kprintf valid
*****************************************************************************/
static void put_str(const char *str)
{
	if(Kprintf_Hook_Flag==0) return;

	uart_dbg_putc(0,0x200);
	while(*str) uart_dbg_putc(0,*str++);
}

/*****************************************************************************
 RAMDISK read handling
*****************************************************************************/
static int catch_flag = 0;
static unsigned char *file_ptr;
static int file_size;
static int read1st;

void dump_ihex(void *top,int size);

void __memcpy(void *dst,void *src,int size)
{
	int i;

	for(i=0;i<size;i+=4)
	{
		*((unsigned int *)(dst+i)) = *((unsigned int *)(src+i));
	}
}

int open_ramdisk(const char *fname)
{
	int size;
	RD_HDR *cur;

log_rd("OPEN:");

	for(cur = ramdisk_top ; cur ; cur = cur->next)
	{
		if(strcmp2(fname,cur->name)==0)
		{
			size = cur->size;
			file_ptr  = (unsigned char *)(cur+1);
			file_size = cur->size;
log_rd("OP '%s' %08X[%5X]:",fname,(int)(cur+1),size);
			read1st= 1;

#ifdef DUMP_CONF_TXT
	// dump conf txt
	if( (fname[4]=='s') && (fname[5]=='y') )
	{
//log_rd("CONFTBL\n%s[EOF]\n",0x89000000);
//log_rd("CONF\n%s[EOF]\n",0x890001c0);

	// work area
	dump_ihex((void *)0x88c17000,0x1000);
	while(1);
#if 0
		// dump mem
		int i;
		for(i=0;i<0x1000;i+=16)
		{
//			unsigned char *p = (unsigned char *)0x890010c0 + i;
			unsigned char *p = (unsigned char *)0x88c17000 + i;
			Kprintf("%04X:%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
			i,
			p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],
			p[8],p[9],p[10],p[11],p[12],p[13],p[14],p[15]);
		}
#endif
	}
#endif

#ifdef RAMDISK_NOLOAD_LIST
/////////////////
// copy header
			if(cur->memid==0xffffffff)
			{
				__memcpy(noload_top,cur,sizeof(RD_HDR));
				noload_top->prev = (RD_HDR *)0;
				file_ptr = (void *)0xffffffff;
			}
/////////////////
#endif
			return 1;
		}
	}
	Kprintf("!!!!!!!!! Can't OPEN '%s' !!!!!!!!!\n",fname);
	file_ptr = 0;
	return -1;
}

int read_ramdisk(void *buf,int size)
{
//	int i;

	if(file_ptr==0) return -1;

	if(size > file_size) size = file_size;

	if(read1st)
	{
		log_rd("R %04X[%X",(int)buf,size);
#ifdef RAMDISK_NOLOAD_LIST
/////////////////
// copy header
		if(file_ptr==(void *)0xffffffff)
		{
			// regist no-load list
			noload_top->prev = (RD_HDR *)RAMDISK_NOLOAD_MAGIC;
			noload_top->next = (RD_HDR *)buf;
			noload_top++;
		}
/////////////////
#endif
	}
	else
	{
		log_rd("%04X",size);
	}
	read1st = 0;

	if(file_ptr!=(void *)0xffffffff)
	{
//		for(i=0;i<size;i++)
//			*(unsigned char *)buf++ = *file_ptr++;
		__memcpy(buf,file_ptr,size);
		file_ptr += size;
	}
	file_size -= size;
	return size;
}

/*****************************************************************************
sceBootLfatOpen
*****************************************************************************/
int (*org_sceBootLfatOpen)(char *arg1,int arg2,int arg3);
int hook_sceBootLfatOpen(char *arg1,int arg2,int arg3)
{
  if( open_ramdisk(arg1) < 0)
  {
	return -1;
//    return org_sceBootLfatOpen(arg1,arg2,arg3);
  }

  if(strcmp2(arg1,"/kd/usersystemlib.prx")==0)
   catch_flag = 1;
  return 0;
}

/****************************************************************************
sceBootLfatRead
****************************************************************************/
int (*org_sceBootLfatRead)(int arg1,int arg2,int arg3);
int hook_sceBootLfatRead(int arg1,int arg2,int arg3)
{
  int readed;
#if 0
  if(catch_flag)
  {
    unsigned int i;
    Kprintf("SOD");
    for(i = 0x89000000;i < (arg1+arg2);i++)
    {
      psp_uart_putc(*(unsigned char *)i);
    }
    Kprintf("EOD");
  }
#endif
  readed = read_ramdisk((void *)arg1,arg2);
  if(readed>=0)
    return readed;

  // flash
  return -1;
//  return org_sceBootLfatRead(arg1,arg2,arg3);
}

/****************************************************************************
sceBootLfatClose
****************************************************************************/
int (*org_sceBootLfatClose)(int arg1);
int hook_sceBootLfatClose(int arg1)
{
  log_rd(":CLOSE\n");

  if(file_ptr)
  {
    file_ptr = 0;
    return 0;
  }
  // flash
  return -1;
//  return hook_sceBootLfatClose(arg1);
}

/****************************************************************************
patch jump direct
****************************************************************************/
static void patch_branch(int patch_addr , int hook_addr)
{
  unsigned int *pp = (unsigned int *)patch_addr;
  pp[0] = MIPS_LUI(2,hook_addr>>16);
  pp[1] = MIPS_ORI(2,2,hook_addr&0xffff);
  pp[2] = MIPS_JR(2);
  pp[3] = MIPS_NOP;
}

/****************************************************************************
hook jal
****************************************************************************/
void patch_jal(int patch_addr , void *hook_addr,void **org_addr)
{
	unsigned int *pp = (unsigned int *)patch_addr;

	if(org_addr)
	{
		*org_addr = (void *)( ((pp[0]&0x03ffffff)<<2)|0x80000000 );
	}
	pp[0] = MIPS_JAL(((int)hook_addr)&0x0fffffff);

}

/****************************************************************************
  loadcore StartModule のフック
****************************************************************************/
extern int patch_loadcore(unsigned int *entry);

/*
	FW2.00 args

	arg1 : = 8
	arg2 : param

--  argv[0] 88400040
--  argv[1] 88C197C4

--  arg0[00] 88000000 : RAM top address?
--  arg0[04] 02000000 : RAM size ?
--  arg0[08] 00000002
--  arg0[0C] 00000046
--  arg0[10] 88400840 : user area top ?
--  arg0[14] 89000000 :
--  arg0[18] 00000001
--  arg0[1C] 00000001
--  arg0[20] 8841C840
--  arg0[24] FFFFFFFF
--  arg0[28] 00000000
--  arg0[2C] 00000000
--  arg0[30] 00000000
--  arg0[34] 00000000
--  arg0[38] 88C19448 : ? reboot.imgのRAM
--  arg0[3C] 00000000

--  arg1[00] 00000000
--  arg1[04] 00000006
--  arg1[08] 8800D5C4 : (sysmemの)最初にsceKernelRegisterLibraryするライブラリ列
--  arg1[0C] 8800D614
--  arg1[10] 8800D5F4
--  arg1[14] 8800D5E4
--  arg1[18] 8800D5D4
--  arg1[1C] 8800D604
--  arg1[20] 00000000
--  arg1[24] 00000000
--  arg1[28] 88012400
--  arg1[2C] 00000004 : (sysmemの)最初にsceKernelRegisterLibraryする数
--  arg1[30] 00000000
--  arg1[34] 00000000
--  arg1[38] 00000000
--  arg1[3C] 00000000
--  arg1[40] 88018DBC : sceKernelLinkLibraryEntriesの引数
--  arg1[44] 00000068
--  arg1[48] 88300000 : ? 
--  arg1[4C] 88C19950 : LoadCoreForKernel_aff947d4の引数 (sysmem)
--  arg1[50] 88C19A10 : LoadCoreForKernel_aff947d4の引数 (loadcore)
--  arg1[54] 00000000
--  arg1[58] 00000000
--  arg1[5C] 00000000
--  arg1[60] 8800B868 : （sysmemの）Kprintf エントリー
--  arg1[64] 88C04988 : sceUtilsGetLoadModuleABLength entry
--  arg1[68] 88C04EBC : memlmd_c3a6f784 entry
*/

void hook_start_loadcore(int arg1,void **arg2,int arg3,void *entry)
{
	void (*func)(int,void **,int);

	patch_loadcore( (unsigned int *)entry);

#if LOG_CALL
	Kprintf("***** GOTO loadcore %08X %08X *****\n\n",arg1,arg2);
#endif
/*
--  argc 00000008
--  argv[0] 88400040
--  argv[1] 88C18E9C
--  argv[2] 88400040
--  argv[3] 88C007A4
--  argv[4] 89000486
--  argv[5] 89000000
--  argv[6] 00008000
--  argv[7] 88B003FC
*/
#if SHOW_ARGS
{
	int i;
	Kprintf("  argc %08X\n",arg1,arg2);
	// arg2[0]
	for(i=0;i<arg1/4;i++)
	    Kprintf("  argv[%d] %08X\n",i,(int)(arg2[i]));

	for(i=0;i<0x40/4;i++)
	    Kprintf("  arg0[%02X] %08X\n",i*4,((int *)(arg2[0]))[i]);
	for(i=0;i<0x6c/4;i++)
	    Kprintf("  arg1[%02X] %08X\n",i*4,((int *)(arg2[1]))[i]);
}
#endif
  func = entry;
  func(arg1,arg2,arg3);
}

/****************************************************************************
  hook sysmem StartModule
****************************************************************************/
extern unsigned int *get_kprintf_entry(void *func);

int hook_start_sysmem(int arg1,void **arg2)
{
  int (*func)(int,void **);
  unsigned int *sysmem_kprintf;

  func = (void *)arg1;
  arg1 = 4; // must be set 4

#if LOG_CALL
  Kprintf("***** CALL sysmem module %08X(%08X,%08X) *****\n",(int)func,arg1,arg2);
#endif
/*
--  argc 00000004
***** CALL sysmem module 00000004 88C18D20 *****
--  argc 00000004
--  argv[0] 00000000
--  argv[1] 88000000
--  argv[2] 02000000
--  argv[3] 88000000
--  argv[4] 00300000
--  argv[5] 88300000
--  argv[6] 00100000
--  argv[7] 08400000
--  argv[8] 00400000
--  argv[9] 08800000
--  argv[10] 01800000
--  argv[11] 00000000
--  argv[12] 00000000
--  argv[13] 00000003
--  argv[14] 00000006
--  argv[15] 00000000
--  argv[16] 00000000
--  argv[17] 00000001
--  argv[18] 8841C840
--  argv[19] 88B009E0 : dbgputf handler
--*/
#if SHOW_ARGS
{
	int i;
	  Kprintf("  argc %08X\n",arg1,arg2);
//  for(i=0;i<arg1;i++)
  for(i=0;i<(0x50/4);i++)
    Kprintf("  argv[%d] %08X\n",i,(int)(arg2[i]));
}
#endif

#if HOOK_KPRINT_DIRTY
	// get Kprintf entry
	sysmem_kprintf = get_kprintf_entry(func);
//	Kprintf("Sysmem CSEG %08X %08X\n",(int)sysmem_cseg,sysmem_kprintf);

	if( Kprintf_Hook_Flag && sysmem_kprintf)
	{
		patch_branch((int)sysmem_kprintf,(int)Kprintf);
		cache_clear();
	}
#endif

//  func = (void *)arg1;
  return func(4,arg2);
}

/***********************************************************************
***********************************************************************/

int _strcmp2(const char *p1,const char *p2)
{
	while(*p1)
	{
		if(*p1++ != *p2++) return 1;
	}
	return 0;
}

/***********************************************************************
  search reboot.bin ver
***********************************************************************/
// "$LastChangedRevision: XXXXX $"
static const char lastChangeStr[] = "$LastChangedRevision: ";

static char *search_version(void)
{
	u32 i;
	for(i=0x88c00000;i<0x88c3ffff;i+=4)
	{
		if( *(u32 *)i == ('$'|('L'<<8)|('a'<<16)|('s'<<24)) )
		{
			if(!_strcmp2(lastChangeStr,(const char *)i))
			{
				return (char *)(i);
			}
		}
	}
	return NULL;
}

/***********************************************************************
  patch reboot.bin

  kprintf : sio_putc
  hook sysmem call entry
  hook loadcore jump
***********************************************************************/
extern void reboot150_patch(void);
extern void reboot200_patch(void);
extern void reboot250_patch(void);
extern void reboot260_patch(void);
extern void reboot271_patch(void);
extern void reboot280_patch(void);
extern const char *reboot300_patch(void);

static void patch_rebootimg(void)
{
	char *ptr = search_version();
	const char *name = NULL;

	// check address of "$LastChangedRevision: "
	switch((u32)ptr)
	{
	case 0x88c16c9c: // 19196
		name = "1.50";
		reboot150_patch();
		break;
	case 0x88c17490: // 24432
		name = "2.00";
		reboot200_patch();
		break;
	case 0x88c15f34: // 28464
		name = "2.50";
		reboot250_patch();
		break;
	case 0x88c159f0: // 28504
		name = "2.60";
		reboot260_patch();
		break;
	case 0x88c15AF0: // 28504
		name = "2.71";
		reboot271_patch();
		break;
	case 0x88C15FA4: // 34654
		name = "2.8x";
		reboot280_patch();
		break;
	case 0x88C13f1c: // 36865==3.00 , 36985==3.01 , 3731330==3.02 , 37313432=3.03
		name = reboot300_patch();
		break;
	default:
		put_str("reboot.bin unknown version , Can't patched\n");
		halt();
	}
	// show version
	put_str("reboot.bin=");
	put_str(name);
	put_str("\n");
}

/****************************************************************************
	msreboot.bin entry point
****************************************************************************/
int entry(int arg1,int arg2,int bootcode,int dummy)
{
	void (*reboot_entry)(int arg1,int arg2,int bootcode,int dummy);

//put_str("msreboot entry\n");

	// parameters from msboot.c
#if LOAD_REGISTRY
	dhLoadRegistry((void *)DH_CONFIG_PARAM);     // Registry
#endif
	ramdisk_top = *(RD_HDR **)RAMDISK_TOP_PARAM; // RAMDISK
#ifdef RAMDISK_NOLOAD_LIST
	// non preload list store buffetr
	noload_top  = (RD_HDR *)RAMDISK_NOLOAD_LIST;
	noload_top->prev = 0;
#endif

	// ref by uart_dbg_putc()
#if LOAD_REGISTRY
	dhGetRegistry("KPRINT_UART",&Kprintf_Hook_Flag,4);
#endif

	/* patch kprintf handler */
	patch_rebootimg();
#if SHOW_ARGS
{
	int i;
	unsigned int *pchar;

	// args表示
	Kprintf("reboot.img start\n");
	Kprintf("ARG1 %08X\n",arg1);
	Kprintf("ARG2 %08X\n",arg2);
	Kprintf("ARG3(api type) %08X\n",bootcode);

	for(i=0;i<8;i++)
		Kprintf("ARG14[%d] %08X\n",i,((unsigned int **)arg1)[4][i]);
	for(i=0;i<8;i++)
		Kprintf("ARG15[%d] %08X\n",i,((unsigned int **)arg1)[5][i]);

	// ARG18
// 'disc0:/PSP_GAME/SYSDIR/EBOOT.BIN',00000021,00000002 VSH UMD boot
// '',00000400,00000100 : VSH

//  'disc0:/PSP_GAME/USRDIR/INV/INVMONO.prx',00000027,00000002

	pchar = (unsigned char **) (((unsigned char **)arg1)[8]);
	Kprintf("ARG18: boot path   '%s',0x%X,0x%X\n",pchar[0],pchar[1],pchar[2]);

#if 0
	if( strcmp("",pchar[0])==0) pchar[0] = "disc0:/SYSDIR/
#endif
	// ARG2
	// boot name
// 'vsh'   '/kd/popbtconf.txt' : VSH 
// 'game' '/kd/popbtconf.txt' : VSH UMD boot
// 'game' 'NULL' LoadExec
	Kprintf("ARG23:boot select '%s'\n",((unsigned char **)arg2)[3]);
	Kprintf("ARG26:table name  '%s'\n",((unsigned char **)arg2)[6]);
}

/*
88400040
--ARG1[0] 88000000 // Kernel TOP address ?
--ARG1[1] 02000000
--ARG1[2] 00000000
--ARG1[3] 00000000
--ARG1[4] 88400840
--ARG1[5] 8841CBC0
--ARG1[6] 00000001
--ARG1[7] 00000001
--ARG1[8] 8841C840 : {filename,?,?}
--ARG1[9] FFFFFFFF
--ARG1[10] 00000000

8826AE90
--ARG2[0] 00000024
--ARG2[1] 00000000
--ARG2[2] 00000000
--ARG2[3] 8826AEC0 : boot select name 'game'
--ARG2[4] 00000000
--ARG2[5] 00000000
--ARG2[6] 8826AFC0 : boot table name NULL / 'pspconftbl.txt'
--ARG2[7] 00000000
--ARG2[8] 00010000
--ARG2[9] FFFFFFFF
--ARG2[10] FFFFFFFF
--ARG2[11] FFFFFFFF
8826AEC0

*/
#endif

  /* goto reboot */
  reboot_entry = (void *)0x88c00000;
  (reboot_entry)(arg1,arg2,bootcode,dummy);

  return 0;
}

// DUMMY , entry point is entry()
int main() { return 0; }
