/*
	script command for devhook 
*/
#include "common.h"

// Do not supported this version
//#define  SUPPORT_INSTALL_STAND_ALONE

/*******************************************************************************
	refresh display
*********************************************************************************/
extern int scePowerGetBatteryVolt(void);

static void refresh_display(void)
{
	char buf1[128],buf2[128],buf3[128];
	u32 val1;
	int x,y;
	int volt;
	u32 devkit;
	int mjr,mnr,rev;

	// title message
	text_clear();
	GetRegistry("STR_TITLE",buf1,sizeof(buf1));
	text_printfXY(TEXT_CENTER,0,buf1);

	// show settings
	y = CONSOLE_HEIGHT - 1;

	// useage
	GetRegistry("STR_STS_KEYS",buf1,sizeof(buf1));
	text_printfXY(TEXT_CENTER,y--,buf1);

	// left side
	GetRegistry("STR_STS_PREADDR",buf1,sizeof(buf1));
	GetRegistry("PRELOAD_ADDR",buf2,sizeof(buf2));
	text_printfXY(0,y--,"%s:%s\n",buf1,buf2);

	GetRegistry("STR_STS_REMOTE",buf1,sizeof(buf1));
	val1 = 0;
	GetRegistryDWORD("KPRINT_UART",&val1);
	GetRegistry(val1 ? "STR_RC_KPRINT" : "STR_RC_NOKPRINT",buf2,sizeof(buf2));
	text_printfXY(0,y--,"%s:%s",buf1,buf2);

	GetRegistry("STR_STS_CLOCK",buf1,sizeof(buf1));
	GetRegistry("CLOCK",buf2,sizeof(buf2));
	text_printfXY(0,y--,"%s:%s MHz",buf1,buf2);

	GetRegistry("STR_STS_FLASH",buf1,sizeof(buf1));
	GetRegistry("FLASH0",buf2,sizeof(buf2));
	GetRegistry("FLASH1",buf3,sizeof(buf3));
	text_printfXY(0,y--,"%s:%s , %s",buf1,buf2,buf3);

	GetRegistry("STR_STS_REBOOT",buf1,sizeof(buf1));
	GetRegistry("REBOOT_PATH",buf2,sizeof(buf2));
	text_printfXY(0,y--,"%s:%s",buf1,buf2);

	GetRegistry("STR_STS_BTCONF",buf1,sizeof(buf1));
	GetRegistry("BTCNF_PATH",buf2,sizeof(buf2));
	text_printfXY(0,y--,"%s:%s",buf1,buf2);

	GetRegistry("STR_STS_UMDMODE",buf1,sizeof(buf1));
	val1 = 0;
	GetRegistryDWORD("SFO_VER",&val1);
	GetRegistry(val1 ? "STR_VM_2XX200" : "STR_VM_NOCHANGE",buf2,sizeof(buf2));
	text_printfXY(0,y--,"%s:SFO Ver.%s",buf1,buf2);
	y--;

	GetRegistry("STR_STS_UMDISO",buf1,sizeof(buf1));
	GetRegistry("UMD_PATH",buf2,sizeof(buf2));
	if(buf2[0]==0x00)
		GetRegistry("STR_UMD_DISC",buf2,sizeof(buf2));
	text_printfXY(0,y--,"%s:%s",buf1,buf2);


	// right side status
	x = CONSOLE_WIDTH-24;
	y = CONSOLE_HEIGHT - 2;

	// battery voltage
	volt = scePowerGetBatteryVolt();
	GetRegistry("STR_STS_BATTERY",buf1,sizeof(buf1));
	switch(volt)
	{
	case 0x802b0100:
		GetRegistry("STR_BAT_NOCONNECT",buf2,sizeof(buf2));
		break;
	default:
		GetRegistry("STR_BAT_UNKLNOWN",buf2,sizeof(buf3));
		if(volt>0) sprintf(buf2,"%d.%03dV",volt/1000,volt%1000);
		else       sprintf(buf2,"%s %08X",buf3,volt);
	}
	text_printfXY(x,y--,"%s:%s",buf1,buf2);

	// devkit version
	GetRegistry("STR_STS_FWVER",buf1,sizeof(buf1));
	devkit = sceKernelDevkitVersion();
	mjr = devkit>>24;
	mnr = (devkit>>16)&0xff;
	rev = devkit &0xffff;
	text_printfXY(x,y--,"%s:%X.%02X.%04X",buf1,mjr,mnr,rev);

	//sceDisplayWaitVblankStart();
}

/*******************************************************************************
	refresh display

refresh(NULL)
*********************************************************************************/
static char *cmd_refresh(int argc,char **argv)
{
	refresh_display();
	return "";
}

/*******************************************************************************
file sesect command

filesel(sx,sy,title,dir_base1,dir_base2,...)
*********************************************************************************/
#define MAX_FILESEL 24
static char fsel_buf[256*MAX_FILESEL];
static char *file_list[MAX_FILESEL];
static char *full_list[MAX_FILESEL];

