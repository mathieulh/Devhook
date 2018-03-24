/*
	PSP ANYUMD emulation
*/
#include "common.h"

extern void *sceUmd9660Init(void);
extern void *sceUmd9660GetDrive(void);

extern int KsceUmd_driver_086ddc0d(int,void *);
extern int KsceUmd_driver_2d81508d(int,void *);


extern int sceUmdManGetDiscInfo(int *ptr);

//#define SHOW_PVD_SFO 

#define EXT_STACK_SIZE 0x2000

/////////////////////////////////////////////////////////////////////////////
// configration value
/////////////////////////////////////////////////////////////////////////////
static u32 sfover = 0;
//static u32 anyumd = 0;
//static u32 noumd  = 0;

/////////////////////////////////////////////////////////////////////////////
// patch restore data
/////////////////////////////////////////////////////////////////////////////
static u32 *umd_read_point = NULL;
static u32 umd_read_value;
static u32 *stub_entry_point = NULL;
static u32 stub_entry_value;

static HOOKAPI_resotre res_sceUmdManGetDiscInfo;
static HOOKAPI_resotre res_sceGpioPortRead;

/****************************************************************************
	SFO version search
****************************************************************************/
static inline int get_version_2xx(char *ptr)
{
	u32 m1,m2;
	if(ptr[0]!='2' || ptr[1]!='.') return 0;

	 m1 = (u32)ptr[2] -'0';
	 m2 = (u32)ptr[3] -'0';

	if(m1>9 || m2>9) return 0;

	return 0x0200 | (m1<<4) | m2;
}
/****************************************************************************
	SFO version patcher
****************************************************************************/
#define PVD_LBA 0x10
#define SFO_SEARCH_TIMEOUT 256

static int sfo_lba     = 0;
static int sfo_timeout = SFO_SEARCH_TIMEOUT;

static void patch_sfo(void *buf,int len,u32 *param)
{
	char *top = (char *)buf;
	if(sfo_lba)
	{
		// when SFO LBA are founded  , release when PVD read
		if(param[0x02] == PVD_LBA)
		{
#ifdef SHOW_PVD_SFO 
Kprintf("clear SFO LBA\n");
#endif
			sfo_lba     = 0;
			sfo_timeout = SFO_SEARCH_TIMEOUT;
			return;
		}
	}
	else if(sfo_timeout>0)
	{
		// when SFO LBA are not founded  , search SFO
		sfo_timeout--;
		if(
			top[0]==0x00 && 
			top[1]=='P'  && 
			top[2]=='S'  && 
			top[3]=='F'
		)
		{
			sfo_lba = param[0x02];
#ifdef SHOW_PVD_SFO 
Kprintf("set SFO LBA = %x\n",sfo_lba);
#endif
		}
	}
	// check SFO sector
	if(sfo_lba != param[0x02]) return;

	// search version code in SFO sector
	int i;
//Kprintf("check SFO file len=%X\n",len);
	for(i=0;i<len-4;i+=4)
	{
		int ver = get_version_2xx(&top[i]);
		if(ver)
		{
#ifdef SHOW_PVD_SFO 
Kprintf("change SFO version %3X -> %03X\n",ver,sfover);
#endif
			top[i+0] = '0'+( (sfover>>16)&0x0f);
//			top[i+1] = '.';
			top[i+2] = '0'+( (sfover>>8)&0x0f);
			top[i+3] = '0'+( (sfover)&0x0f);
			break;
		}
	}
}

/****************************************************************************
	UMD READ CALL
****************************************************************************/
/*
	drvWork[0x00-0x03] : pointer
	drvWork[0x04-0x??] : LE="0dmu:" , BE="umd0:"
	drvWork[0x24-0x27] : ffffffff
	drvWork[0x28-0x2b] : 00000001 
	drvWork[0x3c-0x63] : falgs ? , GAME=E0000800 , updater= E0010800
	                     3c-63 == GET_CAPACIRY DATA
	drvWork[0x64-0x67] : == sceUmdManGetUmdDrive(0)
*/
static int read_umd_iso_file_core(int *argv)
{
	int *drvWork = (int *)argv[0];
	void *buf    = (void *)argv[1];
	int len      = argv[2];
	u32 *param   = (u32 *)argv[3];
	return umd_read_block(drvWork,buf,len,param);
}

