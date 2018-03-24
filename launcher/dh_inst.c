/****************************************************************************
	PSP NAND flash FW installer for devhook 
****************************************************************************/

#include "common.h"

#define USE_MALLOC 0

/*
when "flash1:/registry/system.ireg","flash1:/registry/system.dreg" are removed and flash1 full,
FW1.50 fails in the initialization of the registry file and repeats this forever.
FW1.50 doesn't delete other flash1 file in registry initialization.
Therefore, flash1 always needs a some free area for safety.

BTW.
If the FAT area of flash1 crashed, PSP may not work too.
So I thinks that the recovery mode like CUSTOM FIRMWARE of Dark Alex is necessary for FW1.50 safety.
*/
#define FLASH1_KEEP_FREE_SIZE (1024*1024)

#define FLAG_SUBDIR 1
#define FLAG_MOVE   2

/****************************************************************************
	works
****************************************************************************/
static char src_path[128];
static char dst_path[128];

static char msg1[1024];
static char msg2[1024];
/****************************************************************************
	current firmware path
****************************************************************************/

static char flash_path[256];

// get first FLASH/MS path in multi path
static const char *get_cur_path(char *path_buf,int fs_num,int is_ms)
{
	const char *plist;
	char *ptr;

	if(is_ms)
	{
		// ms
		ptr = path_buf;
	}
	else
	{
		// flash
		sprintf(path_buf,"flash%d:",fs_num);
		ptr = path_buf+strlen(path_buf);
	}

	if(fs_num==0)
		GetRegistry("FLASH0",flash_path,sizeof(flash_path)-1);
	else
		GetRegistry("FLASH1",flash_path,sizeof(flash_path)-1);
	plist = flash_path;

	while(plist!=NULL)
	{
		plist = make_path_one(ptr,plist,NULL);
		if(strchr(ptr,':'))
		{
			// full path
			if(is_ms) return path_buf;
		}
		else
		{
			// flash path
			if(!is_ms) return path_buf;
		}
	}
	path_buf[0]=0;
	return NULL;
}

static const char *ms_cur_path(char *buf,int fs_num)
{
	return get_cur_path(buf,fs_num,1);
}

static const char *flash_cur_path(char *buf,int fs_num)
{
	return get_cur_path(buf,fs_num,0);
}

/****************************************************************************
	make directry
****************************************************************************/
static int make_directry(const char *path)
{
	char tmp[128];
	char *ptr;
	int result = 0;

	strcpy(tmp,path);
	ptr = strchr(tmp,':');
	if(ptr) ptr++;
	if(ptr[0]=='/') ptr++;

	while( (ptr = strchr(ptr,'/') ) != NULL)
	{
		*ptr = 0x00;
		printf("Mkdir %s:",tmp);
		result = sceIoMkdir(tmp,0x777);
#if 0
		if( result < 0 )
			printf("Err %08X",result);
		else
			printf("OK");
#endif
		printf("\n");
		*ptr = '/';
		ptr++;
	}
	if(path[strlen(path)-1]!='/')
	{
		// last
		result = sceIoMkdir(path,0x777);
	}

	return result;
}

/****************************************************************************
	copy/remove file one
****************************************************************************/
static unsigned char file_buf[0x4000];
static int buf_size = 0x4000;