static char *cmd_filesel(int argc,char **argv)
{
	int sx,sy;
	char *title;
	int i;
	char *ptr;
	int sel;
	
	if(argc<5) return "";

	sx = str2val(argv[1]);
	sy = str2val(argv[2]);
	title = argv[3];

	// refresh display
	cmd_refresh(0,NULL);

	// add directry list
	ptr = fsel_buf;
	file_list[0]=full_list[0]=0;
	for(i=4;i<argc;i++)
		ptr = add_file_list(argv[i] ,file_list,full_list,ptr,NULL);

	// no file ?
	if(file_list[0] == 0)
	{
		text_printfXY(sx,sy,"%s",title);
		text_printfXY(sx,sy+1,"NO FILE");
		wait_key();
		return NULL;
	}

	// choise one
	sel = selitem(sx,sy,title,(const char **)file_list,0);
	if(sel>=0)
	{
		return full_list[sel]; // selected file path
	}

	return NULL; // cancel
}

/*******************************************************************************
	check UMD image file
*********************************************************************************/
static void check_umd_image(void)
{
	char umd_path[256] = {0};

	GetRegistry("UMD_PATH",umd_path,sizeof(umd_path));

	if(umd_path[0])
	{
		int fd = sceIoOpen(umd_path, PSP_O_RDONLY,0777);
		if(fd<0)
		{
			// UMD image not found , clear image file
			umd_path[0] = 0x00;
		}
		else
		{
			// UMD image found
			sceIoClose(fd);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// chdir
/////////////////////////////////////////////////////////////////////////////
static char *cmd_chdir(int argc,char **argv)
{
	if(argc<2) return "";
	sceIoChdir(argv[1]);
	return "";
}

/////////////////////////////////////////////////////////////////////////////
// font command
/////////////////////////////////////////////////////////////////////////////
static char *cmd_font(int argc,char **argv)
{
	if(argc<4) return "";

	text_clear();
	text_printf("loading font file\n");

	// terminal
	text_term();

	// initialize launguage registry
	InitRegistryLang();

	if( text_init(argv[1],argv[2]) == 0 )
	{
		script_execFile(argv[3]);
	}

	cmd_refresh(0,NULL);
	return "";
}


/*******************************************************************************
	build menu list from file

	'!' is separater with menu item string

*********************************************************************************/
static int load_menu_item(const char *path,char **mlist,char **mdata)
{
	int num_sel;
	char *ptr;
	int found_sel_name;
	int mid;

	ptr = loadFileAlloc(path,&mid,NULL);
	if(ptr==NULL) return -1;

	// separate script & build  menu list
	num_sel = 0;
	while(*ptr)
	{
		found_sel_name = (ptr[0]=='!');
		if(found_sel_name)
		{
			ptr[0] = 0; // EOB of prev data
			ptr++;
			mlist[num_sel] = ptr; // name
		}
		// search line end
		while(((u8)*ptr)>=0x20) ptr++;
		if(found_sel_name && *ptr)
		{
			ptr[0] = 0x00; // EOB of name
			ptr++;
		}
		// next line start
		while(*ptr && ((u8)*ptr)<0x20) ptr++;

		if(found_sel_name)
		{
			mdata[num_sel] = ptr; // body top
//Kprintf("sel %s\nbody %s",mlist[num_sel],ptr);
			num_sel++;
		}
	}
	mlist[num_sel]=NULL;

	return mid;
}

/////////////////////////////////////////////////////////////////////////////
// menu command
// menu(sx,sy,title_string,menu_file)
/////////////////////////////////////////////////////////////////////////////
static char *cmd_menu(int argc,char **argv)
{
	int sx,sy;
	char *title;
	int sel;
	int mid;
	char *mlist[24],*mdata[24];

	if(argc<5) return "";

	sx = str2val(argv[1]);
	sy = str2val(argv[2]);
	title = argv[3];

	mid = load_menu_item(argv[4],mlist,mdata);
	if(mid<0) return "";

	// refresh display
	cmd_refresh(0,NULL);

	// select menu
	sel = selitem(sx,sy,title,(const char **)mlist,get_value(title));
	if(sel>=0)
	{
		set_value(title,sel);
		// exec item script
		script_execBuf(mdata[sel]);
	}
	// free memory
	sceKernelFreePartitionMemory(mid);

	return "";
}

/////////////////////////////////////////////////////////////////////////////
// menu command
// MENU(sx,sy,title,path)
/////////////////////////////////////////////////////////////////////////////
extern const char base_directry[];
extern const char confg_file_path[];

static char *cmd_reboot(int argc,char **argv)
{
	char msg[128];
	u32 dhver;
	u32 kprint_val = 0;
	int dh_bootmode;

	text_clear();

	text_printf("startup the devhook\n");

	// enable Kprintf REMOTE
	GetRegistryDWORD("KPRINT_UART",&kprint_val);
	if(kprint_val)
	{
		text_printf("enable KPRINTF to REMOTE port\n");
		pspDebugSioInit();
		pspDebugSioSetBaud(115200);
		pspDebugSioInstallKprintf();
		/* pspDebugInstallStdoutHandler(pspDebugSioPutData); */
	}

///////////////////////////////////////////////////////
//	save current setting
///////////////////////////////////////////////////////
	text_printf("save launcher setting\n");
// current directry
	sceIoChdir(base_directry);
	save_launcher_setting(confg_file_path);

///////////////////////////////////////////////////////
// load devhook
///////////////////////////////////////////////////////
	text_printf("loading devhook\n");
	dh_bootmode = dh_load();
	if( dh_bootmode < 0 )
	{
		// Can't load devhook
		text_printf("load error\n");
		sceKernelDelayThread(500000);
    	sceKernelExitGame();
	}

	dhver = dhGetVersion();
	GetRegistry("STR_STS_DHVER",msg,sizeof(msg));
	text_printf("%s:%X.%02X.%04X\n",msg,dhver>>24,(dhver>>16)&0xff,dhver&0xffff);

///////////////////////////////////////////////////////
//	setup devhook registry to reboot
///////////////////////////////////////////////////////
	dh_setup_registry();
	text_printf("reboot system\n");
	dh_reboot_system("",REBOOT_VSH);
	//sceKernelExitGame();
	//sceKernelExitVSHVSH();

	Kprintf("Done DEVHOOK LAUNCHER\n");
	return "";
}

static char *cmd_exit(int argc,char **argv)
{
	text_clear();
	text_printf("sceKernelExitGame()\n");

	text_term();
	// exit to XMB
	sceKernelExitGame();
	return "";
}

/*******************************************************************************
	AUTO RUN waiting
*********************************************************************************/
static int cmd_autorun(int argc,char **argv)
{
	char msg[128];
	int timeout;
	int autorun_time = -1;


	if( GetRegistry("AUTO_RUN_TIME",msg,sizeof(msg)) >=0)
		autorun_time = str2val(msg);

	// no autorun
	if(autorun_time<0) return 0;

	// show current setting
	cmd_refresh(0,NULL);

	// autorun waiting
	for(timeout = autorun_time*50 ; timeout >=0 ; timeout--)
	{
		GetRegistry("STR_AUTORUN_WAIT",msg,sizeof(msg));
		text_printfXY(TEXT_CENTER,6,msg,(timeout+49)/50);

		if( get_key() & PSP_CTRL_CIRCLE) return 0;
		sceKernelDelayThread(20000);
	}

	GetRegistry("STR_AUTORUN",msg,sizeof(msg));
	text_printfXY(TEXT_CENTER,6,msg);

	//
	cmd_refresh(0,NULL);

	// auto start
	cmd_reboot(0,NULL);

	return 1;
}


/*******************************************************************************
	flash install command
*********************************************************************************/
static char *cmd_install(int argc,char **argv)
{
	char *sub_cmd;

	if(argc>=2)
	{
		// flash install
		sub_cmd = argv[1];
		if(strcmp(sub_cmd,"F1_BACKUP")==0){ flash1_backup(); return ""; }
		if(strcmp(sub_cmd,"F1_INST")==0){ flash1_install(); return ""; }
		if(strcmp(sub_cmd,"F1_CLEAN")==0){ flash1_cleanup(); return ""; }
		if(strcmp(sub_cmd,"F0_FONT")==0){ flash0_font(); return ""; }
		if(strcmp(sub_cmd,"F0_KDRES")==0){ flash0_kd_resource(); return ""; }
#ifdef SUPPORT_INSTALL_STAND_ALONE
		if(strcmp(sub_cmd,"F0_ALL")==0){ flash0_all(); return ""; }
#endif
		if(strcmp(sub_cmd,"F0_CLEAN")==0){ flash0_cleanup(); return ""; }
		if(strcmp(sub_cmd,"F0_PSAR")==0){ install_psar_dumper(); return ""; }
	}
	return "";
}

/*******************************************************************************
	top menu
*********************************************************************************/
int menu_main(int dh_bootmode)
{
	char buf[256];

	// umd image check
	check_umd_image();

	// autorun check
	cmd_autorun(0,NULL);

	// show current setting
	cmd_refresh(0,NULL);

	// autoexec file
	GetRegistry("AUTOEXEC",buf,sizeof(buf));
	script_execFile(buf);

	while(1)
	{
		GetRegistry("TOP_MENU",buf,sizeof(buf));
//pspDebugScreenPrintf("menu %s\n",buf);
		script_execBuf(buf);
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// external script COMMAND handling
////////////////////////////////////////////////////////////////////////////////

const struct script_cmd command_list[] = {
// UI
	{"refresh"	,cmd_refresh },
	{"filesel"	,cmd_filesel },
	{"menu"		,cmd_menu },
// file controll
	{"chdir"	,cmd_chdir },
// reboot / exit
	{"reboot"	,cmd_reboot },
	{"exit"		,cmd_exit },
// setting
	{"font"		,cmd_font },
// install
	{"install"	,cmd_install },

	{NULL,NULL}
};
