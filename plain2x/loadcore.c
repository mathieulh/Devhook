/*
	laodcore.prx patcher for plain2x
	FW 2.00 / 2.50 / 2.60 / 2.71 / 2.80 / 2.82 / 3.01
*/

#include <pspkernel.h>
#include <pspdebug.h>
#include <stdio.h>
#include "patch.h"

extern void cache_clear(void);

static u32 *loadcore_cseg;

extern void sceKernelDcacheWBinvAll(void);
extern void sceKernelIcacheClearAll(void);
extern unsigned int sceKernelProbeExecutableObject;

#define PLAIN2X_MODE
//#define HOOK_LOADEXECUTABLE
//#define HOOK_PROBEEXEC
//#define SHOW_CHECKEXEC_INFO
#include "loadcore.h"

/****************************************************************************
	show  CheckExec params
****************************************************************************/

#ifdef SHOW_CHECKEXEC_INFO
static void show_prx_data(int *data)
{
	int i;
	for(i=0;i<0x10/4;i++)
	{
		if( (i%4)==0)
			Kprintf("D %02X:",i*4);
		if( (i%4)==3)
			Kprintf("%08X\n",data[i]);
		else
			Kprintf("%08X ",data[i]);
	}
}

static void show_module_param(int *param)
{
	int i;
	for(i=0;i<0xc4/4;i++)
	{
		if( (i%4)==0)
			Kprintf("Param %02X:",i*4);
		if( (i%4)==3)
			Kprintf("%08X\n",param[i]);
		else
			Kprintf("%08X ",param[i]);
	}
	Kprintf("\n");
}
#endif

#ifdef HOOK_LOADEXECUTABLE
static int (*org_sceKernelLoadExecutableObject)(int *data,int *param);
int hook_sceKernelLoadExecutableObject(int *data,int *param)
{
	int result;
	char *modname;
	int *modinfo;

	modinfo  = &(data[ param[0x4c/4]/4 ]);
	modname = modinfo ? (char *)(&modinfo[1]) : "unknown";

	Kprintf("sceKernelLoadExecutableObject(%08X,%08X)[%s]\n",(int)data,(int)param,modname);

	result = org_sceKernelLoadExecutableObject(data,param);
	Kprintf("sceKernelLoadExecutableObject:%08X\n",result);
	return result;
}
#endif

#ifdef HOOK_PROBEEXEC
/****************************************************************************
****************************************************************************/
static int (*org_sceKernelProbeExecutableObject)(int *arg1,int *arg2);
int hook_sceKernelProbeExecutableObject(int *data,int *param)
{
	int result;
	Kprintf("sceKernelProbeExecutableObject(%08X,%08X)\n",(int)data,(int)param);
	result = org_sceKernelProbeExecutableObject(data,param);
	Kprintf("sceKernelProbeExecutableObject:%08X\n",result);
	// ~PSP ?
	int isPSP = (param[0]==0x5053507e);
	if( (result<0) && isPSP)
	{
		int type = param[0x7c] & 0xff;
		int key  = param[0xd0];
		Kprintf("Can't load TYPE:KEY = %02X:%08X\n",type,key);
	}

	return result;
}
#endif

/****************************************************************************
  hook CheckExec
 ***************************************************************************/
