/*
	PSP VSH extender MENU controll
*/
#include "common.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#define DISP_YESNO_Y   200
#define DISP_UMD_Y     264
#define DISP_SHOW_TIME 3*60

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

//
void dump_memregion(const char* file, void *addr, int len);

//
void change_clock(int dir);
//

/////////////////////////////////////////////////////////////////////////////
// CTRL button code for kernel mode
/////////////////////////////////////////////////////////////////////////////
#define PSP_CTRL_WLAN   0x00040000
#define PSP_CTRL_REMOTE 0x00080000
#define PSP_CTRL_UMDIN  0x01000000

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
extern char path_buf[256];
extern char umd_path[256];

/////////////////////////////////////////////////////////////////////////////
// draw strings
/////////////////////////////////////////////////////////////////////////////
static char freq_buf[3+3+2] = "Default";

#if SUPPORT_PRELOAD
static char preload_str_buf[8+1];
#endif

static const char str_default[] = "Default";

static const char *str_on_off[2] = {"OFF","ON "};

enum{
	TMENU_UMD,
	TMENU_CLK,
TMENU_NULL1,
#if SUPPORT_SFO_VER
	TMENU_SFO,
#endif
	TMENU_AUTOMENU,
#if SUPPORT_PRELOAD
	TMENU_PRELOAD,
#endif
#if SUPPORT_FW_SEL
	TMENU_FW,
#endif
#if SUPPORT_REMOTE
	TMENU_REMOTE,
#endif

#if DUMP_KMEM
TMENU_NULLD,
	TMENU_KDUMP,
#endif
TMENU_NULL4,
#if SUPPORT_NORMALFW
	TMENU_NORMALFW,
#endif
#if SUPPORT_REBOOT
	TMENU_REBOOT,
#endif
	TMENU_RETURN,
//
	TMENU_MAX
};

static const char *top_menu_list[TMENU_MAX] ={
"UMD IMAGE   ",
"CPU CLOCK   ",
NULL,
#if SUPPORT_SFO_VER
"UMD SFO VER ",
#endif
"MENU on BOOT",
#if SUPPORT_PRELOAD
"PRELOAD ADDR",
#endif
#if SUPPORT_FW_SEL
"FIRMWARE",
#endif
#if SUPPORT_REMOTE
"REMOTE PORT",
#endif
NULL,
#if DUMP_KMEM
NULL,
"DUMP_KMEM ",
#endif
#if SUPPORT_NORMALFW
"ESCAPE the DEVHOOK",
#endif
#if SUPPORT_REBOOT
"REBOOT with DEVHOOK",
#endif
"CLOSE MENU",
};

enum {
	SURE_IDLE,
	SURE_YES,
	SURE_NO
};

static int item_fcolor[TMENU_MAX];
static const char *item_str[TMENU_MAX];
static int yes_no_sel = SURE_IDLE;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int menu_sel = TMENU_RETURN;