/****************************************************************************
	generic UMD read emulation
****************************************************************************/
static int (*org_hook_umdread)(int *drvWork,void *buf,int len,u32 unk01,u32 *param) = NULL;
static int hook_umdread(int *drvWork,void *buf,int len,u32 unk01,u32 *param)
{
	int result;
#if 0
	Kprintf("hook_umdread(%08X,%08X,%08X,%d,%08X):%08X\n",drvWork,buf,len,unk01,param,result);
	Kprintf("LBA %08X\n",param[0x02]);
#endif
	if(is_real_umd())
	{
		// USE UMD
		result = org_hook_umdread(drvWork,buf,len,unk01,param);
	}
	else
	{
		// UMD EMU
		//result = umd_read_block(drvWork,buf,len,param);
		int argv[4];
		argv[0] = (int)drvWork;
		argv[1] = (int)buf;
		argv[2] = (int)len;
		argv[3] = (int)param;
		result = (int)sceKernelExtendKernelStack(EXT_STACK_SIZE,(void *)read_umd_iso_file_core,(void *)&argv);
	}
	if(sfover) patch_sfo(buf,len,param);
	return result;
}

/////////////////////////////////////////////////////////////////////////////
// FW2.00/2.50 UMD HOOK
/////////////////_////////////////////////////////////////////////////////////
static void patch_any_umd(u32 *umd_read,u32 *stub_entry)
{
	// Real UMD and no SFO patch
	if(is_real_umd() && (sfover==0) ) return;

	// generic read entry replace jal
	umd_read_point = umd_read;
	umd_read_value = *umd_read;
	org_hook_umdread = (void *)((((*umd_read)&0x03ffffff)<<2)|0x80000000);
	*umd_read = MIPS_JAL(hook_umdread);

	// finish if RealUMD
	if(is_real_umd()) return;

	// force DiskInfo to GAME
	hookAPIDirect((void *)sceUmdManGetDiscInfo,(void *)hook_sceUmdManGetDiscInfo,&res_sceUmdManGetDiscInfo);

	// overwrite MAX LBA,etc.
	stub_entry_point = stub_entry;
	stub_entry_value = *stub_entry;
	*stub_entry = MIPS_J(hook_sceUmdMan_driver_Unkonow_e192c10a);

	// GPIO read to UMD eject emulation
	hookAPIDirect(sceGpioPortRead,hook_sceGpioPortRead,&res_sceGpioPortRead);
}

/////////////////////////////////////////////////////////////////////////////
// UMD patch database
/////////////////////////////////////////////////////////////////////////////
enum {
	UMD_PP_150,
	UMD_PP_200,
	UMD_PP_250,
	UMD_PP_260,
	UMD_PP_271,
	UMD_PP_280,
	UMD_PP_301,
	UMD_PP_MAX
};

typedef struct umdPatchStruct
{
	void *check_api_stub_entry;         // lib  stub entry point of target module
	u32 check_api_offset;               // offset of target module
	u32 ModuleInfo_offset;
	u32 ModuleVer;
//
	u32 sceUmdMan_driver_Unkonow_e192c10a_offset;
	u32 umdReadCall_offset;
	const char *fw_name;
}UMD_PP;