/*
exception 2.00 prx

1st
--Param 00:--00000000 --00000000 --00000050 --00000000
--Param 10:--00000000 --00000000 --00000000 --00000000
--Param 20:--00000000 --00000000 --00000000 --00000000
--Param 30:--00000000 --00000000 --00000000 --00000000
--Param 40:--00000000 --00000000 --00000000 --00000000
--Param 50:--00000000 --00000000 --00000000 --00000000
--Param 60:--00000000 --00000001 --00000000 --00000000
--CheckExecFile(8901F440,882FFE90)
--result 00000000
--Param 00:--00000000 --00000000 --00000050 --00000000
--Param 10:--00000E50 --00000000 --00000000 --00000000
--Param 20:--00000001 --00000000 --00000000 --00000000
--Param 30:--000024E0 --00000000 --00000000 --000013E0
--Param 40:--00000000 --00000001 --00000000 --000010B0
--Param 50:--00000000 --00000000 --00011007 --00001AB8
--Param 60:--00000000 --00000001 --00000000 --00000000

2nd
--Param 00:--00000000 --00000000 --00000050 --00000000
--Param 10:--00000E50 --00000000 --00000000 --00000000
--Param 20:--00000001 --882F9A00 --00000000 --00000000
--Param 30:--000024E0 --00000000 --00000000 --000013E0
--Param 40:--00000000 --00000001 --00000000 --000010B0
--Param 50:--00000000 --00000000 --00011007 --00001AB8
--Param 60:--00000000 --00000001 --00000000 --00000000
--CheckExecFile(8901F440,882FFE90)
--memlmd_c3a6f784(8901F440,00000E50)
--result 00000000
--sceUtilsGetLoadModuleABLength(8901F440,00000E50,882FFBE8
--result 00000000
--Param 00:--00000000 --00000000 --00000050 --00000000
--Param 10:--00000E50 --00000000 --00000000 --882F9A00
--Param 20:--00000001 --00000000 --00000000 --00000000
--Param 30:--000024E0 --00000000 --00000000 --000013E0
--Param 40:--00000000 --00000001 --00000001 --000010B0
--Param 50:--00000000 --00000001 --00011007 --00001AB8
--Param 60:--00000001 --00000001 --00000000 --00000000
--result 00000000

exception 2.00 ELF

--Param 00:--00000000 --00000000 --00000050 --00000000
--Param 10:--00000000 --00000000 --00000000 --00000000
--Param 20:--00000000 --00000000 --00000000 --00000000
--Param 30:--00000000 --00000000 --00000000 --00000000
--Param 40:--00000000 --00000000 --00000000 --00000000
--Param 50:--00000000 --00000000 --00000000 --00000000
--Param 60:--00000000 --00000001 --00000000 --00000000
--CheckExecFile(8901F440,882FFE90)
--result 00000000
--Param 00:--00000000 --00000000 --00000050 --00000000
--Param 10:--00000000 --00000000 --00000000 --00000000
--Param 20:--00000001 --00000000 --00000000 --00000000
--Param 30:--000024E0 --00000000 --00000000 --00000000
--Param 40:--00000000 --00000000 --00000000 --000010B0
--Param 50:--00000000 --00000000 --00000000 --00000000
--Param 60:--00000000 --00000001 --00000000 --00000000

-----------

FW1.50 sysmem ELF 
--Param 00:--00000000 --00000000 --00000000 --00000000
--Param 10:--00000000 --00000000 --00000000 --00000000
--Param 20:--00000001 --00000000 --00000000 --00000000
--Param 30:--0000FE18 --00000000 --00000000 --00000000
--Param 40:--00000000 --00000000 --00000000 --0000D570
--Param 50:--00000000 --00000000 --00000000 --00000000
--Param 60:--00000000 --00000000 --00000000 --00000000

FW1.50 loadcore ~PSP
--Param 00:--00000000 --00000000 --00000000 --00000000
--Param 10:--0000A3A0 --00000000 --00000000 --00000000
--Param 20:--00000001 --00000000 --00000AB8 --00000000
--Param 30:--00008C70 --00000000 --00000000 --000016D4
--Param 40:--00000000 --00000000 --00000001 --000065A4
--Param 50:--00000000 --00000000 --00003007 --0000A24C
--Param 60:--00000000 --00000000 --00000000 --00000000

checkexec params : size = 0x78

0008 l : api type
0010 l : ~PSP file size / 00000000
0014 l : memory allocated sizeH , align(param[0x30],0x100)
0018 l : sceKernelAllocPartitionMemory handle
001C l : top of source data pointer ? 882F9A00 / 00000000
0020 l : ~type 01=kernel prx,...err == 0xffffffff
0024 l : decrypt dest pointer
0028 l : entry point ?
0030 | : ~PSP decompress memory size
0034 l : file descripter
003c | : 000013E0 / 00000000
0040 l : memory pertation number
0044 l : ? 00000001 / 00000000
0048 l : 1 == ~PSP , 0 == plane prx
004c l : ? check sum ?
0054 l : 00000001 / 00000000 compress flag
0058 s : type 1000=kernel module,0800=VSH module,0200=user module / 0000
005a s : bit0 = compress flag
005c l : ~PSP decompressed data size ? / 00000000
0064 l : 1 =pre-decrypt mnark "$" in trap_memlmd_c3a6f784() ?


0068
006c
0070
0074

00c0 l : result of sceKernelLoadExecutableObject()

api type
 50:kernel module
 51:Kernel,
120:UMD boot
 10:plain prx? (GTA after load)
 20:sceATRAC3plus_Library
210:vsh_module

*/