/////////////////////////////////////////////////////////////////////////////
// open menu 
/////////////////////////////////////////////////////////////////////////////
int menu_open(void)
{
	// create framebuffer

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// clsoe menu 
/////////////////////////////////////////////////////////////////////////////
int menu_close(void)
{
	if(yes_no_sel!=SURE_IDLE) return 0;

	// purge create framebuffer
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// draw menu 
/////////////////////////////////////////////////////////////////////////////
int menu_draw(void)
{
	u32 fc,bc;
	const char *msg;
	int menu_len;
	int max_menu;
	
//Kprintf("DRAW\n");
	// check & setup video mode
	if( blit_setup() < 0) return -1;

	// show menu list
	blit_set_color(0xffffff,0x8000ff00);

//	blit_string_ctr(4*8,"vshex DEVHOOK MENU"); // noise on right side
	blit_string(10*8,4*8,"vshex DEVHOOK SETTING MENU");

	// menu title & list
	menu_len  = 12; //get_max_len(top_menu_list,TMENU_MAX);
	for(max_menu=0;max_menu<TMENU_MAX;max_menu++)
	{
		fc = 0xffffff;
		bc = (max_menu==menu_sel) ? 0xff8080 : 0xc00000ff;
		blit_set_color(fc,bc);

		msg = top_menu_list[max_menu];
		if(msg)
		{
			blit_string(10*8,(6+max_menu)*8,msg);

			msg = item_str[max_menu];
			if(msg)
			{
				blit_set_color(item_fcolor[max_menu],bc);
				blit_string((10+menu_len+1)*8,(6+max_menu)*8,msg);
			}
		}
	}

	// are you sure mode
	if(yes_no_sel)
	{
		fc = RGB(0xff,0xff,0x00);
		blit_set_color(fc,RGBT(0xff,0x00,0x00,0x40));
		blit_string(20*8,DISP_YESNO_Y,"Are you sure ?");

		bc = yes_no_sel==SURE_YES ? RGBT(0x00,0x00,0x00,0x00) : RGBT(0x00,0x00,0x00,0xe0);
		blit_set_color(fc,bc);
		blit_string(24*8,DISP_YESNO_Y+8,"Yes");
		bc = yes_no_sel==SURE_NO  ? RGBT(0x00,0x00,0x00,0x00) : RGBT(0x00,0x00,0x00,0xe0);
		blit_set_color(fc,bc);
		blit_string(28*8,DISP_YESNO_Y+8,"NO");
	}

	// UMD IMAGE NAME
	blit_set_color(0x00ffffff,0x80ff8080);
	blit_string(0,DISP_UMD_Y,path_buf);

	blit_set_color(0x00ffffff,0x00000000);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// setup to menu draw
/////////////////////////////////////////////////////////////////////////////
int menu_setup(void)
{
	u32 cpu_clk,bus_clk;
#if SUPPORT_SFO_VER
	u32 sfo_ver;
#endif
	int i;

	// preset
	for(i=0;i<TMENU_MAX;i++)
	{
		item_str[i] = NULL;
		item_fcolor[i] = RGB(255,255,255);
	}

	// UMD mode / path
	regGetUmdPath(umd_path,256);
	if(umd_path[0]==0x00)
	{
		strcpy(path_buf,"UMD:UMD Disc");
		item_str[TMENU_UMD] = "UMD Disc";
	}
	else
	{
		sprintf(path_buf,"UMD:%s",umd_path);
		path_buf[59]=0;
		item_str[TMENU_UMD] = "Emulation";
	}

	// CLOCK
	regGetClock(&cpu_clk,&bus_clk);
	sprintf(freq_buf,"%3d/%3d",cpu_clk,bus_clk);
	if(cpu_clk==0) item_str[TMENU_CLK] = str_default;
	else           item_str[TMENU_CLK] = freq_buf;

#if SUPPORT_SFO_VER
	// SFO
	sfo_ver = regGetSfoVer();
	item_str[TMENU_SFO] = sfo_ver ? "force 2.00" : str_default;
#endif

	// AUTO MENU
	item_str[TMENU_AUTOMENU] = str_on_off[regGetAutoMenu()];

#if SUPPORT_PRELOAD
	// PRELOAD
	sprintf(preload_str_buf,"%08X",regGetPreloadAddr());
	item_str[TMENU_PRELOAD]  = preload_str_buf;
	// item_fcolor[TMENU_PRELOAD] = RGB(255,255,0);
#endif

#if SUPPORT_FW_SEL
	item_str[TMENU_FW]  = "not supported";
	item_fcolor[TMENU_FW] = RGB(255,255,0);
#endif
#if SUPPORT_REMOTE
	item_str[TMENU_REMOTE]  = "not supported";
	item_fcolor[TMENU_REMOTE] = RGB(255,255,0);
#endif

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int are_you_sure_yes(void)
{
	switch(menu_sel)
	{
#if SUPPORT_NORMALFW
	case TMENU_NORMALFW:
		dhCLKSet(222, 111);
		sceKernelDelayThread(1100000);
		scePower_driver_0442D852();
		while(1) sceKernelDelayThread(1000000);
		return 1;
#endif
#if SUPPORT_REBOOT
	case TMENU_REBOOT:
		break;
#endif
	}
	return 1; // continue
}

static void trap_1AEC(void)
{
	Kprintf("Enter 1AEC\n");
	while(1);
}

int sceKernelExitVSHVSH(void);

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int menu_ctrl(u32 buttons,u32 button_on)
{
	int direction;
	int val32;
	//
	// check buttons 
	//
	button_on &=
		PSP_CTRL_SELECT|PSP_CTRL_START|
		PSP_CTRL_UP|PSP_CTRL_RIGHT|PSP_CTRL_DOWN|PSP_CTRL_LEFT|
		PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|
		PSP_CTRL_TRIANGLE|PSP_CTRL_CIRCLE|PSP_CTRL_CROSS|PSP_CTRL_SQUARE|
		PSP_CTRL_HOME|PSP_CTRL_NOTE; // PSP_CTRL_HOLD

	// are you sure check
	if(yes_no_sel)
	{
		int result = 1;
		if(button_on==PSP_CTRL_LEFT)  yes_no_sel=SURE_YES;
		if(button_on==PSP_CTRL_RIGHT) yes_no_sel=SURE_NO;
		if(button_on==PSP_CTRL_CIRCLE)
		{
			if(yes_no_sel==SURE_YES)
				result = are_you_sure_yes();
			yes_no_sel = SURE_IDLE;
		}
		return result;
	}

	// change menu select
	direction = 0;
	if(button_on&PSP_CTRL_DOWN) direction++;
	if(button_on&PSP_CTRL_UP) direction--;
	do
	{
		menu_sel = limit(menu_sel+direction,0,TMENU_MAX-1);
	}while(top_menu_list[menu_sel]==NULL);

	// LEFT & RIGHT
	direction = -2;
	if(button_on&PSP_CTRL_LEFT)   direction = -1;
	if(button_on&PSP_CTRL_CIRCLE) direction = 0;
	if(button_on&PSP_CTRL_RIGHT)  direction = 1;
	if(direction>-2)
	{
		switch(menu_sel)
		{
		case TMENU_UMD:
			if(direction) umd_auto_mount(direction);
			break;
		case TMENU_CLK:
			if(direction) change_clock(direction);
			break;
		case TMENU_RETURN:
			if(direction==0) return 0; // finish
			break;
#if SUPPORT_SFO_VER
		case TMENU_SFO:
			change_sfo(direction);
			break;
#endif
		case TMENU_AUTOMENU:
			regSetAutoMenu(regGetAutoMenu()^1);
			break;
#if SUPPORT_PRELOAD
		case TMENU_PRELOAD:
			val32 = regGetPreloadAddr();
			val32 += direction * 0x00200000;
			val32 = limit(val32&0x7fffffff,0x088e00000,0x09800000) | 0x80000000;
			regSetPreloadAddr(val32);
			break;
#endif
#if SUPPORT_FW_SEL
		case TMENU_FW:
			break;
#endif
#if SUPPORT_REMOTE
		case TMENU_REMOTE:
		break;
#endif
#if SUPPORT_NORMALFW
		case TMENU_NORMALFW:
			if(direction==0) yes_no_sel = SURE_NO;
			break;
#endif
#if SUPPORT_REBOOT
		case TMENU_REBOOT:
			if(direction==0)
			{
				pspSdkSetK1(0x00100000);
				// sceKernelGetUserLevel must be 4
				Kprintf("K1 %08X\n",pspSdkGetK1());
				Kprintf("sceKernelGetUserLevel %08X\n",sceKernelGetUserLevel());
{
	u32 *lp;

	lp = (u32 *)(0x80000000 + (((*(u32 *)sceKernelGetUserLevel)&0x03ffffff)<<2) );
	lp[0] = MIPS_JR(31);
	lp[1] = MIPS_ADDI(2,0,4);

//	lp = (u32 *)sceKernelIsIntrContext;
//	lp[0] = MIPS_JR(31);
//	lp[1] = MIPS_ADDI(2,0,1);

#if 0
	lp = (u32 *)(0x80000000 + (((*(u32 *)sceKernelExitVSHVSH)&0x03ffffff)<<2) );
	lp -= 0x000015AC/4;
	lp[0x1aec/4] = MIPS_J(trap_1AEC);
	lp[0x1af0/4] = MIPS_NOP;

	lp[0x1608/4] = MIPS_J(trap_1AEC);
	lp[0x160c/4] = MIPS_NOP;

	lp[0x15ec/4] = MIPS_J(trap_1AEC);
	lp[0x15f0/4] = MIPS_NOP;
#endif
}
  sceKernelDcacheWritebackAll();
  sceKernelIcacheClearAll();
				// who is lock there ?
//				Kprintf("sceKernelExitGame %08X\n",sceKernelExitGame());
				Kprintf("sceKernelExitVSHVSH %08X\n",sceKernelExitVSHVSH());
//				Kprintf("sceKernelLoadExec %08X\n",sceKernelLoadExec("",NULL));
			}
			break;
#endif
#if DUMP_KMEM
		case TMENU_KDUMP:
			dump_memregion("ms0:/kmem.bin",(void *)0x88000000,0x00400000);
			break;
#endif
		}
	}

	return 1; // continue
}

