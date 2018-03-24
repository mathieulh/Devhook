/*
  BOOT with MS FW
*/

#include "common.h"

// log switch
#define LOG_SETUP 0

//#define SHOW_REBOOT_ARGS

#define USE_PRELOAD_ME_AREA 0

//
#define KEEP_PRX_BY_NAME 0

// short cut to return 1.50
#define RETURN_OS_MASK (ALL_ALLOW|ALL_TRIGGER|ALL_FUNCTION)
#define RETURN_OS_KEY  (PSP_CTRL_UP | PSP_CTRL_RTRIGGER | PSP_CTRL_LTRIGGER)

/****************************************************************************
****************************************************************************/

static const char conftbl_path[] = "/kd/pspcnf_tbl.txt";
const char normal_os_path[] = "ms0:/dh/kd";
const char flash_os_path[]  = "flash0:/dhcommon";
const char reboot_bin_path[]  = "/reboot.bin";
const char patcher_path[] = "/kd/msreboot.bin";
//const char patcher_path[] = "/msreboot.bin";

static int reboot_normal = 0;

/****************************************************************************
****************************************************************************/
static unsigned int *loadexec_cseg;
static unsigned int *loadexec_GZipPoint;

/* return function from reboot hook */
static int (*setup_return_func)(int arg1,int arg2,int arg3,int arg4,int arg5,int arg6);
static int (*setup2_return_func)(int arg1,int arg2,int arg3,int arg4,int arg5,int arg6);

/****************************************************************************
halt
*****************************************************************************/
static void halt(void)
{
	Kprintf("reboot HALT");
	while(1);
}

/****************************************************************************
goto reboot entry or reboot patcher
 ****************************************************************************/
