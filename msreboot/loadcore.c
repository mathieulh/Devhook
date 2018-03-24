/*
  PSP loadcore patch for msreboot.bin
*/

#include "common.h"

// debug message
// #define SHOW_CHECKEXEC_INFO

/****************************************************************************
****************************************************************************/
#define MSREBOOT_MODE
#include "../plain2x/loadcore.h"

/****************************************************************************
  patch loadcore
****************************************************************************/
static unsigned int *loadcore_cseg;

#ifdef SHOW_CHECKEXEC_INFO
/****************************************************************************
  CheckExecのparam表示
****************************************************************************/
static void show_prx_data(int *data)
{
#if 1
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
#endif
}

static void show_module_param(int *param)
{
#if 1
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
#endif
}
#endif

/****************************************************************************
  CheckExecのtrap
****************************************************************************/

static int (*org_sceKernelCheckExecFile)(int *arg1,int *arg2);
static int hook_sceKernelCheckExecFile(int *data,int *param)
{
	int result;
	int isPSP;
	int isELF;
	int *ptr;

//	Kprintf("\n");

	// "~SCE" header
	ptr = data;
	if(ptr[0]==0x4543537e) ptr += 0x40/4;
	isPSP = ptr[0]==0x5053507e;

	isELF = (data[0] == 0x464c457f);

#ifdef SHOW_CHECKEXEC_INFO
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

	if(isELF && param[0x44/4])
	{
		// 偽装後のdecrypt要求

		// set decrypted flag
		param[0x48/4] = 1;
		return 0;
	}

#if 0
	if(isELF)
	{
		// plain ELFの偽装工作

		Kprintf("Plain ELF:");

		// decompの代わりにコピー
//		if(param[0x24/4])
		if(param[0x44/4])
		{
			// 2nd call
			unsigned char *p1,*p2;
			int i;

//			Kprintf("Plain ELF:2nd\n");

			// decompressのコピー
			if(param[0x24/4])
			{
				// decomp先がある時
				p1 = (unsigned char *)(param[0x24/4]);
				p2 = (unsigned char *)data;
				for(i=0;i<param[0x30/4];i++)
					*p1++ = *p2++;

				// データTOP更新
				param[0x1c/4] = param[0x24/4];
				// dstポインタはクリア
				param[0x24/4] = 0;
			}
			else
			{
				// dstなしの時はdataが
				param[0x1c/4] = (int)data;
			}

			// 展開OKで１になる？
			param[0x48/4] = 1;

			// よく分からないが~PSPと同じに
			param[0x54/4] = 1;
			param[0x60/4] = 1;

			result = 0;
		}
		else
		{
			Kprintf("Plain ELF:1st\n");

			// ２重decryptは強制OFF
			param[0x64/4] = 0;

			// 1st call
//	Kprintf("call sceKernelCheckExecFile(%08X,%08X)\n",(int)data,(int)param);
			result = org_sceKernelCheckExecFile(data,param);

			if(result==0)
			{
				// crypt flag
				param[0x44/4] = 1;

				// ０とそうでないときがある。
				param[0x28/4] = 0; // ?

				// ~PSP のATTRはELF内からコピー
				// compress flagは立てない
				param[0x58/4]  = ( data[ param[0x4c/4]/4 ] ) & 0xffff;

				// prxの元ファイルサイズ？、情報がないので適当
				param[0x10/4] = param[0x30/4];

				// 必要メモリ量,align 256でよい
				param[0x14/4] = (param[0x30/4]+0xff) & ~0xff;

				// 不明、適当に
				param[0x3c/4]   = param[0x30/4]; // flag ?
				param[0x5c/4]   = param[0x30/4]; // ELFファイルサイズ
			}
		}
	}
	else
#endif
	{
//		Kprintf("call sceKernelCheckExecFile(%08X,%08X)\n",(int)data,(int)param);
		result = org_sceKernelCheckExecFile(data,param);
	}

#ifdef SHOW_CHECKEXEC_INFO
	if(isELF)
		show_module_param(param);
#endif

	if(isELF)
	{
		int api_type  = param[0x08/4];
		int *modinfo  = &(data[ param[0x4c/4]/4 ]);
		int attr      = modinfo[0];
#ifdef SHOW_CHECKEXEC_INFO
		char *elf_name= (char *)(&modinfo[1]);
		Kprintf("ELF type %03X , attr %08X , name '%s'\n",api_type,attr,elf_name);
#endif
		switch(api_type)
		{
		case 0x050: // kernel module : init
		case 0x051: // kernel module : many
//		case 0x120: // UMD boot module
//		case 0x010: // user module?? (GTA after boot)
		case 0x020: // codec ? sceATRAC3plus_Library
			if(attr & 0xff00)
			{
				// kernel prx,vshの場合、ELFを偽装
				param[0x44/4] = 1;
#ifdef SHOW_CHECKEXEC_INFO
Kprintf("Modify ELF to Crypt Module\n");
#endif
				// decrypt flagを立てる
				param[0x44/4] = 1;

				// ~PSPのATTRをELF moduleinfo からコピー
				// 2.0は and orで、typeはここから参照される
				param[0x58/4]  = attr & 0xffff;

				result = 0;
			}
			break;
		}
	}
//	Kprintf("result %08x\n",ret);
#if 0
	show_module_param(param);
#endif

#if 0
	// show dst buffer
	if(param[0x24/4])
		data = (int *)(param[0x24/4]);
	show_prx_data(data);
#endif
	return result;
}