/****************************************************************************
	copy ATTR ELF to ELF ~PSP header
****************************************************************************/
static int set_psp_attr(int *data,int *param)
{
	int *modinfo;
	int attr;

	modinfo  = &(data[ param[0x4c/4]/4 ]);
	attr     = modinfo[0];
	param[0x58/4]  = attr & 0xffff; 

#ifdef SHOW_CHECKEXEC_INFO
	int app_type = param[0x08/4];
	char *elf_name= (char *)(&modinfo[1]);
	Kprintf("copy ATTR ELF2PSP apptype %03X , attr %08X , name '%s'\n",app_type,attr,elf_name);
#endif
	return attr;
}

/****************************************************************************
	org sceKernelCheckExecFile entry
****************************************************************************/
static int (*org_sceKernelCheckExecFile)(int *arg1,int *arg2);

/****************************************************************************
	before sceKernelCheckExecFile
****************************************************************************/
int sceKernelCheckExecFile_before(int *data,int *param)
{
//	int result;
	int isELF;

	// ELF headr
	isELF = (data[0] == 0x464c457f);

#ifdef SHOW_CHECKEXEC_INFO
	if(isELF)
		show_module_param(param);
#endif

#ifdef SHOW_CHECKEXEC_INFO
	int isPSP;
	int *ptr;
	// "~SCE" header
	ptr = data;
	if(ptr[0]==0x4543537e) ptr += 0x40/4;
	isPSP = ptr[0]==0x5053507e;
	// show enter message
 	if(isPSP)
	{
		Kprintf("sceKernelCheckExecFile(%08X,%08X) ~PSP '%s'\n",(int)data,(int)param,((int)ptr)+10);
	}
	else
		Kprintf("sceKernelCheckExecFile(%08X,%08X) not ~PSP\n",(int)data,(int)param);
#endif

#if 0
	show_prx_data(data);
#endif
#if 0
	show_module_param(param);
#endif

#if 1
	if(isELF)
	{
		int app_type  = param[0x08/4];

		switch(app_type)
		{
		case 0x120: // UMD boot module
		case 0x141: // MS GAME module
			if(param[0x10/4])
			{
				// escape ProbeExec error at 2nd call
				param[0x44/4] = 1;
				param[0x48/4] = 1;

				set_psp_attr(data,param);
				return 0;
			}

		case 0x050: // kernel module : init
		case 0x051: // kernel module : many
			if(param[0x44/4])
			{
//Kprintf("Plain ELF: 2nd bypass\n");
				// need decrypt after camouflage ELF
				// set decrypted flag
				param[0x48/4] = 1;
				return 0;
			}
			break;
		}
	}
#endif
	return -1; // no hook return
}

/****************************************************************************
	after sceKernelCheckExecFile modify
****************************************************************************/
int sceKernelCheckExecFile_after(int *data,int *param,int isELF,int result)
{
#if 0//def SHOW_CHECKEXEC_INFO
	if(isELF)
		show_module_param(param);
#endif

	// camouflage plain ELF
	if(isELF)
	{
		int app_type  = param[0x08/4];

		switch(app_type)
		{
#if 0// def SHOW_CHECKEXEC_INFO
		case 0x000: // LoadModule/ID from Kernel
		case 0x010: // LoadModule/ID from User
			// when File type(LoadModule/ID),data is only 0x200 of head,
			// so can not judge user or kernel module,
			if(param[0x1c/4]!=0)
			{
				modinfo  = &(data[ param[0x4c/4]/4 ]);
				attr     = modinfo[0];
				char *elf_name= (char *)(&modinfo[1]);
				Kprintf("ELF apptype %03X , attr %08X , name '%s'\n",app_type,attr,elf_name);
			}
			else
			{
				Kprintf("ELF apptype %03X \n",app_type);
			}
			break;
#endif

#if 0
		case 0x120: // UMD boot module
		case 0x141: // MS GAME module , call with only 0x200 of head
			// set decrypt flag
			break;
#endif

		case 0x050: // kernel module : init
		case 0x051: // kernel module : many
//		case 0x020: // codec ? sceATRAC3plus_Library
			if( set_psp_attr(data,param) & 0xff00)
			{
				// set crypt flag if kernel/vsh module
				param[0x44/4] = 1;
				result = 0;
			}
			break;
#ifdef SHOW_CHECKEXEC_INFO
		default:
			Kprintf("ELF type %03X \n",app_type);
#endif
		}
	}
//	Kprintf("result %08x\n",ret);

#if 0
	// show dst buffer
	if(param[0x24/4])
		data = (int *)(param[0x24/4]);
	show_prx_data(data);
#endif
	return result;
}