static int copy_file_one(const char *src,const char *dst,int flags)
{
	SceUID fds;
	SceUID fdd;
	int result = 0;
	int readed;

	// copy
	if(dst)
	{
		printf("Copy %s:",src);

		fds = sceIoOpen(src, PSP_O_RDONLY, 0777);
		if(fds < 0 )
		{
			printf("open error\n");
			return -1;
		}
		fdd = sceIoOpen(dst, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
		if(fdd < 0 )
		{
			printf("create error\n");
			sceIoClose(fds);
			return -2;
		}

		do
		{
			readed = result = sceIoRead(fds, file_buf , buf_size );
			if(result < 0 )
			{
				printf("read error\n");
				break;
			}
			result = sceIoWrite(fdd, file_buf , result );
			if(result != readed)
			{
				printf("write error\n");
				result = -1;
			}
		}while(result>0);

		sceIoClose(fds);
		sceIoClose(fdd);

		if(result < 0)
			return result;
	}

	// remove source
	if(flags & FLAG_MOVE)
	{
		printf("Remove %s:",src);
		result = sceIoRemove(src);
		if(result < 0)
		{
			printf("error\n");
			return result;
		}
	}

	printf("OK\n");

	return 0;
}

/****************************************************************************
	copy/remove file one
****************************************************************************/
static int copy_file(const char *src,const char *dst)
{
	return copy_file_one(src,dst,0);
}

static int move_file(const char *src,const char *dst)
{
	return copy_file_one(src,dst,FLAG_MOVE);
}

/****************************************************************************
	copy/remove directry
****************************************************************************/
static int dir_copy_one(const char *src,const char *dst,int flags)
{
	char *path_s,*path_d;
	SceUID fd;
	SceIoDirent *dir;
	int result = 0;

	fd = sceIoDopen(src);
	if(fd < 0)
	{
		switch(fd)
		{
		case 0x80010002:
			printf("Directry %s : not found\n",src);
			break;
		default:
			printf("sceIoDopen(%s) : error %08x\n",src,fd);
		}
		return fd;
	}

	printf("---- Directry %s\n",src);
	if(dst)
	{
		printf("Mkdir %s:",dst);
		result = sceIoMkdir(dst,0x777);
		if( result < 0 )
			printf("Err %08X\n",result);
		else
			printf("OK\n");
	}

#if USE_MALLOC
	path_s = (char *)malloc(128+128+sizeof(SceIoDirent));
#else
	int memid = sceKernelAllocPartitionMemory(2,"dir heap",PSP_SMEM_Low, 128+128+sizeof(SceIoDirent), NULL);
	if(memid<0)
	{
		printf("Memory alloc error\n");
		return memid;
	}
	path_s = (char *)sceKernelGetBlockHeadAddr(memid);
#endif
	path_d = dst ? (path_s+128) : NULL;
	dir    = (SceIoDirent *)(path_s+256);

	while(sceIoDread(fd, dir) > 0)
	{
		sprintf(path_s,"%s/%s",src,dir->d_name);
		if(dst) sprintf(path_d,"%s/%s",dst,dir->d_name);

		if((dir->d_stat.st_attr & FIO_SO_IFDIR) && (flags & FLAG_SUBDIR) )
		{
			// directry
			if(dir->d_name[0]!='.')
			{
				result = dir_copy_one(path_s,path_d,flags);
			}
		}
		else
		{
			// file copy/move/delete
			result = copy_file_one(path_s,path_d,flags);
		}
	}
	sceIoDclose(fd);

	// move or remove
	if(flags & FLAG_MOVE)
	{
		printf("Rmdir %s:",src);
		result = sceIoRmdir(src);
		if(result < 0)
			printf("err %08X\n",result);
		else
			printf("OK\n");
	}

#if USE_MALLOC
		free(path_s);
#else
		sceKernelFreePartitionMemory(memid);
#endif
	return result;
}

/****************************************************************************
	move directry
****************************************************************************/
int move_directry(const char *src,const char *dst)
{
	return dir_copy_one(src,dst,FLAG_MOVE|FLAG_SUBDIR);
}

/****************************************************************************
	copy directry
****************************************************************************/
int copy_directry(const char *src,const char *dst,int copy_subdir)
{
	return dir_copy_one(src,dst,copy_subdir);
}

/****************************************************************************
	remove directry
****************************************************************************/
int remove_directry(const char *src)
{
	return dir_copy_one(src,NULL,FLAG_MOVE|FLAG_SUBDIR);
}

/****************************************************************************
	make dummy file
****************************************************************************/
int make_dummy_file(const char *path,int size)
{
	SceUID fd;
	int result;

	printf("make dummy file %s:",path);

	fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if(fd < 0 )
	{
		printf("create error %08x\n",fd);
		return fd;
	}

	memset(file_buf,0,sizeof(file_buf));
	while(size)
	{
		int req_size = size > sizeof(file_buf) ? sizeof(file_buf) : size;
		result = sceIoWrite(fd, file_buf , req_size );
		if(result != req_size)
		{
			sceIoClose(fd);
			printf("write error\n");
			return result;
		}
		size -= req_size;
	}
	sceIoClose(fd);
	printf("OK\n");
	return 0;
}

/****************************************************************************
	check directry

	non thread safe function !!
****************************************************************************/
static SceIoDirent chk_dir;

char *check_directry(const char *base_path)
{
	SceUID fd;

	fd = sceIoDopen(base_path);
	if(fd < 0 ) return NULL;

	while(sceIoDread(fd, &chk_dir) > 0)
	{
		if((chk_dir.d_stat.st_attr & FIO_SO_IFDIR) )
		{
			// directry
			if(chk_dir.d_name[0]!='.')
			{
				sceIoDclose(fd);
				return chk_dir.d_name;
			}
		}
	}
	sceIoDclose(fd);
	return NULL;
}

/****************************************************************************
	sunc NAND Flash memory
****************************************************************************/
static int flash_sync()
{
	int result;

	// sync NAND flash
	result = sceIoDevctl("flash0:",0x5802,NULL,0,NULL,0);
	if(result<0)
		printf("flash0: sync error %08X\n",result);

	result = sceIoDevctl("flash1:",0x5802,NULL,0,NULL,0);
	if(result<0)
		printf("flash1: sync error %08X\n",result);

	return result;
}

/****************************************************************************
	enable/protect  flash0 write
****************************************************************************/
static int flash0_enable_write(void)
{
	int result;

	printf("flash0 write enable\n");

	result = sceIoUnassign("flash0:");
	if( result < 0)
	{
		printf("flash0 unassign error %08X\n",result);
	}
	else
	{
		result = sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, NULL, 0);
		if( result < 0)
		{
			printf("flash0 assign error %08X\n",result);
		}
	}
	return result;
}

