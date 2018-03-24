/*
	PSP VSH extender for devhook 0.50+

	short cut key in XMB

	HOME+RTRG   : mount next UMD file in same directry
	HOME+LTRG   : mount prev UMD file in same directry
	HOME+SELECT : show info / change CLOCK

	image path 
to do:
	change VSH(FW) version code
	reboot to return no-devhook Firmware

*/
#include "common.h"

//
int menu_open(void);
int menu_close(void);
int menu_setup(void);
int menu_draw(void);
int menu_ctrl(u32 buttons,u32 buttons_on);

//
void umd_auto_mount(int diff);

//
void textblit_set_vram(void *memory,int width,int height);

//
void change_clock(int dir);

/////////////////////////////////////////////////////////////////////////////
// CTRL button code for kernel mode
/////////////////////////////////////////////////////////////////////////////
#define PSP_CTRL_WLAN   0x00040000
#define PSP_CTRL_REMOTE 0x00080000
#define PSP_CTRL_UMDIN  0x01000000

/////////////////////////////////////////////////////////////////////////////
// module info
/////////////////////////////////////////////////////////////////////////////
PSP_MODULE_INFO("vshExtenderForDevhook", 0x1007, 1, 1);
PSP_MAIN_THREAD_ATTR(0);

/////////////////////////////////////////////////////////////////////////////
// CTRL API to hold VSH in DH MENU
/////////////////////////////////////////////////////////////////////////////
static int thid;

static int menu_mode  = 0;
static u32 cur_buttons = 0;
static u32 button_on  = 0;
static u32 short_cut_key = 0;
static SceCtrlData ctrl_pad;

// original call entry
int org_sceCtrlReadBufferPositive(SceCtrlData *pad_data, int count)
{
	HOOK_FUNC_SPACE();
}