static UMD_PP umd_patch_db[UMD_PP_MAX] = {
// 1.50
{
	(void *)sceUmd9660GetDrive, // check_api_stub_entry
	0x000025D4, // check_api_offset
	0x00005D58, // ModuleInfo_offset
	0x01021007, // ModuleVer
	0x00005C00, // sceUmdMan_driver_Unkonow_e192c10a_offset
	0x000028F8, // umdReadCall_offset;
	"1.50"
},
// 2.00
{
	(void *)sceUmd9660GetDrive, // check_api_stub_entry
	0x00001858, // check_api_offset
	0x000051C0, // ModuleInfo_offset
	0x01031007, // ModuleVer
	0x00005038, // sceUmdMan_driver_Unkonow_e192c10a_offset
	0x00003760, // umdReadCall_offset;
	"2.00"
},
// 2.50
{
	(void *)sceUmd9660GetDrive, // check_api_stub_entry
	0x000019a4, // check_api_offset
	0x00005310, // ModuleInfo_offset
	0x01031007, // ModuleVer
	0x0000518C, // sceUmdMan_driver_Unkonow_e192c10a_offset
	0x000038A8, // umdReadCall_offset;
	"2.50"
},
// 2.60
{
	(void *)sceUmd9660GetDrive, // check_api_stub_entry
	0x00001934, // check_api_offset
	0x00005500, // ModuleInfo_offset
	0x01031007, // ModuleVer
	0x000053C0, // sceUmdMan_driver_Unkonow_e192c10a_offset
	0x000038E4, // umdReadCall_offset;
	"2.60"
},

// 2.71
{
	(void *)sceUmd9660GetDrive, // check_api_stub_entry
	0x0000187c, // check_api_offset
	0x000056BC, // ModuleInfo_offset
	0x01051007, // ModuleVer
	0x00005584, // sceUmdMan_driver_Unkonow_e192c10a_offset
	0x00003BC0, // umdReadCall_offset;
	"2.71"
},
// 2.80
{
	(void *)sceUmd9660Init, // check_api_stub_entry
	0x0000151c, // check_api_offset
	0x000057BC, // ModuleInfo_offset
	0x01051007, // ModuleVer
	0x0000568C, // sceUmdMan_driver_Unkonow_e192c10a_offset
	0x00003D0C, // umdReadCall_offset;
	"2.8x"
},
// 3.01
{
	(void *)sceUmd9660Init, // check_api_stub_entry
	0x000017E4, // check_api_offset
	0x00005C2C, // ModuleInfo_offset
	0x01051007, // ModuleVer
	0x00005AF8, // sceUmdMan_driver_Unkonow_e192c10a_offset
	0x00003FD4, // umdReadCall_offset;
	"3.0x"
},
};

/////////////////////////////////////////////////////////////////////////////
// UMD9660のバージョンチェック
/////////////////////////////////////////////////////////////////////////////
static u32 *get_umd9660_seg(void *api_stub,u32 sceUmd9660GetDrive_o,u32 modinfo_o,u32 mod_ver)
{
	u32 *umd9660_seg;

	umd9660_seg = getModuleSeg(api_stub,sceUmd9660GetDrive_o);
	if(umd9660_seg)
	{
		if(
			(umd9660_seg[modinfo_o/4]  == mod_ver) &&
			(umd9660_seg[(modinfo_o+4)/4] == 0x55656373) // "sceU..."
		)
		{
			return umd9660_seg;
		}
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// HOOK UMD9660 driver
/////////////////////////////////////////////////////////////////////////////
int hook_umd9660_driver(void)
{
	u32 *umd9660_seg;
	int fw_no;
	UMD_PP *p;

	// release if already hooked
	release_umd9660_driver();

	// get config value
	dhGetRegistry("SFO_VER",&sfover,sizeof(sfover));

//	dhGetRegistry("ANYUMD",&anyumd,sizeof(anyumd));
//	dhGetRegistry("NOUMD",&noumd,sizeof(noumd));

	// search FW version
	for(fw_no=0;fw_no<UMD_PP_MAX;fw_no++)
	{
		p = &umd_patch_db[fw_no];
		umd9660_seg = get_umd9660_seg(
				p->check_api_stub_entry,
				p->check_api_offset,
				p->ModuleInfo_offset,
				p->ModuleVer);
		if(umd9660_seg)
		{
			// found , patch ANYUMD
			u32 *stub_sceUmdMan_driver_Unkonow_e192c10a = &umd9660_seg[p->sceUmdMan_driver_Unkonow_e192c10a_offset/4];
			u32 *umd_read = &umd9660_seg[p->umdReadCall_offset/4];
			patch_any_umd(umd_read,stub_sceUmdMan_driver_Unkonow_e192c10a);
			Kprintf("ANYUMD %s\n",p->fw_name);
			return 0;
		}
	}
	Kprintf("umd9660 no hook\n");
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// RELEASE HOOKED UMD9660 driver
/////////////////////////////////////////////////////////////////////////////
int release_umd9660_driver(void)
{
	if(umd_read_point)
	{
		*umd_read_point = umd_read_value;
		umd_read_point = NULL;
	}

	if(stub_entry_point)
	{
		restoreAPIDirect(&res_sceUmdManGetDiscInfo);
		restoreAPIDirect(&res_sceGpioPortRead);
		*stub_entry_point = stub_entry_value;
		stub_entry_point  = NULL;
	}
	return 0;
}