static int flash0_protect(void)
{
	int result;

	printf("flash0 write protect\n");

	flash_sync();

	result = sceIoUnassign("flash0:");
	if( result < 0)
	{
		printf("flash0 unassign error %08X\n",result);
	}
	else
	{
		result = sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDONLY, NULL, 0);
		if( result < 0)
		{
			printf("flash0 assign error %08X\n",result);
		}
	}
	return result;
}
/****************************************************************************
ui
****************************************************************************/
#define NOWARNING 0x1000

static int flash_done(void)
{
	GetRegistry("STR_FLASH_DONE",msg1,sizeof(msg1));
	printf(msg1);
	printf("\n");
	wait_key();
	return 0;
}

static int flash_cancel(void)
{
	// cancel
	GetRegistry("STR_FLASH_CANCEL",msg1,sizeof(msg1));
	printf(msg1);
	printf("\n");
	flash_done();
	return -1;
}

static int flash_title(const char *title_string,int fs_num)
{
	int hold_time;
	char *str;
	int flags = fs_num & 0xfffffff0;

	fs_num &= 0x0f;

	text_clear();
	GetRegistry(title_string,msg1,sizeof(msg1));
	printf(msg1,src_path,dst_path);
	printf("\n\n");

	// show installed files
	str = check_directry("flash0:/dh/");
	GetRegistry(str ? "STR_FLASH0_INST_MSG" : "STR_FLASH0_NONE_MSG",msg1,sizeof(msg1));
	text_printf(msg1,str);
	text_printf("\n");

	str = check_directry("flash1:/dh/");
	GetRegistry(str ? "STR_FLASH1_INST_MSG" : "STR_FLASH1_NONE_MSG",msg1,sizeof(msg1));
	text_printf(msg1,str);
	text_printf("\n");

	if(flags&NOWARNING)
	{
		// no warning
		msg1[0]=0;
		GetRegistry("STR_FLASH_NO_WARNING",msg2,sizeof(msg2));
		hold_time = 1*1000;
	}
	else if(fs_num==0)
	{
		GetRegistry("STR_FLASH_DUP_WARNING",msg1,sizeof(msg1));
		GetRegistry("STR_FLASH0_WARNING",msg2,sizeof(msg2));
		hold_time = 5*1000;
	}
	else
	{
		GetRegistry("STR_FLASH_DUP_WARNING",msg1,sizeof(msg1));
		GetRegistry("STR_FLASH1_WARNING",msg2,sizeof(msg2));
		hold_time = 1*1000;
	}

	// duplicate warning 
	printf(msg1);

	// write warning 
	text_set_fc(COLOR_WARNING_FG);
	text_set_bc(COLOR_WARNING_BG);
	printf(msg2);
	text_set_fc(COLOR_NORMAL_FG);
	text_set_bc(COLOR_NORMAL_BG);

	if( wait_hold_button(PSP_CTRL_CIRCLE,hold_time) == 0)
		return 0;

	// cancel
	return flash_cancel();
}