/////////////////////////////////////////////////////////////////////////////
int hook_sceCtrlReadBufferPositive(SceCtrlData *pad_data, int count)
{
	int result;
	u32 buttons;
	int eat_key;

	// get pad data
	result = org_sceCtrlReadBufferPositive(pad_data,count);

	// copy true value
	memcpy(&ctrl_pad,pad_data,sizeof(ctrl_pad));

	// buttons check
	buttons     = ctrl_pad.Buttons;
	button_on   = ~cur_buttons & buttons;
	cur_buttons = buttons;

	// short cut key check
	eat_key = 0;

#if 1
	if( (buttons&ALL_CTRL)&PSP_CTRL_HOME )
	{
		eat_key = 1;
		if(button_on==PSP_CTRL_HOME)
			short_cut_key = PSP_CTRL_SELECT;
	}
#else
	if( (buttons&ALL_CTRL&~button_on)== PSP_CTRL_HOME) // HOME + BUTTON
	{
		eat_key = 1;
		short_cut_key = (button_on & ALL_CTRL);
	}
#endif

	// menu mode
	if(menu_mode!=0) eat_key = 1;

	// mask buttons for LOCK VSH controll
	if(eat_key)
	{
		pad_data->Buttons &= ~(
		PSP_CTRL_SELECT|PSP_CTRL_START|
		PSP_CTRL_UP|PSP_CTRL_RIGHT|PSP_CTRL_DOWN|PSP_CTRL_LEFT|
		PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|
		PSP_CTRL_TRIANGLE|PSP_CTRL_CIRCLE|PSP_CTRL_CROSS|PSP_CTRL_SQUARE|
		PSP_CTRL_HOME|PSP_CTRL_NOTE);
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////
// menu mode controll
/////////////////////////////////////////////////////////////////////////////
static int open_menu(void)
{
	if( menu_open() == 0) return -1;
	menu_setup();
	// when set priority level <= 5 then cause audio noise.
/*	sceKernelChangeThreadPriority(0,6); */
	sceKernelChangeThreadPriority(0,8);
	menu_mode = 1;
	return 0;
}

static int close_menu(void)
{
	if(menu_close()==0) return 0;
	sceKernelChangeThreadPriority(0,47);
	menu_mode = 2;
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// check VSH loaded
/////////////////////////////////////////////////////////////////////////////
extern void patch_vshmain(void);

static int valid_vsh(void)
{
	SceModule *module = sceKernelFindModuleByName("vsh_module");

//	if(module) patch_vshmain();

	return (int)module;
}

/////////////////////////////////////////////////////////////////////////////
// short cut key
/////////////////////////////////////////////////////////////////////////////
static void button_func(void)
{
	if(short_cut_key)
	{
		switch(short_cut_key & ALL_CTRL)
		{
#if 0
		case PSP_CTRL_START:
Kprintf("reboot\n");
//			sceKernelLoadExec("disc0:/PSP_GAME/SYSDIR/EBOOT.PBP",0);
//			sceKernelLoadExec("ms0:/PSP/GAME/DEVHOOK/EBOOT.PBP",0);
//			sceKernelExitVSHVSH();
			break;
#endif
		case PSP_CTRL_RTRIGGER:
			umd_auto_mount(+1);
			break;
		case PSP_CTRL_LTRIGGER:
			umd_auto_mount(-1);
			break;
		case PSP_CTRL_SELECT:
			if(menu_mode==0)      open_menu();
			else if(menu_mode==1) close_menu();
			break;
		}
		short_cut_key = 0;
	}

	// menu controll
	switch(menu_mode)
	{
	//case 0:
	case 1:
		if( menu_ctrl(cur_buttons,button_on)==0) close_menu();
		break;
	case 2: // exit waiting 
		// exit menu
		if( (cur_buttons & ALL_CTRL) == 0)
		{
			menu_mode = 0;
		}
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// TSR thread
/////////////////////////////////////////////////////////////////////////////
int TSRThread(SceSize args, void *argp)
{
	// hook controll API
	hookAPIDirect(sceCtrlReadBufferPositive,hook_sceCtrlReadBufferPositive,org_sceCtrlReadBufferPositive);

	//int cur_prio = sceKernelGetThreadCurrentPriority();
	sceKernelChangeThreadPriority(0,47);

	while(!valid_vsh())
	{
		//Kprintf("waiting for load VSH\n");
		sceKernelDelayThread(100000);
	}
	Kprintf("vsh_main found\n");

	// umd autorun disable
	if(regGetUmdDelaymount())
	{
		umd_disable_autoboot();
		// clear registry
	}
	regSetUmdDelaymount(0);

	// auto menu check
	if(regGetAutoMenu())
	{
		open_menu();
	}

	while(1)
	{
		if( sceDisplayWaitVblankStart() < 0)
			break; // end of VSH ?

		// stop request
		if(thid==0) break;

		// menu draw
		if(menu_mode==1)
		{
			// draw first!
			menu_draw();
			// setup next one
			menu_setup();
		}
		// button func
		button_func();

	}
	if(menu_mode) close_menu();

	Kprintf("vshex term\n");
	return 1;
}

/*****************************************************************************
	entry point
*****************************************************************************/
int module_start(SceSize args, void *argp)
{
	Kprintf("----- enter %s -----\n",module_info.modname);
	thid = sceKernelCreateThread("DEVHOOK_THREAD", TSRThread, 1 , 0x1000, 0, 0);
	if(thid >= 0)
		sceKernelStartThread(thid, 0, 0);
	return 0;
}

int module_stop(SceSize args, void *argp)
{
	SceUInt timeout = 500000;
	SceUID id = thid;

	thid = 0;
	if(sceKernelWaitThreadEnd(id,&timeout) < 0)
	{
		sceKernelTerminateDeleteThread(id);
	}

	// restore API hook
	restoreAPIDirect((HOOKAPI_resotre *)org_sceCtrlReadBufferPositive);

	return 0;
}