/****************************************************************************
****************************************************************************/
int patch_loadcore(unsigned int *entry)
{
	const LC_PP *p;
	int i;
	u32 *lp;

	// FW1.50
	loadcore_cseg = entry-(0x00000AB8/4);
	if(
		(loadcore_cseg[0x2ff8/4]==0x30871000) &&
		(loadcore_cseg[0x3060/4]==0x3C118002)
	)
	{
		// enable plain module of kernel/user/vsh,...
		loadcore_cseg[0x2FEC/4] = MIPS_LUI(16,1);
		Kprintf("FW1.50 loadcore patched\n");
		cache_clear();
		return 0;
	}
	// FW1.00
	loadcore_cseg = entry-(0xCF4/4);
	if(
		(loadcore_cseg[0x3a1c/4]==0x30821000) &&
		(loadcore_cseg[0x3a20/4]==0x1440003f)
	)
	{
		// kernel plane
		loadcore_cseg[0x3a1c/4] = MIPS_LUI(2,0);
		// user plane
		loadcore_cseg[0x3ab0/4] = MIPS_LUI(3,0);

		Kprintf("FW1.00 loadcore patched loadcore_cseg=%08x\n",(int)loadcore_cseg);
		cache_clear();
		return 0;
	}

	// FW 2.00-3.01
	// search version
	for(i=0,p=patch_point;i<MAX_LCPP;i++,p++)
	{
		loadcore_cseg = entry - (p->moduleStart_OFFSET/4);

		// compare sceKernelProbeExecutableObject and
		// version and name of ModuleInfo
		lp = &loadcore_cseg[p->sceKernelModInfo_OFFSET/4];

//Kprintf("%s %08X %08X\n",p->name,(int)lp,lp[0]);

		if(
			lp[0]==p->sceKernelModInfo_VER && 
			lp[0]==p->sceKernelModInfo_VER && 
			lp[1]==0x4C656373             // sceL
			)
				goto found;
	}
	Kprintf("loadcore.prx unsupported version\n");
	while(1);

found:

	Kprintf("loadcore.prx %s : %08X\n",p->name,(int)loadcore_cseg);

	// hook sceKernelCheckExecFile
	lp = &loadcore_cseg[p->sceKernelCheckExecFile_LIB/4];

	// hook Kernellib entry
	org_sceKernelCheckExecFile = (void *)(*lp);
	//*lp = (u32)hook_sceKernelCheckExecFile; // do not hook lib entry
	// static lib
	loadcore_cseg[p->sceKernelCheckExecFile_CALL1/4] = MIPS_JAL(hook_sceKernelCheckExecFile);
	loadcore_cseg[p->sceKernelCheckExecFile_CALL2/4] = MIPS_JAL(hook_sceKernelCheckExecFile);
	loadcore_cseg[p->sceKernelCheckExecFile_CALL3/4] = MIPS_JAL(hook_sceKernelCheckExecFile);

	// 2.80 : "Error : kernel module cannot link SYSCALL_EXPORT library"
	if(p->KernelMode_SYSCALL_link)
		loadcore_cseg[p->KernelMode_SYSCALL_link/4] = MIPS_LUI(8,0);

	return 1;
}