/****************************************************************************
	hook sceKernelCheckExecFile entry
****************************************************************************/
int hook_sceKernelCheckExecFile(int *data,int *param)
{
	int result;
	int isELF;

	// rremenber API TYPE
//	int app_type  = param[0x08/4];

	if( sceKernelCheckExecFile_before(data,param)==0)
		return 0; // eary end

	isELF = (data[0] == 0x464c457f);

//	Kprintf("call sceKernelCheckExecFile(%08X,%08X)\n",(int)data,(int)param);
	// exec org CheckExec

	result = org_sceKernelCheckExecFile(data,param);
//Kprintf("result %08X\n",result);

	// restore API TYPE
//	param[0x08/4] = app_type;

	return sceKernelCheckExecFile_after(data,param,isELF,result);
}

/**************************************************************************:
	plain prx load patcher for 
	2.00/2.50/2.60/2.71
**************************************************************************/
int loadcore2x_patch(void)
{
	const LC_PP *p;
	int i;
	u32 *lp;
	u32 entry;
//	u32 *loadcore_cseg;

	// one function entry
	entry = (0x80000000 | ((sceKernelProbeExecutableObject & 0x03FFFFFF) << 2));

	// search version
	for(i=0,p=patch_point;i<MAX_LCPP;i++,p++)
	{
		loadcore_cseg = (u32 *)(entry - (p->sceKernelProbeExecutableObject_OFFSET));

		// compare sceKernelProbeExecutableObject and
		// version and name of ModuleInfo
		lp = &loadcore_cseg[p->sceKernelModInfo_OFFSET/4];

//Kprintf("%s %08X %08X\n",p->name,(int)lp,lp[0]);

		if(
			loadcore_cseg[p->sceKernelProbeExecutableObject_LIB/4]==entry &&
			lp[0]==p->sceKernelModInfo_VER && 
			lp[0]==p->sceKernelModInfo_VER && 
			lp[1]==0x4C656373             // sceL
			)
				goto found;
	}
	Kprintf("loadcore.prx unsupported version\n");
	while(1);
	//return 1;

found:
	Kprintf("loadcore.prx %s : %08X\n",p->name,(int)loadcore_cseg);

	// hook sceKernelCheckExecFile
	lp = &loadcore_cseg[p->sceKernelCheckExecFile_LIB/4];
	// hook Kernellib entry
	org_sceKernelCheckExecFile = (void *)(*lp);
	*lp = (u32)hook_sceKernelCheckExecFile;
	// static lib
	loadcore_cseg[p->sceKernelCheckExecFile_CALL1/4] = MIPS_JAL(hook_sceKernelCheckExecFile);
	loadcore_cseg[p->sceKernelCheckExecFile_CALL2/4] = MIPS_JAL(hook_sceKernelCheckExecFile);
	loadcore_cseg[p->sceKernelCheckExecFile_CALL3/4] = MIPS_JAL(hook_sceKernelCheckExecFile);

#ifdef HOOK_PROBEEXEC
	// hook KernelLib Entry
	lp = &loadcore_cseg[p->sceKernelProbeExecutableObject_LIB/4];
	org_sceKernelProbeExecutableObject = (void *)(*lp);
	*lp = (u32)hook_sceKernelProbeExecutableObject;
#endif

#ifdef HOOK_LOADEXECUTABLE
	//sceKernelLoadExecutableObject
	lp = &loadcore_cseg[p->sceKernelLoadExecutableObject_LIB/4];
	org_sceKernelLoadExecutableObject = (void *)(*lp);
	*lp = (u32)hook_sceKernelLoadExecutableObject;
#endif

/*
LoadCoreForKernel_Unkonow_54ab2675

"********************"
"PSP cannot load this image"
"unacceptable relocation type: 0x%7"

	jmp case 7 to case 0,same as 2.00

*/
	if(p->switch_table_54ab2675)
	{
		loadcore_cseg[(p->switch_table_54ab2675)/4+7] = loadcore_cseg[(p->switch_table_54ab2675)/4];
	}

#if 0
	// 2.80 : "Error : kernel module cannot link SYSCALL_EXPORT library"
	// already patched by msreboot.bin
	if(p->KernelMode_SYSCALL_link)
		loadcore_cseg[p->KernelMode_SYSCALL_link/4] = MIPS_LUI(8,0);
#endif

	clear_cache();

	return 0;
}