/*
1st boot
--------------------------------------------------------
-hook_reboot_entry(88400040,882A9690,00000220,00000201):0
--------------------------------------------------------

umd boot
--------------------------------------------------------
-hook_reboot_entry(88400040,882E8190,00000120,D50A5DF6):0 UMD BOOT
--------------------------------------------------------

return vsh
--------------------------------------------------------
-hook_reboot_entry(88400040,8829EC90,00000210,7D834457):0
--------------------------------------------------------
*/
static void hook_reboot_entry(u32 arg1,u32 arg2,u32 arg3,u32 arg4)
{
	void (*func)(u32 arg1,u32 arg2,u32 arg3,u32 arg4);
#ifdef SHOW_REBOOT_ARGS
{
	int i;
	unsigned int *ptr;
	unsigned int *pchar;

Kprintf("-------------------------------------------------------\n");
Kprintf("hook_reboot_entry(%08X,%08X,%08X,%08X):%d\n",arg1,arg2,arg3,arg4,reboot_normal);

	// args表示
	Kprintf("reboot.img start\n");
	Kprintf("ARG1:",arg1);
	for(i=0;i<10;i++)
		Kprintf("%08X ",((unsigned int **)arg1)[i]);
	Kprintf("\n");

	Kprintf("ARG2:\n",arg2);
	for(i=0;i<8;i++)
		Kprintf("%08X ",((unsigned int **)arg2)[i]);
	Kprintf("\n");

	Kprintf("ARG3(api type) %08X\n",arg3);

	Kprintf("ARG14:");
	for(i=0;i<8;i++)
		Kprintf("%08X ",((unsigned int **)arg1)[4][i]);
	Kprintf("\n");

	Kprintf("ARG15:");
	for(i=0;i<8;i++)
		Kprintf("%08X ",((unsigned int **)arg1)[5][i]);
	Kprintf("\n");

	// ARG18
// 'disc0:/PSP_GAME/SYSDIR/EBOOT.BIN',00000021,00000002 VSH UMD boot
// '',00000400,00000100 : VSH

//  'disc0:/PSP_GAME/USRDIR/INV/INVMONO.prx',00000027,00000002

	pchar = (unsigned char *) (((unsigned char **)arg1)[8]);
	Kprintf("ARG18: boot path '%s',0x%X,0x%X\n",pchar[0],pchar[1],pchar[2]);

	// ARG2
	// boot name
// 'vsh'   '/kd/popbtconf.txt' : VSH 
// 'game' '/kd/popbtconf.txt' : VSH UMD boot
// 'game' 'NULL' LoadExec
	Kprintf("ARG23:boot select '%s'\n",((unsigned char **)arg2)[3]);
	Kprintf("ARG26:table name  '%s'\n",((unsigned char **)arg2)[6]);

/*
88400040
--ARG1[0] 88000000 // Kernel TOP address ?
--ARG1[1] 02000000
--ARG1[2] 00000000
--ARG1[3] 00000000
--ARG1[4] 88400840 pointer
--ARG1[5] 8841CBC0 pointer
--ARG1[6] 00000001
--ARG1[7] 00000001
--ARG1[8] 8841C840 : boot path
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

Kprintf("-------------------------------------------------------\n");
}
#endif
	func = reboot_normal ? (void *)0x88c00000 : (void *)PATCHER_ADDR;
	func(arg1,arg2,arg3,arg4);
}

#if 0
/****************************************************************************
check escape devhook
*****************************************************************************/
static int escape_devhook_req(void)
{
	SceCtrlData pad;

	// L+R+UP が押されてる時は、強制リターン
	sceCtrlPeekBufferPositive(&pad,1);
	return (pad.Buttons & RETURN_OS_MASK) == RETURN_OS_KEY;
}
#endif

#if 0
/****************************************************************************
make free area
*****************************************************************************/
/*
typedef struct SceModule {
	struct SceModule	*next;
	unsigned short		attribute;   // 04
	unsigned char		version[2];  // 06
	char				modname[27]; // 08
	char				terminal;
	unsigned int		unknown1;    // 24
	unsigned int		unknown2;    // 28
	SceUID				modid;       // 2c
	unsigned int		unknown3[4]; // 30
	void *				ent_top;     // 34
	unsigned int		ent_size;    // 38
	void *				stub_top;
	unsigned int		stub_size;   // 40

unk4[0] == StartModuleEntry


	unsigned int		unknown4[4]; // 44
	unsigned int		entry_addr;  // 54
	unsigned int		gp_value;    // 5c
	unsigned int		text_addr;   // 60
	unsigned int		text_size;   // 64
	unsigned int		data_size;
	unsigned int		bss_size;
	unsigned int		nsegment;
	unsigned int		segmentaddr[4];
	unsigned int		segmentsize[4];
} SceModule;
*/

int  LoadCoreForKernel_929b5c69(int *);
//SceModuleInfo *sceKernelFindModuleByUID(SceUID);
//int sceKernelReleaseModule(SceModule *);
//int sceKernelDeleteModule(SceModule *);
#endif


#if 1

/////////////////////////////////////////////////////////////////////////////////
int SysMemForKernel_a089eca4(u32 arg1,u32 arg2,u32 arg3);
HOOKAPI_resotre res_SysMemForKernel_a089eca4;

int hook_SysMemForKernel_a089eca4(u32 arg1,u32 arg2,u32 arg3)
{
	restoreAPIDirect(&res_SysMemForKernel_a089eca4);
//Kprintf("hook_SysMemForKernel_a089eca4(%08X,%08X,%08X)\n",arg1,arg2,arg3);
	return 0;
}

/****************************************************************************
  preload boot prx

sceKernelGetModuleListWithAlloc

 ****************************************************************************/
#if KEEP_PRX_BY_NAME
static const char *keep_list[] =
{
"sceSystemMemoryManager",
"sceLoaderCoreTool",
"sceExceptionManager",
"sceInterruptManager",
"sceSysclib",
"sceThreadManager",
"sceDMAManager",
"sceSystimer",
"sceIOFileManager",
"sceUart4",
"sceStdio",
"sceKernelUtils",
"sceMemlmd",
"sceModuleManager",
"sceInit",
"sceLoadExec",
"sceSYSREG_Driver",
"sceGPIO_Driver",
"scePWM_Driver",
"sceI2C_Driver",
"sceDMACPLUS_Driver",
"sceLCDC_Driver",
"sceNAND_Driver",
"sceDDR_Driver",
"sceGE_Manager",
"sceIdStorage_Service",
"sceSYSCON_Driver",
"sceRTC_Service",
"sceLFatFs_Driver",
"sceClockgen_Driver",
"sceWM8750_Driver",
"sceAudio_Driver",
"sceDisplay_Service",
"sceController_Service",
"sceLED_Service",
"sceHP_Remote_Driver",
"scePower_Service",
"sceMeCodecWrapper",
"sceUSB_Driver",
"sceOpenPSID_Service",
"sceSIRCS_IrDA_Driver",
"sceUmd_driver",
"sceATA_ATAPI_driver",
"sceUmdMan_driver",
"sceBLK_driver",
"sceUmd9660_driver",
"sceIsofs_driver",
"sceMScm_Driver",
"sceMSstor_Driver",
"sceMSFAT_Driver",
"sceWlan_Driver",
"sceVaudio_driver",
"sceRegistry_Service",
"sceUtility_Driver",
"sceMesgLed",
"sceImpose_Driver",
"sceKernelLibrary",
//"PSP DEVHOOK 0.4x FEP",
"DEVHOOK",

// 3.01 VSH
"sceLoaderCore",
"plain prx for 200-301",
"scePspNpDrm_Driver",
"UMD_ISO_CISO",
"sceWlanFirmMagpie_driver",
"sceMgr_Driver",
"sceMsAudio_Service",
"sceMGVideo_Service",
"sceSemawm",
"sceVaudio_driver",
"sceChkreg",
"sceAvcodec_wrapper",
//"sceVshBridge_Driver",
//"sceChnnlsv",
//"scePafHeaparea_Module",
//"sceATRAC3plus_Library",
//"scePaf_Module",
//"sceVshCommonGui_Module",
//"sceVshCommonUtil_Module",
//"vsh_module",
//"sceMpeg_library",
//"sceUSB_Stor_Driver",
//"sceUSB_Stor_Mgr_Driver",
//"sceUSB_Stor_Ms_Driver",
//"sceUSB_Stor_Boot_Driver",
"game_plugin_module",

// 3.01 game
/*
Fin  sceIrDA_Driver
Fin  UMD_ISO_CISO
Fin  sceAmctrl_driver
Fin  sceIoFilemgrDNAS
Fin  DemoDisc
Fin  sceVideocodec_Driver
Fin  sceAudiocodec_Driver
Fin  sceMpegbase_Driver
Fin  sceMpeg_library
Fin  sceATRAC3plus_Library
*/
NULL};
#endif

/****************************************************************************
 ****************************************************************************/
int is_keep_driver(SceModule *info)
{
#if 0
// show module info
Kprintf("\n%s\n",info->modname);
Kprintf("MID %08X : ATT %08X : U1  %08X : U2  %08X\n",info->modid , info->attribute , info->ent_top ,info->modname);
Kprintf("ENT %08X : ESZ %08X : STB %08X : SSZ %08X\n",info->ent_top,info->ent_size,info->stub_top,info->stub_size);
Kprintf("UK4 %08X : 48  %08X : 4C  %08X : 50  %08X\n",info->unknown4[0],info->unknown4[1],info->unknown4[2],info->unknown4[3]);
Kprintf("ETA %08X : GP  %08X : TXA %08X : XSZ %08X\n",info->entry_addr,info->gp_value,info->text_addr,info->text_size);
Kprintf("DSZ %08X : BSZ %08X : NSG %08X\n",info->data_size,info->bss_size,info->nsegment);

int j;
for(j=0;j<4;j++)
{
Kprintf("SEG %08X : SSZ %08X\n",info->segmentaddr[j],info->segmentsize[j]);
}
#endif

#if KEEP_PRX_BY_NAME
	const char **list;
	// check name
	for(list=keep_list;*list;list++)
	{
		if(strcmp(info->modname,*list)==0) return 1;
	}
#else
	if( (u32)info->ent_top >= 0x80000000 ) return 1;
#endif

	return 0;
}

int LoadCoreForKernel_929b5c69(int *);
HOOKAPI_resotre res_LoadCoreForKernel_929b5c69;

extern int dh_walker_tern;

/****************************************************************************
  sceKernelGetModuleListWithAlloc by sceKernelRebootBeforeForKernel
 ****************************************************************************/
int hook_LoadCoreForKernel_929b5c69(int *pcount)
{
	int i;
	int *modidp;
	SceUID memid , modid;
	SceModule *info;
	int count;
//
// sceKernelGetModuleListWithAlloc
//
	restoreAPIDirect(&res_LoadCoreForKernel_929b5c69);
	memid = LoadCoreForKernel_929b5c69(pcount);

	modidp = (int *)sceKernelGetBlockHeadAddr(memid);
	// sceKernelFindModuleByUID( r16[0x00] )

	count = 0;
	for(i=0;i<*pcount;i++)
	{
		modid = modidp[i];
		info = sceKernelFindModuleByUID(modid);

		if( is_keep_driver(info) )
		{
//Kprintf("Keep %s\n",info->modname);
		}
		else
		{
#if LOG_SETUP
Kprintf("Finish  %s\n",info->modname);
#endif
			modidp[count++] = modid;
		}
	}
	*pcount = count;

	// hook to kill sysmem term? in 1st call
	hookAPIDirect(SysMemForKernel_a089eca4,hook_SysMemForKernel_a089eca4,&res_SysMemForKernel_a089eca4);

	return memid;
}
#endif

/****************************************************************************
  preload boot prx
 ****************************************************************************/
int reboot_preload(void)
{
	int result;
#if LOG_SETUP
	Kprintf("reboot_preload\n");
#endif
	char reboot_path[128];
	char cnf_path[64];

	// stop Flash emu & ms_share
	// when still ms_share , cause overflow
	remove_flash_emu();

///////////////////////////////////////////////////////////////
// preload address
/////////////////////////////////////////////////////////////

#if USE_PRELOAD_ME_AREA
	sceSysregMeResetEnable();
	sceSysregVmeResetEnable();
	sceSysregMeBusClockDisable();
//	sceSysregMeResetDisable();
//	sceSysregVmeResetDisable(); 
	ramdisk_set_top_addr((void *)0x88380000,0x00080000);
//	ramdisk_set_top_addr((void *)0x88340000,0x000C0000);

#else
	void *preload_addr = (void *)RAMDISK_PRELOAD_DEF_ADDR;
	dhGetRegistry("PRELOAD_ADDR",&preload_addr,sizeof(preload_addr));

	if(((int)preload_addr)&0xffff) preload_addr = (void *)RAMDISK_PRELOAD_DEF_ADDR;
	ramdisk_set_top_addr(preload_addr,8*1024*1024);
#endif

///////////////////////////////////////////////////////////////
// set CWD 'flash0' for full mode
///////////////////////////////////////////////////////////////
	sceIoChdir("flash0:/");

///////////////////////////////////////////////////////////////
// get reboot path list
///////////////////////////////////////////////////////////////
	reboot_path[0]=0;
	dhGetRegistry("REBOOT_PATH",reboot_path,sizeof(reboot_path)); // multi path
//Kprintf("REBOOT_PATH=%s\n",reboot_path);

#if 0
///////////////////////////////////////////////////////////////
// return to original FW
/////////////////////////////////////////////////////////////
	// press L+R+START and reboot.bin in MS , force 1.50
	if( escape_devhook_req() )
	{
		dhSetRegistry("FLASH0",NULL,0); // mark return
	}

	if(dhGetRegistry("FLASH0",NULL,0)!=0)
	{
		// load FW1.50 reboot.bin
		result = ramdisk_save_file(flash_os_path,reboot_bin_path); // flash0
		if(result < 0)
			result = ramdisk_save_file(normal_os_path,reboot_bin_path); // ms
		if(result < 0) halt();

		reboot_normal = 1;
Kprintf("reboot normal Firmware\n");
		clear_cache();
		return 0;
	}
#endif

///////////////////////////////////////////////////////////////
// load reboot.bin
///////////////////////////////////////////////////////////////
	// search on flash0
	// flash0:用に'b'付き検索
//extern char drvString[];

//Kprintf("reboot.bin\n");
	result = ramdisk_save_file(reboot_path,reboot_bin_path);

	if(result <= -3) halt();
	if(result >=0)
	{
		//reboot_size = readed;
		//prx_buf += readed;
		// GZip CALLをつぶす。
		//  jal    sceKernelGzipDecompress            ;00002344[0C0009CE,'....']
		if(loadexec_GZipPoint)
			*loadexec_GZipPoint = MIPS_LUI(2,0);
	}

///////////////////////////////////////////////////////////////
// load reboot.bin patcher
///////////////////////////////////////////////////////////////
	// reboot path

//Kprintf("msreboot.bin\n");

	result = ramdisk_save_file(reboot_path,patcher_path);
	if(result < 0)
	{
		// 読めなければ起動できない
Kprintf("Can't load reboot patcher\n");
halt();
	}

///////////////////////////////////////////////////////////////
// load boot PRX to RAMDISK
///////////////////////////////////////////////////////////////
	strcpy(cnf_path,conftbl_path);
	dhGetRegistry("BTCNF_PATH",cnf_path,sizeof(cnf_path));

//Kprintf("boot prx\n");

	result = build_ramdisk(reboot_path,cnf_path);
	if( result < 0)
	{
Kprintf("Error PRX load\n");
//		halt();
	}
	// rename boot conf table
	ramdisk_rename(cnf_path,conftbl_path);

	// flush cache
	clear_cache();

	return 0;
}

/****************************************************************************
*****************************************************************************/
void reboot_preload2(void)
{
	// connect
	ramdisk_collect( (void *)REBOOT_ADDR );

	// move to final position
//	ramdisk_collect( (void *)RAMDISK_TOP );
}

/****************************************************************************
catch loadExec 1 (preload prx)
*****************************************************************************/
/*
  code_base     : Loadexec code base address == entry_of_sceKernelLoadExec-0x000010d0;
                  an entry of sceKernelLoadExec can get by stub of 'LoadExecForKernel'.
  boot_img_name : boot image 'PLANE BINALY FORMAT' (example "fatms0:/myboot.img" )
  address       : top of boot image (64K orign,original = 0x88c00000)
  size          : maximum program size (original = 0x00400000)

*/
int setup_reboot(int arg1,int arg2,int arg3,int arg4,int arg5,int arg6)
{
	int result;

	// term walker & reset clock
	// terminate DH thread
	dh_walker_tern = 1;
	while(dh_walker_tern<2)
		sceKernelDelayThread(10000);
	// change to normal clock
	// scePowerSetClockFrequency(222,222,111);

	// preload boot files
//white noize on LCD!!
//	KsceKernelExtendKernelStack(0x4000,(void *)reboot_preload,NULL);
	// return to an original func

#if 1
	// catch and choice terminate module by "sceKernelGetModuleListWithAlloc"
	hookAPIDirect(LoadCoreForKernel_929b5c69,hook_LoadCoreForKernel_929b5c69,&res_LoadCoreForKernel_929b5c69);
#endif
	// 1st call
	result = setup_return_func(arg1,arg2,arg3,arg4,arg5,arg6);

	// setup from m
	reboot_preload();

	// 2nd call
	result = setup_return_func(arg1,arg2,arg3,arg4,arg5,arg6);

	// build data
	reboot_preload2();

	return result;
}

/****************************************************************************
	load boot prx to RAMDISK
*****************************************************************************/
void *reboot_load(void)
{
	void *ptr;
	int size;
	int i;

#if LOG_SETUP
	Kprintf("reboot_load\n");
#endif

	// set first load address
#if 1
	void *addr = (void *)REBOOT_ADDR; // TMP reboot.bin area
#else
	// ダイナミックの場合はこう？
	void *addr = ramdisk_get_top_addr();
	if(addr > (void *)REBOOT_ADDR) addr = (void *)REBOOT_ADDR;
#endif

	// connect
//	ramdisk_collect( addr );

	// move to final position
	ramdisk_collect( (void *)RAMDISK_TOP );

	// load reboot patcher
	for(i=0;i<PATCHER_SIZE;i+=4)
	{
		((u32 *)PATCHER_ADDR)[i] = 0;
	}

	if((ptr = ramdisk_search(patcher_path,&size))!=NULL)
	{
		memcpy((void *)PATCHER_ADDR,ptr,size);
	}
	else
	{
Kprintf("no reboot patcher\n");
		*(u32 *)PATCHER_ADDR = MIPS_J(REBOOT_ADDR);
	}

	// set parameter for reboot patcher
	dhSaveRegistry((void *)DH_CONFIG_PARAM);               // registry data
	*(unsigned char **)RAMDISK_TOP_PARAM = (unsigned char *)RAMDISK_TOP; // TOP of RAMDISK

	// load reboot.bin
	if((ptr = ramdisk_search(reboot_bin_path,&size))!=NULL)
	{
// Kprintf("copy reboot.bin code\n");
		memset((void *)REBOOT_ADDR,0,REBOOT_SIZE);
		memcpy((void *)REBOOT_ADDR,ptr,size);
	}

	// flush cache
	clear_cache();
	return (void *)PATCHER_ADDR;
}

/****************************************************************************
catch loadExec 2
*****************************************************************************/
static int reboot_setup2(int arg1,int arg2,int arg3,int arg4,int arg5,int arg6)
{
	void *entry;

	// setup ramdisk
	entry = reboot_load();
//	entry = (void *)KsceKernelExtendKernelStack(0x4000,(void *)reboot_load,NULL);

	// return to org func
	// setup2_return_func(arg1,arg2,arg3,arg4,arg5,arg6);

	return 0;
}

/****************************************************************************
catch loadExec
*****************************************************************************/

/*
MS boot from VSH are 141-1
LoadExecForKernel_Unkonow_28d0d249(path,&params)

params[]
	00000024 size
	00000020 args
	08AF6530 argv "ms0:/PSP/GAME/.../..."
	09CEF390 "game"
	00000400 app type ?
	09CF344C pointer of...
	00000000
	00000000
	00000000

params[5][]
  0x00000400
  0x00000020
  0x00000000 ...

UMD boot from VSH are 120-1
LoadExecForKernel_Unkonow_1b97bdb3(path,&param)
	00000024 size
	00000020 args
	08AF6530 argv "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN"
 	09CEF390 "game"
	00000400 app type ?
	09CF344C pointer of...
	00000000
	00000000
	00000000

params[5][]
-00000400 -00000020 -00000000 -00000000 -00000000 -00000000 -00000000 -00000000
-00000000 -00000000 -00000000 -00000000 -00000000 -00000000 -00000000 -00000000
-00000000 -00000000 -00000000 -00000000 -00000000 -00000000 -00000000 -00000000
-00000000 -00000000 -00000000 -00000000 -00000000 -00000000 -00000000 -00000000

*/


/****************************************************************************
	FW2.80 loadexec hook
****************************************************************************/
extern void sceKernelExitVSHVSH(void);

int install_my_reboot2(const char *sm_path)
{
	u32 *lp;

/****************************************************************************
	3.01 / 3.02 / 3.03


|LoadExecForKernel        |A3D5E142|sceKernelExitVSHVSH             |000006E8|

M0_ModuleInfo
  srav   r6,r4,r8                           ;00001CF4[01043007,'.0..']

****************************************************************************/
	loadexec_cseg = getModuleSeg((void *)sceKernelExitVSHVSH,0x000006E8);
	if(loadexec_cseg && loadexec_cseg[0x00001CF4/4] == 0x01043007) // module ver.
	{
/*
  jal    sceKernelRebootBeforeForKernel     ;000018CC[0C0006AE,'....']
*/
		lp = &loadexec_cseg[0x000018CC/4];
		setup_return_func  = (void *)( ((lp[0]&0x03ffffff)<<2) | 0x80000000);
		lp[0] = MIPS_JAL(setup_reboot);

/*
before GZip,prx setup
SysMemForKernel_a089eca4 entry point

  jal    SysMemForKernel_a089eca4           ;00001688[0C0006C0,'....']
  jal    SysMemForKernel_a089eca4           ;000019A0[0C0006C0,'....']

*/
		lp = &loadexec_cseg[0x00001688/4];
		setup2_return_func  =(void *)&(loadexec_cseg[ *lp & 0x03ffffff ]); 
		*lp = MIPS_JAL(reboot_setup2);

		lp = &loadexec_cseg[0x000019a0/4];
//		*lp = MIPS_JAL(reboot_setup2);
		*lp = MIPS_NOP;

/*
GZip Decomp bypass point
  jal    UtilsForKernel_7dd07271            ;000016A0[0C0006E6,'....']
*/
		loadexec_GZipPoint = &(loadexec_cseg[0x000016A0/4]);

/*
hook reboot jmp
  lui    r1,$88c0                           ;000016DC[3C0188C0,'...<']
  jalr   r1                                 ;000016E0[0020F809,'.. .']
  addu   r7,r30,0                           ;000016E4[03C03821,'!8..']
*/
		loadexec_cseg[0x000016E0/4]   = MIPS_JAL(hook_reboot_entry);

		clear_cache();

#if LOG_SETUP
		Kprintf("LoadExec 3.0x\n");
#endif
		return 0;
	}




/****************************************************************************
	FW 2.82 try

|LoadExecForKernel        |A3D5E142|sceKernelExitVSHVSH             |00000824|

M0_ModuleInfo
  srav   r6,r4,r8                           ;00001CA0[01043007,'.0..']

****************************************************************************/
	loadexec_cseg = getModuleSeg((void *)sceKernelExitVSHVSH,0x0824);
	if(loadexec_cseg && loadexec_cseg[0x00001CA0/4] == 0x01043007) // module ver.
	{
/*
  jal    sceKernelRebootBeforeForKernel     ;00001978[0C00069A,'....']


  jal    UtilsForKernel_7dd07271            ;00001738[0C0006CD,'....']

  lui    r1,$88c0                           ;00001774[3C0188C0,'...<']
  jalr   r1                                 ;00001778[0020F809,'.. .']
  addu   r7,r30,0                           ;0000177C[03C03821,'!8..']
*/
/*

jal    sceKernelRebootBeforeForKernel     ;00001964[0C000693,'....']
sceKernelRebootBeforeForKernel
L00001A4C:
*/
		lp = &loadexec_cseg[0x00001978/4];
//		setup_return_func  = (void *)&(loadexec_cseg[ *lp & 0x03ffffff ]);
		setup_return_func  = (void *)( ((lp[0]&0x03ffffff)<<2) | 0x80000000);
		lp[0] = MIPS_JAL(setup_reboot);

/*
before GZip,prx setup
SysMemForKernel_a089eca4 entry point

  jal    SysMemForKernel_a089eca4           ;00000064[0C0006AC,'....']
  jal    SysMemForKernel_a089eca4           ;00001734[0C0006AC,'....']

*/
		lp = &loadexec_cseg[0x00001734/4];
		setup2_return_func  =(void *)&(loadexec_cseg[ *lp & 0x03ffffff ]); 
		*lp = MIPS_JAL(reboot_setup2);

		//lp = &loadexec_cseg[0x00000064/4];
		//*lp = MIPS_JAL(reboot_setup2);

/*
GZip Decomp bypass point
  jal    UtilsForKernel_7dd07271            ;0000174C[0C0006D4,'....']
*/
		loadexec_GZipPoint = &(loadexec_cseg[0x0000174c/4]);

/*
hook reboot jmp
  lui    r1,$88c0                           ;00001788[3C0188C0,'...<']
  jalr   r1                                 ;0000178C[0020F809,'.. .']
  addu   r7,r30,0                           ;00001790[03C03821,'!8..']
*/
		loadexec_cseg[0x0000178c/4]   = MIPS_JAL(hook_reboot_entry);

		clear_cache();

#if LOG_SETUP
		Kprintf("LoadExec 2.82\n");
#endif
		return 0;
	}

/****************************************************************************
	FW 2.80 try

UserModuleのsceKernelLoadExec()は引けないので

|LoadExecForKernel        |A3D5E142|sceKernelExitVSHVSH             |00000810|

M0_ModuleInfo 
  srav   r6,r4,r8                           ;00001C80[01043007,'.0..']
  nmadd.? f13,f3,f12,f5                     ;00001C84[4C656373,'sceL']
  cop1   $164616f                           ;00001C88[4564616F,'oadE']
  dsll   r12,r3,21                          ;00001C8C[00636578,'xec.']

****************************************************************************/
	loadexec_cseg = getModuleSeg((void *)sceKernelExitVSHVSH,0x0810);
	if(loadexec_cseg && loadexec_cseg[0x00001C80/4] == 0x01043007) // module ver.
	{
/*
  jal    sceKernelRebootBeforeForKernel     ;00001964[0C000693,'....']
//
  jal    SysMemForKernel_a089eca4           ;00000064[0C0006A5,'....']
  jal    SysMemForKernel_a089eca4           ;00001720[0C0006A5,'....']<<<

  jal    UtilsForKernel_7dd07271            ;00001738[0C0006CD,'....']

  lui    r1,$88c0                           ;00001774[3C0188C0,'...<']
  jalr   r1                                 ;00001778[0020F809,'.. .']
  addu   r7,r30,0                           ;0000177C[03C03821,'!8..']
*/
/*

jal    sceKernelRebootBeforeForKernel     ;00001964[0C000693,'....']
sceKernelRebootBeforeForKernel
L00001A4C:
*/
		lp = &loadexec_cseg[0x00001964/4];
//		setup_return_func  = (void *)&(loadexec_cseg[ *lp & 0x03ffffff ]);
		setup_return_func  = (void *)( ((lp[0]&0x03ffffff)<<2) | 0x80000000);
		lp[0] = MIPS_JAL(setup_reboot);

/*
before GZip,prx setup
SysMemForKernel_a089eca4 entry point

  jal    SysMemForKernel_a089eca4           ;00000064[0C0006A5,'....']
  jal    SysMemForKernel_a089eca4           ;00001720[0C0006A5,'....']<<<
*/
		lp = &loadexec_cseg[0x00001720/4];
		setup2_return_func  =(void *)&(loadexec_cseg[ *lp & 0x03ffffff ]); 
		*lp = MIPS_JAL(reboot_setup2);

		//lp = &loadexec_cseg[0x00000064/4];
		//*lp = MIPS_JAL(reboot_setup2);

/*
GZip Decomp bypass point
x  jal    UtilsForKernel_7dd07271            ;00001738[0C0006CD,'....']
*/
		loadexec_GZipPoint = &(loadexec_cseg[0x00001738/4]);

/*
hook reboot jmp
  lui    r1,$88c0                           ;00001774[3C0188C0,'...<']
  jalr   r1                                 ;00001778[0020F809,'.. .']
  addu   r7,r30,0                           ;0000177C[03C03821,'!8..']
*/
		loadexec_cseg[0x00001778/4]   = MIPS_JAL(hook_reboot_entry);

		clear_cache();

#if LOG_SETUP
		Kprintf("LoadExec 2.80\n");
#endif
		return 0;
	}

//////////////////////////////////////////////////////////////////////////////
// FW 2.71 try
// |LoadExecForKernel        |A3D5E142|sceKernelExitVSHVSH             |000015AC|
//////////////////////////////////////////////////////////////////////////////
	loadexec_cseg = getModuleSeg((void *)sceKernelExitVSHVSH,0x15AC);
	if(loadexec_cseg && loadexec_cseg[0x00002AD4/4] == 0x01043007)
	{
/*
  jal    sceKernelRebootBeforeForKernel     ;00002780[0C000A20,' ...']

//ここを捕まえる
  jal    SysMemForKernel_a089eca4           ;00002524[0C000A30,'0...']

  jal    UtilsForKernel_7dd07271            ;0000253C[0C000A5E,'^...']

  lui    r1,$88c0                           ;00002578[3C0188C0,'...<']
  jalr   r1                                 ;0000257C[0020F809,'.. .']
  addu   r7,r30,0                           ;00002580[03C03821,'!8..']
*/
		setup_return_func  = (void *)&(loadexec_cseg[0x0A20]);
		loadexec_cseg[0x00002780/4] = MIPS_JAL(setup_reboot);

		/* GZip手前を捕まえ、prxデータセットアップ */
		setup2_return_func  =(void *)&(loadexec_cseg[0x0A30]); 
		loadexec_cseg[0x00002524/4] = MIPS_JAL(reboot_setup2);

		/* GZipつぶしポイント */
		loadexec_GZipPoint = &(loadexec_cseg[0x0000253C/4]);

		// リブートジャンプをフック
		loadexec_cseg[0x0000257C/4]   = MIPS_JAL(hook_reboot_entry);

		clear_cache();
#if LOG_SETUP
		Kprintf("LoadExec 2.71\n");
#endif
		return 0;
	}
//	Kprintf("Unknown LoadExec version\n");
	return -1;
}


/****************************************************************************
****************************************************************************/
int install_my_reboot1(const char *sm_path)
{
	void *entry;
	// u32 *lp;

	// loadexec コードセグメントテスト
	entry = getAPIEntry(sceKernelLoadExec);
	if(!entry) return -1;

#if 0
/*
FW 1.00

  jal    sceKernelRebootBeforeForUser       ;00000EE8[0C00094C,'L...']
  jal    sceKernelSuspendAllUserThreads     ;00000F10[0C00096C,'l...']
  jal    sceIoDevctl                        ;00000F38[0C000938,'8...']
//
  jal    sceKernelRebootBeforeForKernel     ;00000F40[0C00094E,'N...']
//
  jal    $00000CA4                          ;00000F68[0C000329,')...'] // memclr +alpha

L00000CA4:
  jal    memset                             ;00000CE8[0C000964,'d...'] (88400000,0,0001cbc0)
  jal    sceKernelRemoveByDebugSection      ;00000D00[0C000940,'@...']
  jal    KDebugForKernel_Unkonow_24c32559   ;00000D0C[0C00093E,'>...']
  jal    KDebugForKernel_Unkonow_24c32559   ;00000D38[0C00093E,'>...']

L00000FF4:
  jal    sceKernelFindModuleByName          ;00001094[0C000944,'D...']
  jal    sceKernelQueryMemoryBlockInfo      ;000010B0[0C000954,'T...']
  jal    sceKernelQueryMemoryInfo           ;00001104[0C000950,'P...']
  jal    sceKernelQueryMemoryBlockInfo      ;00001114[0C000954,'T...']
  jal    $00001470                          ;00001160[0C00051C,'....'] // memmove
  jal    RebootForKernel_72e280a8           ;00001188[0C00092C,',...']
  addu   r6,r20,0                           ;0000118C[02803021,'!0..']
  j      $00000FD0                          ;00001190[080003F4,'....']
  lw     r31,$4c(r29)                       ;00001194[8FBF004C,'L...']
*/
	loadexec_cseg = (u32 *)( ((int *)entry) - (0x000017c0/4) ); 
	if(loadexec_cseg[0x0000270c/4] == 0x01011007)
	{
		// file setup
//		setup_return_func  = (void *)&(loadexec_cseg[0x94e]);
//		loadexec_cseg[0x00000f40/4] = MIPS_JAL(setup_reboot);
		setup_return_func  = (void *)&(loadexec_cseg[0x94c]);
		loadexec_cseg[0x00000ee8/4] = MIPS_JAL(setup_reboot);

		/* GZip手前を捕まえ、prxデータセットアップ */
		setup2_return_func  =(void *)&(loadexec_cseg[0x964]); 
		loadexec_cseg[0x00000ce8/4] = MIPS_JAL(reboot_setup2);

		/* GZipつぶしポイント */
		loadexec_GZipPoint = loadexec_cseg[0x00001160/4] ;

		// リブートジャンプをフック
		loadexec_cseg[0x00001188/4]   = MIPS_JAL(hook_reboot_entry);
#if LOG_SETUP
		Kprintf("LoadExec 1.00\n");
#endif
		return 0;
	}
#endif

/*
FW 1.50

  jal    $00002138 ;00002090

  jal    sceKernelRebootBeforeForUser       ;00002174[0C00099C,'....']
  jal    ModuleMgrForKernel_a6e8c1f5        ;00002190[0C00099E,'....']
  jal    ModuleMgrForKernel_a6e8c1f5        ;000021A4[0C00099E,'....']
  jal    sceKernelSuspendAllUserThreads     ;000021B4[0C0009BE,'....']
//
  jal    SysMemForKernel_95f5e8da           ;000021C8[0C0009AC,'....']
  jal    ModuleMgrForKernel_a6e8c1f5        ;000021D4[0C00099E,'....'] // free memory
  jal    sceUtilsGetLoadModuleABLength      ;00002204[0C0009D0,'....']
//
  jal    sceKernelRebootBeforeForKernel     ;00002214[0C0009A0,'....']
//
  jal    SysMemForKernel_95f5e8da           ;00002238[0C0009AC,'....']
  jal    $00000DB8                          ;00002240[0C00036E,'n...']

///////////////////////////////////////////

L000022CC:
  jal    SysMemForKernel_a089eca4           ;0000232C[0C0009A8,'....']
  jal    sceKernelGzipDecompress            ;00002344[0C0009CE,'....']
  jal    sceKernelDcacheWritebackAll        ;00002354[0C0009CA,'....']
  jal    sceKernelIcacheInvalidateAll       ;0000235C[0C0009CC,'....']
  lui    r1,$88c0                           ;00002384[3C0188C0,'...<']
  jalr   r1                                 ;00002388[0020F809,'.. .']
*/
	loadexec_cseg = (u32 *)( ((int *)entry) - (0x000010d0/4) ); 
	if(loadexec_cseg[0x00002880/4] == 0x01023007)
	{
		setup_return_func  = (void *)&(loadexec_cseg[0x9a0]);
		loadexec_cseg[0x2214/4] = MIPS_JAL(setup_reboot);
//		setup_return_func  = (void *)&(loadexec_cseg[0x9ac]);
//		loadexec_cseg[0x21c8/4] = MIPS_JAL(setup_reboot);

		/* GZip手前を捕まえ、prxデータセットアップ */
		setup2_return_func  =(void *)&(loadexec_cseg[0x9a8]); 
		loadexec_cseg[0x232C/4] = MIPS_JAL(reboot_setup2);
		/* GZipつぶしポイント */
		loadexec_GZipPoint = &(loadexec_cseg[0x00002344/4]);

		// リブートジャンプをフック
		loadexec_cseg[0x00002388/4]   = MIPS_JAL(hook_reboot_entry);

		clear_cache();
#if LOG_SETUP
		Kprintf("LoadExec 1.50\n");
#endif
		return 0;
	}

//////////////////////////////////////////////////////////////////////////////
// FW 2.00 try
//////////////////////////////////////////////////////////////////////////////
	loadexec_cseg = (u32 *)( ((int *)entry) - (0x00001334/4) );
	if(loadexec_cseg[0x00002A44/4] == 0x01033007)
	{
/*
  jal    sceKernelRebootBeforeForUser       ;00002340[0C000A04,'....']
  jal    ModuleMgrForKernel_a6e8c1f5        ;00002350[0C000A06,'....']
  jal    ModuleMgrForKernel_a6e8c1f5        ;00002360[0C000A06,'....']
  jal    sceKernelSuspendAllUserThreads     ;00002370[0C000A2E,'....']
  jal    SysMemForKernel_95f5e8da           ;0000237C[0C000A1A,'....']
  jal    ModuleMgrForKernel_a6e8c1f5        ;00002388[0C000A06,'....'] // free memory
//
  jal    sceUtilsGetLoadModuleABLength      ;000023AC[0C000A40,'@...']
//
  jal    sceKernelRebootBeforeForKernel     ;000023B4[0C000A08,'....']
//
  jal    SysMemForKernel_a089eca4           ;000024B4[0C000A16,'....']
  jal    sceKernelGzipDecompress            ;000024CC[0C000A3E,'>...']

  lui    r1,$88c0                           ;00002504[3C0188C0,'...<']
  jalr   r1                                 ;00002508[0020F809,'.. .']
  addu   r6,r19,0                           ;0000250C[02603021,'!0`.']
*/
		setup_return_func  = (void *)&(loadexec_cseg[0x2820/4]);
		loadexec_cseg[0x23b4/4] = MIPS_JAL(setup_reboot);

		/* GZip手前を捕まえ、prxデータセットアップ */
		setup2_return_func  =(void *)&(loadexec_cseg[0x2858/4]); 
		loadexec_cseg[0x24b4/4] = MIPS_JAL(reboot_setup2);

		/* GZipつぶしポイント */
		loadexec_GZipPoint = &(loadexec_cseg[0x000024CC/4]);

		// リブートジャンプをフック
		loadexec_cseg[0x00002508/4]   = MIPS_JAL(hook_reboot_entry);
#if 0
		// loadexecのしっぱいリブート
		hookAPIDirect(&loadexec_cseg[0x1ae0/4],trap_reboot);
		reboot_func  = (void *)&loadexec_cseg[0x1fa8/4];
#endif
		clear_cache();

#if LOG_SETUP
		Kprintf("LoadExec 2.00 at %08X\n",(int)loadexec_cseg);
#endif
		return 0;
	}

//////////////////////////////////////////////////////////////////////////////
// FW 2.60 try
//////////////////////////////////////////////////////////////////////////////
	loadexec_cseg = (u32 *)( ((int *)entry) - (0x0000131C/4) ); // sceKernelLoadExec
	if(loadexec_cseg[0x00002BB4/4] == 0x01043007)
	{
/*
//ここを捕まえるでは、2GBでエラー？
  jal    sceKernelRebootBeforeForKernel     ;00002860[0C000A56,'V...']

//ここを捕まえる
  jal    SysMemForKernel_a089eca4           ;00002604[0C000A66,'f...']
  jal    sceKernelGzipDecompress            ;0000261C[0C000A92,'....']

  lui    r1,$88c0                           ;00002658[3C0188C0,'...<']
  jalr   r1                                 ;0000265C[0020F809,'.. .']
  addu   r7,r30,0                           ;00002660[03C03821,'!8..']

  jal    sceKernelStartThread               ;00001C6C[0C000A8E,'....']

*/
		setup_return_func  = (void *)&(loadexec_cseg[0x0A56]);
		loadexec_cseg[0x00002860/4] = MIPS_JAL(setup_reboot);

		/* GZip手前を捕まえ、prxデータセットアップ */
		setup2_return_func  =(void *)&(loadexec_cseg[0x0A66]); 
		loadexec_cseg[0x00002604/4] = MIPS_JAL(reboot_setup2);

		/* GZipつぶしポイント */
		loadexec_GZipPoint = &(loadexec_cseg[0x0000261C/4]);

		// リブートジャンプをフック
		loadexec_cseg[0x0000265c/4]   = MIPS_JAL(hook_reboot_entry);

		clear_cache();
#if LOG_SETUP
		Kprintf("LoadExec 2.60 at %08X\n",(int)loadexec_cseg);
#endif
		return 0;
	}

//////////////////////////////////////////////////////////////////////////////
// FW 2.50 try
//////////////////////////////////////////////////////////////////////////////
	loadexec_cseg = (u32 *)( ((int *)entry) - (0x00001314/4) );
	if(loadexec_cseg[0x00002C24/4] == 0x01033007)
	{
/*
  jal    sceKernelSuspendAllUserThreads     ;000024E0[0C000AA1,'....']
//ここにしてみる
  jal    SysMemForKernel_95f5e8da           ;000024EC[0C000A8B,'....']
  jal    ModuleMgrForKernel_a6e8c1f5        ;00002504[0C000A71,'q...'] // free memory
//ここを捕まえるでは、2GBでエラー？
  jal    sceKernelRebootB0eforeForKernel     ;00002524[0C000A73,'s...']

//ここを捕まえる
  jal    SysMemForKernel_a089eca4           ;00002650[0C000A85,'....']
  jal    sceKernelGzipDecompress            ;00002668[0C000AB1,'....']

  lui    r1,$88c0                           ;000026A4[3C0188C0,'...<']
  jalr   r1                                 ;000026A8[0020F809,'.. .']
  addu   r7,r30,0                           ;000026AC[03C03821,'!8..']
*/
		setup_return_func  = (void *)&(loadexec_cseg[0x29cc/4]);
		loadexec_cseg[0x00002524/4] = MIPS_JAL(setup_reboot);
//		setup_return_func  = (void *)&(loadexec_cseg[0xa8b]);
//		loadexec_cseg[0x000024ec/4] = MIPS_JAL(setup_reboot);

		/* GZip手前を捕まえ、prxデータセットアップ */
		setup2_return_func  =(void *)&(loadexec_cseg[0x00002A14/4]); 
		loadexec_cseg[0x00002650/4] = MIPS_JAL(reboot_setup2);

		/* GZipつぶしポイント */
		loadexec_GZipPoint = &(loadexec_cseg[0x00002668/4]);

		// リブートジャンプをフック
		loadexec_cseg[0x000026a8/4]   = MIPS_JAL(hook_reboot_entry);

		clear_cache();
#if LOG_SETUP
		Kprintf("LoadExec 2.50 at %08X\n",(int)loadexec_cseg);
#endif
		return 0;
	}
	return -1;
}

/****************************************************************************
****************************************************************************/
int install_my_reboot(const char *sm_path)
{
	int result;
	result = install_my_reboot2(sm_path);
	if(result) result = install_my_reboot1(sm_path);
	if(result) return result;
	return result;
}
