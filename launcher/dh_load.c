/*
	PSP devhook setting / loader

	compatible as devhook 0.50.0000 - 0.51.0000
*/

#include "common.h"

const char devhook_path[] = "/kd/devhook.prx";

////////////////////////////////////////////////////////////////////////////////
// clear cache after patch
////////////////////////////////////////////////////////////////////////////////
static void clearCache(void)
{
	sceKernelDcacheWritebackAll();
	sceKernelIcacheClearAll();
}

////////////////////////////////////////////////////////////////////////////////
// load / start module
////////////////////////////////////////////////////////////////////////////////
int loadStartModule(const char *path)
{
	int mid;
	int result;

Kprintf("loadStartModule %s\n",path);

	mid = sceKernelLoadModule(path,0,0);

	if(mid==0x80020139)
	{
Kprintf("module %s already loaded\n",path);
		// already loaded
		return 0; // already loaded 
	}

	if(mid<0)
	{
Kprintf("module %s can't load\n",path);
		return mid;
	}

	result = sceKernelStartModule(mid,0,0,0,0);
	if(result < 0)
	{
Kprintf("module %s can't start\n",path);
		return result;
	}

Kprintf("module %s load/start ok\n",path);
	return mid;
}

////////////////////////////////////////////////////////////////////////////////
// FW1.50 enable plain kernel module
////////////////////////////////////////////////////////////////////////////////
extern void pspSdkInstallNoPlainModuleCheckPatch(void);

////////////////////////////////////////////////////////////////////////////////
// FW1.00 enable plain kernel module
////////////////////////////////////////////////////////////////////////////////
/*
  andi   r2,r4,$1000                        ;00003A1C[30821000,'...0']
  bne    r2,0,$00003b20                     ;00003A20[1440003F,'?.@.']

  ori    r3,r3,$0148                        ;00003AB0[34630148,'H.c4']
*/
#define MIPS_LUI(R,IMM) 0x3c000000|(R<<16)|((unsigned int)(IMM)&0xffff)

#if 0
static int pspSdkInstallNoPlainModuleCheckPatch_100(void)
{
	unsigned int *loadcore_cseg = (unsigned int *)0x8800f600;

	if(
		(loadcore_cseg[0x3a1c/4]==0x30821000) &&
		(loadcore_cseg[0x3a20/4]==0x1440003f)
	)
	{
		loadcore_cseg[0x3a1c/4] = MIPS_LUI(2,0);
		loadcore_cseg[0x3ab0/4] = MIPS_LUI(3,0);
		return 1;
	}
	return 0;
}
#endif

/****************************************************************************

	kill loaded module name for override load other version .

	original code is patchEntry() of MPH FW_launcher !

****************************************************************************/

typedef struct MainOption	// Main options
{
 u32 debugPrint;			// If 1, print debug message
 u32 oldSystem;				// If 1, use old root system instead new driver system
 u32 logFile;				// If 1, create a log file
} MainOption;

static MainOption mainOption;