/****************************************************************************
	flash install
****************************************************************************/
static const char dummy_file[] = "flash1:/dh/dummy.dat";

int flash1_install(void)
{
	if(ms_cur_path(src_path,1)==NULL || flash_cur_path(dst_path,1)==NULL)
		return -1;

	if(flash_title("STR_FLASH1_WORK",1)) return -1;

#if 0
	// remove registry for TEST!!
	sceIoRemove("flash1:/registry/system.ireg");
	sceIoRemove("flash1:/registry/system.dreg");
#endif

	remove_directry(dst_path);
	make_directry(dst_path);

	// dummy file for keep free space
	if( make_dummy_file(dummy_file,FLASH1_KEEP_FREE_SIZE) >= 0)
		copy_directry(src_path,dst_path,1);

	// remove dummy file
	printf("remove %s\n",dummy_file);
	sceIoRemove(dummy_file);

	flash_sync();
	flash_done();
	return 0;
}

/****************************************************************************
	flash install
****************************************************************************/
int flash1_backup(void)
{
	// flash0 install check
	if(check_directry("flash1:/dh/")==NULL)
	{
		text_clear();
		GetRegistry("STR_FLASH1_NONE_MSG",msg1,sizeof(msg1));
		printf(msg1);
		flash_done();
		return -1;
	}

	if(flash_cur_path(src_path,1)==NULL || ms_cur_path(dst_path,1)==NULL)
		return -1;

	if(flash_title("STR_FLASH1_BACKUP",1|NOWARNING)) return -1;

	remove_directry(dst_path);
	make_directry(dst_path);
	copy_directry(src_path,dst_path,1);
	flash_sync();
	flash_done();

	return 0;
}

/****************************************************************************
	flash install
****************************************************************************/
int flash1_cleanup(void)
{
	src_path[0] = 0;
	strcpy(dst_path,"flash1:/dh");

	if(flash_title("STR_FLASH1_CLEANUP",1)) return -1;

	// remove dest dir
	remove_directry(dst_path);
	flash_sync();

	flash_done();

	return 0;
}
/****************************************************************************
  flash0 font install
****************************************************************************/
int flash0_font(void)
{
	if(ms_cur_path(src_path,0)==NULL || flash_cur_path(dst_path,0)==NULL)
		return -1;

	strcat(src_path,"/font");
	strcat(dst_path,"/font");

	if(flash_title("STR_FLASH0_FONT",0)) return -1;

	if( flash0_enable_write()>=0)
	{
		remove_directry(dst_path);
		make_directry(dst_path);
		copy_directry(src_path,dst_path,1);

		flash0_protect();
	}

	flash_done();

	return 0;
}

/****************************************************************************
	flash0 install
****************************************************************************/
int flash0_kd_resource(void)
{
	if(ms_cur_path(src_path,0)==NULL || flash_cur_path(dst_path,0)==NULL)
		return -1;

	strcat(src_path,"/kd/resource");
	strcat(dst_path,"/kd/resource");

	if(flash_title("STR_FLASH0_KDRES",0)) return -1;

	if( flash0_enable_write()>=0)
	{
		remove_directry(dst_path);
		make_directry(dst_path);
		copy_directry(src_path,dst_path,1);

		flash0_protect();
	}
	flash0_protect();

	flash_done();
	return 0;
}

#if 0
/****************************************************************************
	flash0 install ALL FW with DarkAlex's CustomFW
****************************************************************************/
int flash0_all(void)
{
	// check dark alex's custom FW

	// remove 1.50 vsh modules
	// remove 1.50 kernel drivers

	// FW2.71 kd's
	if(ms_cur_path(src_path,0)==NULL || flash_cur_path(dst_path,0)==NULL)
		return -1;

	if(flash_title("STR_FLASH0_KDRES",0)) return -1;

	if( flash0_enable_write()>=0)
	{
		//remove_directry(dst_path);
		remove_directry("flash0:/");
		copy_directry(src_path,dst_path,1);

#if 0
		// devhook common
		copy_directry("ms0:/dh/kd","flash0:/dh/kd",0);

		// reboot.bin
		sprintf(dst_path,"flash0:%sb",flash_cur_path() ); // 'flash0:/dh/271b'
		make_directry(dst_path);
		sprintf(src_path,"%s/reboot.bin",ms_cur_path()    );        // ms
		strcat(dst_path,"/reboot.bin"); // 'flash0:/dh/271b/reboot.bin'
		copy_file(src_path,dst_path);
#endif

		flash0_protect();
	}

	flash_done();
	return 0;
}
#endif

