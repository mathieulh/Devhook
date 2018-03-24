/*
	PSP VSH extender for devhook 0.50+
*/
#include "common.h"

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
static SceIoDirent g_dir;

char path_buf[256];
char umd_path[128];

/////////////////////////////////////////////////////////////////////////////
// path handling
/////////////////////////////////////////////////////////////////////////////
char *make_path(char *buf,const char *boot_path,const char *target_name)
{
	int pos,dpos;
	char ch;

	//clear first
	for(pos=0;pos<128;pos++) path_buf[pos]=0;

	pos = dpos = 0;
	do
	{
		ch = *boot_path++;
		buf[pos++] = ch;
		if(ch=='/') dpos = pos;
	}while(ch!=0);

	if(target_name==0)
	{
		buf[dpos++] = 0;
		return buf;
	}

	do
	{
		ch = *target_name++;
		buf[dpos++] = ch;
	}while(ch!=0);
	return buf;
}

/////////////////////////////////////////////////////////////////////////////
// UMD image AUTO SELECT
/////////////////////////////////////////////////////////////////////////////
void umd_auto_mount(int diff)
{
	int sel;
	int fd;
	int umd_sel;

Kprintf("UMD auto mount:");

	regGetUmdPath(umd_path,sizeof(umd_path));
	if(umd_path[0]==0x00)
	{
		// real UMD to file

Kprintf("from real UMD:");

		strcpy(umd_path,"ms0:/ISO/DUMMY"); // default directry
		umd_sel = 0; // top file in directry
		sel = 1;
	}
	else
	{
		umd_sel = -1; // UMD disc when not found
		fd = sceIoDopen(make_path(path_buf,umd_path,NULL)); // mount mode
		if(fd >0)
		{
			sel = 0;
			while(sceIoDread(fd, &g_dir) > 0)
			{
				if((g_dir.d_stat.st_attr & FIO_SO_IFDIR) == 0)
				{
					if(strcmp(umd_path,make_path(path_buf,umd_path,g_dir.d_name))==0)
						umd_sel = sel + diff; // found current , set next
					sel++;
				}
			}
			sceIoDclose(fd);
		}
	}

	// roll over select
//		if(umd_sel < 0)        umd_sel = sel-1;
//		else if(umd_sel >=sel) umd_sel = 0;

	if( (umd_sel >= 0) && (umd_sel <sel) )
	{
		// search Mount file

		// open again , get next target name
		fd = sceIoDopen(make_path(path_buf,umd_path,NULL));
		if(fd <0) return;

		sel = 0;
		while(sceIoDread(fd, &g_dir) > 0)
		{
			if((g_dir.d_stat.st_attr & FIO_SO_IFDIR) == 0)
			{
				//if(g_dir.d_name)
				{
					if(umd_sel == sel) break;
					sel++;
				}
			}
		}
		sceIoDclose(fd);

		// make full path
		make_path(path_buf,umd_path,g_dir.d_name);
	}
	else
	{
		// real UMD
		path_buf[0] = 0x00;
	}
Kprintf("UMD MOUNT '%s'\n",path_buf);

	// mount new UMD image file
	if( dhUMDMount(path_buf) < 0)
		return; // error

	// save current config file for launcher
	// devhook_save_config(devhook_cfg_path);
}

#if SUPPORT_SFO_VER
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void change_sfo(int direction)
{
	regSetSfoVer(regGetSfoVer()^1) ;
}
#endif

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void umd_disable_autoboot(void)
{
	// get umd path
	regGetUmdPath(umd_path,sizeof(umd_path));

	// mound ignore image to EJECT emulation
	dhUMDMount("$$DUMMY$$");

	// wait until UMD check from VSH
	sceKernelDelayThread(1000000);

	// remound umd
	dhUMDMount(umd_path);
}