int patchModuleName (const char *modName)
{
	SceModule *modMem;
	SceLibraryEntryTable *entryTable, *entryEnd;

	modMem = sceKernelFindModuleByName(modName);
	if(modMem==0)
	{
		Kprintf("Module '%s' not found\n");
		return 1;
	}
	// Print info

	if (mainOption.debugPrint)
	{
		Kprintf("modMem : 0x%X\n",((u32) modMem));
		Kprintf("modMem->modname : %s\n",modMem->modname);
		Kprintf("modMem->attribute : 0x%X\n",modMem->attribute);
	}

	// If bad module
	if ((((long) modMem) & 0xFF000000) != 0x88000000) return 1;
	if ((modMem->stub_top - modMem->ent_top) < 40) return 1;

	// Patch name and attributes
	modMem->attribute = 0x1006;
	modMem->modname[0] = '*';

	// Find entry table info
	entryTable = (SceLibraryEntryTable *) ((u32 *) modMem->ent_top);
	entryEnd = (SceLibraryEntryTable *) (((u8 *) modMem->ent_top) + modMem->ent_size);

	// Entry loop
	while (entryTable < entryEnd)
	{
		// Print info
		if (mainOption.debugPrint)
		{
			Kprintf("entryTable : 0x%X\n",((u32) entryTable));
			Kprintf("entryTable->libname : %s\n",((entryTable->libname) ? entryTable->libname : "NULL"));
			Kprintf("entryTable->stubcount : %d\n",entryTable->stubcount);
			Kprintf("entryTable->vstubcount : %d\n",entryTable->vstubcount);
		}
		// Wait else crashes ??
		//sceKernelDelayThread(20000);

		// Patch lib name
		if (entryTable->libname) ((char *) entryTable->libname)[0] = '*';

		// Next entry
		entryTable = (SceLibraryEntryTable *) (((u32 *) entryTable) + entryTable->len);
	}
	clearCache();
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// override load fw1.50 loadexec.prx for rboot from FW1.00
////////////////////////////////////////////////////////////////////////////////
static int load_loadexec(const char *path)
{
	// kill LoadCore
	if(patchModuleName("sceLoadExec"))
		return -1;

	// load reboot version of loadcore
	if( loadStartModule(path)<0 )
		return -1;
	return 0;
}

/****************************************************************************
	extend os for start and reboot devhook
****************************************************************************/
static int extend_pspos(void)
{
	int ver = sceKernelDevkitVersion();

	// Load devhook.prx
	switch( ver )
	{
	case 0x01000300: // 1.00
		return -1;
	case 0x01050001: // 1.50
		// enable plain prx
		pspSdkInstallNoPlainModuleCheckPatch();
		// support 2.00 crypted prx
//		if(loadStartModule(prx20_path)<0) return -1;
		break;
#if 0
	case 0x0200????
		break;
#endif
	default:
Kprintf("prx extender , unsupprted FW ver. %08X\n",ver);
		return -1; // Do Not supported other version
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////
// load devhook
////////////////////////////////////////////////////////////////////////////////
int dh_load(void)
{
	char path[128];
	char reboot_path[256];
	int dh_mid;
	const char *plist;
	SceUID result;

Kprintf("dh_load\n");

	// setup
	if(extend_pspos())
	{
		Kprintf("Can't extended prx handler\n");
		text_printf("Can't extended prx handler\n");
		sceKernelDelayThread(2000000);
		return -1; /* error */
	}

	GetRegistry("REBOOT_PATH",reboot_path,sizeof(reboot_path));
	plist=reboot_path;
	while(plist!=NULL)
	{
		plist = make_path_one(path,plist,devhook_path);

//text_printf("load %s\n",path);

		dh_mid = loadStartModule(path);
		if(dh_mid>=0) break;
	}
	if(strchr(path,':')) result= 1;
	else result = 2; /* from flash0 */

	if( dh_mid < 0 )
	{
		Kprintf("Can't load/start devhook\n");
		text_printf("Can't load/start devhook\n");
		sceKernelDelayThread(2000000);
		return -1;
	}

	// booted with devhook ?
	if(dh_mid == 0)
	{
		return 0; // alread loaded;
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////
// reboot system with VSH API
/////////////////////////////////////////////////////////////////////////////
static const char game_boot_sel[] = "game";
static unsigned int reboot_param2[0x400/4];

extern int LoadExecForKernel_Unkonow_1b97bdb3(const char *,void *);
extern int LoadExecForKernel_Unkonow_28d0d249(const char *,void *);
extern int sceKernelExitVSHVSH(int unk);

int dh_reboot_system(const char *path,int type)
{
	unsigned int params[9];

	params[0] = sizeof(params);
	params[1] = path ? strlen(path)+1 : 0;
	params[2] = (unsigned int)path;
	params[3] = (unsigned int)game_boot_sel;
	params[4] = 0x400;
	params[5] = (unsigned int)reboot_param2;
	params[6] = 0;
	params[7] = 0;
	params[8] = 0;

	memset(reboot_param2,0,sizeof(reboot_param2));
	reboot_param2[0] = sizeof(reboot_param2);
	reboot_param2[1] = 0x00000020;
	reboot_param2[0x90] = 1;
	reboot_param2[0x91] = 4;
	reboot_param2[0x92] = 0x13;
	reboot_param2[0xa0] = 1;

	switch(type)
	{
	case REBOOT_VSH: return sceKernelExitVSHVSH(0);
	case REBOOT_UMD: return LoadExecForKernel_Unkonow_1b97bdb3(path,&params);
	case REBOOT_MS:  return LoadExecForKernel_Unkonow_28d0d249(path,&params);
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static void copy_reg_str(const char *name)
{
	char val[256];

	memset(val,0,sizeof(val));
	if( GetRegistry(name,val,sizeof(val)-1) < 0) return;
	dhSetRegistry(name,val,strlen(val)+1);
}

static void copy_reg_dword(const char *name)
{
	char val[256];
	u32 value;
	memset(val,0,sizeof(val));
	if( GetRegistry(name,val,sizeof(val)-1) < 0) return;
	value = str2val(val);
	dhSetRegistry(name,&value,4);
}

/////////////////////////////////////////////////////////////////////////////
//	setup devhook registry from launcher registry
/////////////////////////////////////////////////////////////////////////////
void dh_setup_registry(void)
{
	u32 param[2];
	char val[256];
	char *p;

	copy_reg_dword("SFO_VER");
	copy_reg_dword("KPRINT_UART");

	// convert ascii/ascii to dword x2
	GetRegistry("CLOCK",val,sizeof(val));
	if(p=strchr(val,'/'))
	{
		*p = 0;
		param[0] = atoi(val);
		param[1] = atoi(p+1);
		if(param[0]&&param[1])
			dhSetRegistry("CLOCK",param,8);
	}

	param[0] = 0;
	GetRegistryDWORD("PRELOAD_ADDR",param);
	if(param[0])
		dhSetRegistry("PRELOAD_ADDR",param,4);

	copy_reg_str("FLASH0");
	copy_reg_str("FLASH1");
	copy_reg_str("UMD_PATH");
	copy_reg_str("REBOOT_PATH");
	copy_reg_str("BTCNF_PATH");

	// for VSHex
	copy_reg_dword("VSHEX_AUTOMENU");
	copy_reg_dword("VSHEX_UMD_DELAY");
}