/****************************************************************************
****************************************************************************/
int flash0_cleanup(void)
{
//	strcpy(src_path,"flash0:/dhcommon"); // devhook common directry
	strcpy(dst_path,"flash0:/dh");       // flashem root directry

	if(flash_title("STR_FLASH0_CLEANUP",0)) return -1;
	if( flash0_enable_write()>=0)
	{
		// remove dest dir
//		remove_directry(src_path);
		remove_directry(dst_path);
		flash0_protect();
	}

	flash_done();

	return 0;
}

/****************************************************************************
	FW install from PSAR Dumper
****************************************************************************/
const char psar_data[]   = "ms0:/F0/PSARDUMPER/data0.bin";
const char sysmem_path[] = "ms0:/F0/kd/sysmem.prx";

int install_psar_dumper(void)
{
	SceUID fd;
	int readed;
	char ver[4];
	int sig_or;
	int i;

	text_clear();
	GetRegistry("STR_PSAR_INSTALL",msg1,sizeof(msg1));
	text_printf(msg1);
	text_printf("\n\n");

	// check & get version code
	fd = sceIoOpen(psar_data, PSP_O_RDONLY, 0777);
	if(fd < 0 )
	{
		text_printf("open error\n");
		return flash_cancel();
	}

	readed = sceIoRead(fd, file_buf , sizeof(file_buf) );
	sceIoClose(fd);
	if(readed < 0x11)
	{
		text_printf("read error\n");
		return flash_cancel();
	}
	if(file_buf[0x1c]!=',' && file_buf[0x1e]!='.')
	{
		text_printf("unknown version code\n");
		return flash_cancel();
	}

	// check SIGCHECK file
	ver[0] = file_buf[0x1d];
	ver[1] = file_buf[0x1f];
	ver[2] = file_buf[0x20];
	ver[3] = 0;

	// read sysmem to check dump mode
	fd = sceIoOpen(sysmem_path, PSP_O_RDONLY, 0777);
	if(fd < 0 )
	{
		text_printf("'%s' open error\n",sysmem_path);
		return flash_cancel();
	}
	readed = sceIoRead(fd, file_buf , sizeof(file_buf) );
	sceIoClose(fd);
	if(readed < 0x150)
	{
		text_printf("'%s'  read error\n",sysmem_path);
		return flash_cancel();
	}

	// check encrypted kd
	if(memcmp(file_buf,"~PSP",4)!=0)
	{
		text_printf("Firmware is Decrypted\n");
		return flash_cancel();
	}

	// check SIG check area
	sig_or = 0;
	for(i=0xe0;i<0x120;i++) sig_or |= file_buf[i];
	if(sig_or==0)
	{
		text_printf("Firmware is no SIGCHECK\n");
		return flash_cancel();
	}

	// display FW version
	text_printf("Firmware Version is %c.%c%c\n",ver[0],ver[1],ver[2]);

	// start waiting
	GetRegistry("STR_START_BTN",msg1,sizeof(msg1));
	text_printf(msg1);
	text_printf("\n");
	if( (wait_key() & PSP_CTRL_CIRCLE)==0)
	{
		// cancel
		return flash_cancel();
	}

	// remove PSAR arcive

	sceIoRemove("ms0:/DATA.PSAR");

	// make FW base directry
	sprintf(dst_path,"ms0:/dh/%s/",ver);
	make_directry(dst_path);

#if 0
	move_file("ms0:/F0/reboot.bin",dst_path);
#endif

	// make base directry
	sprintf(dst_path,"ms0:/dh/%s/F1",ver);
	make_directry(dst_path);
	sprintf(dst_path,"ms0:/dh/%s/F0",ver);
	make_directry(dst_path);

	// remove lflash format module
	sceIoRename("ms0:/F0/kd/lflash_fatfmt.prx","lflash_fatfmt.prx_");

	// remove IPL / LOG file
	remove_directry("ms0:/F0/PSARDUMPER");

	// move flash0 files
	move_directry("ms0:/F0",dst_path);

	// remove PSAR DUMP directry
	// remove_directry("ms0:/F0");

	// finish
	flash_done();
	return 0;
}
