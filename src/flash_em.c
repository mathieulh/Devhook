/*
	PSP flashROM emulation

	based Driver functions By MPH (mphtheone@hotmail.com)
*/

#include "common.h"

#define HOOK_DRV 1

#define BLOCK_FORMAT 0

/* select log */

//#define LOG_OPEN
//#define LOG_DEV
//#define LOG_REDIRECT
#define LOG_ERR

// 2.71+
extern u32 IoFileMgrForKernel_bd17474f(PspIoDrvFileArg *arg);

#define ERR_NOT_FOUND   0x80010002
#define ERR_NOT_SUPPORT 0x80020325

// in msboot.h
//#define MS_FLASH0_PATH "/flash0"
//#define MS_FLASH1_PATH "/flash1"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#ifdef LOG_DEV
#define log_dev Kprintf
#else
#define log_dev(text,...) {}
#endif

#ifdef LOG_OPEN
#define log_open Kprintf
#else
#define log_open(text,...) {}
#endif

static const char dev_name[] = "flash";
#define DEV_NAME dev_name

#define ERR_DELETED        0x80010013
#define ERR_HANDLE_FULL    0x80010018

#define EXT_STACK_SIZE 0x4000

#define ERR_HANDLE_FULL    0x80010018

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// Temporary string to use
char drvString[512];

// flash redirect multi path list
static char path_list[2][128];
static const char dummy_direct[] = "";

static PspIoDrvFuncs *ioctrl_orig_table = NULL;
static int semaid = 0;

#if BLOCK_FORMAT
static PspIoDrvFuncs *ioctrl_lflash_table = NULL;
static int (*org_lflash_IoWrite)(PspIoDrvFileArg *arg, const char *data, int len);
//static int (*org_lflash_IoIoctl)(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);
#endif

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
const char *get_path_list(PspIoDrvFileArg *arg)
{
	if(arg->fs_num >=2) return dummy_direct;
	return path_list[arg->fs_num];
}

/////////////////////////////////////////////////////////////////////////////
// path redirecter
// call in semoaphore lock
/////////////////////////////////////////////////////////////////////////////
//SceIoStat *stat;
const char *createPathOne(const char *fname,const char *ptr,PspIoDrvFileArg *arg,int *is_ms)
{
	int plen;
	int fd;
	int fs_num = arg->fs_num;

	// make redirect path
retry:
	if(ptr==NULL)
	{
		*is_ms = -1;
		return NULL;
	}
	ptr = get_path_one(drvString,ptr);

	plen = strlen(drvString);
	*is_ms = strchr(drvString,':')!=NULL;

	// check validate base directry
	if(*is_ms==0 && fs_num==1)
	{
		//goto retry;
		// if fash1 and flash redirect , check base drectry
		fd = (ioctrl_orig_table->IoDopen)(arg,drvString);
//		fd = (ioctrl_orig_table->IoGetstat)(arg,drvString,stat);
		if(fd < 0)
		{
#ifdef LOG_REDIRECT
Kprintf("redirect bypass '%s'\n",drvString);
#endif
			goto retry;
		}
		(ioctrl_orig_table->IoDclose)(arg);
	}

	// add file path
	if(plen>0 && drvString[plen-1]=='/' && fname[0]=='/') fname++;
	strcpy(&drvString[plen],fname);

#ifdef LOG_REDIRECT
Kprintf("redirect 'flash%d:%s' ->'%s' : %08X , %d\n",fs_num,fname,drvString,ptr,*is_ms);
#endif

	return ptr;
}

/////////////////////////////////////////////////////////////////////////////
// semaohore
/////////////////////////////////////////////////////////////////////////////

static void Lock(void)
{
	sceKernelWaitSema(semaid, 1, 0);
}
static void Unlock(void)
{
	sceKernelSignalSema(semaid, 1);
}

/////////////////////////////////////////////////////////////////////////////
// lflash.IoIoctl
/////////////////////////////////////////////////////////////////////////////
/*
Hook lflash format
.....lflash0.IoIoctl(88052EB0,0003D001, 00000000,000000, 8825D1D0,000044)::RET(0
0000000)
....lflash0.IoIoctl(88052EB0,554D5302, 00000000,000000, 8825D1B8,000004)::RET(80
010016)
.......lflash0.IoWrite(88052EB0,882656F8,00004000):RET(00004000)
.lflash0.IoWrite(88052EB0,882656F8,00004000):RET(00004000)
lflash0.IoWrite(88052EB0,882656F8,00004000):RET(00004000)
.lflash0.IoWrite(88052EB0,882656F8,00004000):RET(00004000)
........lflash0.IoIoctl(88052EB0,0003D001, 00000000,000000, 8825D1D0,000044)::RE
T(00000000)
....lflash0.IoIoctl(88052EB0,554D5302, 00000000,000000, 8825D1B8,000004)::RET(80
010016)
.......lflash0.IoWrite(88052EB0,882656F8,00004000):RET(00004000)
.lflash0.IoWrite(88052EB0,882656F8,00004000):RET(00004000)
lflash0.IoWrite(88052EB0,882656F8,00004000):RET(00004000)
.lflash0.IoWrite(88052EB0,882656F8,00004000):RET(00004000)
...force READ MOUNT to flash0:
lflash0.IoIoctl(88052F50,0003D001, 00000000,000000, 882F9370,000044)::RET(000000
00)
lflash0.IoIoctl(88052FF0,0003D001, 00000000,000000, 882F9370,000044)::RET(000000
00)
Release lflash format
*/
static int hook_lflash_IoWrite(PspIoDrvFileArg *arg, const char *data, int len)
{
	int result;
Kprintf("%s%d.IoWrite(%08X,%08X,%08X)%\n","lflash",arg->fs_num,(int)arg,(int)data,len);
	Kprintf("Block lflash format\n");
	result = len;
//	result = org_lflash_IoWrite(arg,data,len);
Kprintf(":RET(%08X)\n",result);
	return result;
}

#if 0
static int hook_lflash_IoIoctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
	int result;

Kprintf("%s%d.IoIoctl(%08X,%08X, %08X,%06X, %08X,%06X):","lflash",arg->fs_num, (int)arg,cmd,(int)indata,inlen,(int)outdata,outlen);

#if 0
	//if(arg->fs_num==1)
	if(cmd==0x00003d001)
	{
Kprintf("Block lflash format\n");
		return 0;
	}
#endif
	result = org_lflash_IoIoctl(arg,cmd,indata,inlen,outdata,outlen);

Kprintf(":RET(%08X)\n",result);
	return result;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// IoInit
/////////////////////////////////////////////////////////////////////////////
static int IoInit(PspIoDrvArg* arg)
{
	int result=0;
	log_dev("%s.init(%08X)",DEV_NAME,(int)arg);

	if(ioctrl_orig_table->IoInit)
		result = (ioctrl_orig_table->IoInit)(arg);
	log_dev(":RET(%08X)\n",result);

	return result;
}

/////////////////////////////////////////////////////////////////////////////
// IoExit
/////////////////////////////////////////////////////////////////////////////
static int IoExit(PspIoDrvArg* arg)
{
	int result=0;

	log_dev("%s.exit(%08X)",DEV_NAME,(int)arg);

	ms_share_shutdown();

	if(ioctrl_orig_table->IoExit)
		result = (ioctrl_orig_table->IoExit)(arg);
	log_dev(":RET(%08X)\n",result);

	return result;
}

/////////////////////////////////////////////////////////////////////////////
// IoOpen
/////////////////////////////////////////////////////////////////////////////
static int IoOpen(PspIoDrvFileArg *arg, char *path, int flags, SceMode mode)
{
	int result = ERR_NOT_SUPPORT;
	const char *ptr;
	int is_ms;

	log_open("%s%d.IoOpen(%08X,%s,%08X,%08X):",DEV_NAME,arg->fs_num,(int)arg,path,flags,mode);

//	if( strcmp(path,"/net/http/auth.dat")==0) return result;
//	if( strcmp(path,"/net/http/cookie.dat")==0) return result;

	// multi path search
	Lock();
	ptr = get_path_list(arg);
	while(result<0)
	{
		ptr = createPathOne(path,ptr,arg,&is_ms);
		if(is_ms<0) break;
		if(is_ms) result = ms_share_open(arg,drvString,flags,mode);
		else result = (ioctrl_orig_table->IoOpen)(arg,drvString,flags,mode);			}
	Unlock();
	log_open("result = %08X\n",result);

#ifdef LOG_ERR
	if(result<0)
		Kprintf("%s%d.IoOpen(%08X,%s,%08X,%08X) error:%08x\n",DEV_NAME,arg->fs_num,(int)arg,path,flags,mode,result);
#endif

	return result;
}
/////////////////////////////////////////////////////////////////////////////
// IoClose
/////////////////////////////////////////////////////////////////////////////
static int IoClose(PspIoDrvFileArg *arg)
{
	int result;
	log_dev("%s%d.IoClose(%08X):",DEV_NAME,arg->fs_num,(int)arg);

	Lock();
	// try ms
	result = ms_share_close(arg);

	if(result!=0)
	{
		// flash
		result = (ioctrl_orig_table->IoClose)(arg);
	}
	Unlock();

	log_dev("result = %08X\n",result);
	return result;
}

/////////////////////////////////////////////////////////////////////////////
// IoRead
/////////////////////////////////////////////////////////////////////////////
static int IoRead(PspIoDrvFileArg *arg, char *data, int len)
//(unsigned int *fp,void *buf,int size)
{
	int result;
	log_dev("%s%d.IoRead(%08X,%08X,%08X):",DEV_NAME,arg->fs_num,(int)arg,(int)data,len);

	Lock();
	if(!ms_share_check_fd(arg) )
	{
		// msshare
		result = ms_share_read(arg,data,len);
	}
	else
	{
		if(!ioctrl_orig_table->IoRead) result = -1;
		else result = (ioctrl_orig_table->IoRead)(arg,data,len);
	}
	Unlock();

	log_dev("result = %08X\n",result);
	return result;
}

/////////////////////////////////////////////////////////////////////////////
// IoWrite
/////////////////////////////////////////////////////////////////////////////
static int IoWrite(PspIoDrvFileArg *arg, const char *data, int len)
{
	int result;
	log_dev("%s%d.IoWrite(%08X,%08X,%08X)%\n",DEV_NAME,arg->fs_num,(int)arg,(int)data,len);

	Lock();
	if(!ms_share_check_fd(arg) )
	{
		if(data==NULL &&len==0) result = 0; // 'init.dat'
		else result = ms_share_write(arg,data,len);
		// ms_share_flush(arg);
	}
	else
	{
		if(!ioctrl_orig_table->IoWrite) result = -1;
		else result = (ioctrl_orig_table->IoWrite)(arg,data,len);
	}
	Unlock();
	log_dev(":RET(%08X)\n",result);

	return result;
}

/////////////////////////////////////////////////////////////////////////////
// IoLseek
/////////////////////////////////////////////////////////////////////////////
static SceOff IoLseek(PspIoDrvFileArg *arg, u32 unk, SceOff ofs, int whence)
{
	int result;
	log_dev("%s%d.IoLseek(%08X,%08X,%08X,%d)%\n",DEV_NAME,arg->fs_num,(int)arg,unk,(int)ofs,whence);

	Lock();
	if(!ms_share_check_fd(arg) )
	{
		result = ms_share_seek(arg,unk,ofs,whence);
	}
	else
	{
		if(!ioctrl_orig_table->IoLseek) result = -1;
		else result = (ioctrl_orig_table->IoLseek)(arg,unk,ofs,whence);
	}
	Unlock();

	log_dev("result = %08X\n",(int)result);
	return result;
}

/////////////////////////////////////////////////////////////////////////////
// IoIoctl
/////////////////////////////////////////////////////////////////////////////
static int IoIoctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
	int result;

	log_dev("%s%d.IoIoctl(%08X,%08X, %08X,%06X, %08X,%06X):",DEV_NAME,arg->fs_num, (int)arg,cmd,(int)indata,inlen,(int)outdata,outlen);
//Kprintf("%s%d.IoIoctl(%08X,%08X, %08X,%06X, %08X,%06X):",DEV_NAME,arg->fs_num, (int)arg,cmd,(int)indata,inlen,(int)outdata,outlen);

	Lock();
	if(!ms_share_check_fd(arg) )
	{
		switch(cmd)
		{
		case 0x00008003: // FW1.50
		case 0x00208003: // FW2.00 load prx  "/vsh/module/opening_plugin.prx"
			log_dev("%s%d.IoIoctl:Unknown command %08X\n",DEV_NAME,arg->fs_num,cmd);
			result = 0;
			break;

		case 0x00208006: // load prx
			log_dev("%s%d.IoIoctl:Unknown command %08X\n",DEV_NAME,arg->fs_num,cmd);
			result = 0;
			break;

		case 0x00208007: // after start FW2.50
			log_dev("%s%d.IoIoctl:Unknown command %08X\n",DEV_NAME,arg->fs_num,cmd);
			result = 0;
			break;

		case 0x00208081: // FW2.00 load prx "/vsh/module/opening_plugin.prx"
			log_dev("%s%d.IoIoctl:Unknown command %08X\n",DEV_NAME,arg->fs_num,cmd);
			result = 0;
			break;

#if 1
		case 0x00208082: // FW2.80 "/vsh/module/opening_plugin.prx"
//			Kprintf("\n\n%s%d.IoIoctl(%08X,%08X, %08X,%06X, %08X,%06X):",DEV_NAME,arg->fs_num, (int)arg,cmd,(int)indata,inlen,(int)outdata,outlen);
//			Kprintf("%s%d.IoIoctl:Unknown command %08X\n",DEV_NAME,arg->fs_num,cmd);

			result = 0x80010016; // opening_plugin.prx , mpeg_vsh,prx , impose_plugin.prx
			break;
#endif

		case 0x00005001: // vsh_module : system.dreg / sytem.ireg
			//‚½‚Ô‚ñAwrite flush
			log_dev("%s%d.IoIoctl:Unknown command %08X\n",DEV_NAME,arg->fs_num,cmd);
			result = ms_share_flush(arg);
			break;
		default:
			Kprintf("%s%d.IoIoctl:Unknown command %08X : trap -------------------------- \n",DEV_NAME,arg->fs_num,cmd);
			//result = sceIoIoctl((SceUID) arg->arg,cmd,indata,inlen,outdata,outlen);
			result = 0xffffffff;
		}
	}
	else
	{
		// in flash emu
//		if(!ioctrl_orig_table->IoIoctl) result = -1;
//		else
		result = (ioctrl_orig_table->IoIoctl)(arg,cmd,indata,inlen,outdata,outlen);

#if 0
		if(cmd==0x00208082)
		{
			Kprintf("\n\n%s%d.IoIoctl(%08X,%08X, %08X,%06X, %08X,%06X):",DEV_NAME,arg->fs_num, (int)arg,cmd,(int)indata,inlen,(int)outdata,outlen);
			Kprintf(":RET(%08X)\n",result);
//			while(1);
		}
#endif

	}
	Unlock();

	log_dev(":RET(%08X)\n",result);
	return result;
}

/////////////////////////////////////////////////////////////////////////////
// IoRemove
/////////////////////////////////////////////////////////////////////////////
static int IoRemove(PspIoDrvFileArg *arg, const char *name)
{
	int result = ERR_NOT_SUPPORT;
	const char *ptr;
	int is_ms;

	log_dev("%s%d.IoRemove(%08X,%s)",DEV_NAME,arg->fs_num,(int)arg,name);

	Lock();
	ptr = get_path_list(arg);
	while(result<0)
	{
		ptr = createPathOne(name,ptr,arg,&is_ms);
		if(is_ms<0) break;
		if(is_ms) result = sceIoRemove(drvString);
		else result = (ioctrl_orig_table->IoRemove)(arg,drvString);
	}
	Unlock();

	log_dev(":RET(%08X)\n",result);
	return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int IoMkdir(PspIoDrvFileArg *arg, const char *path, SceMode mode)
{
	int result = ERR_NOT_SUPPORT;
	const char *ptr;
	int is_ms;

	log_dev("%s%d.IoMkdir(%08X,%s,%08X)",DEV_NAME,arg->fs_num,(int)arg,path,mode);

	Lock();
	ptr = get_path_list(arg);
	while(result<0)
	{
		ptr = createPathOne(path,ptr,arg,&is_ms);
		if(is_ms<0) break;
		if(is_ms) result = sceIoMkdir(drvString,mode);
		else result = (ioctrl_orig_table->IoMkdir)(arg,drvString,mode);
	}
	Unlock();

	log_dev(":RET(%08X)\n",result);
	return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int IoRmdir(PspIoDrvFileArg *arg, const char *path)
{
	int result = ERR_NOT_SUPPORT;
	const char *ptr;
	int is_ms;

	log_dev("%s.IoRmdir(%08X,%s,%08X)",DEV_NAME,(int)arg,path);

	Lock();
	ptr = get_path_list(arg);
	while(result<0)
	{
		ptr = createPathOne(path,ptr,arg,&is_ms);
		if(is_ms<0) break;
		if(is_ms) result = sceIoRmdir(drvString);
		else result = (ioctrl_orig_table->IoRmdir)(arg,drvString);
	}
	Unlock();

	log_dev(":RET(%08X)\n",result);
	return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int IoDopen(PspIoDrvFileArg *arg, const char *path)
{
	int result = ERR_NOT_SUPPORT;
	const char *ptr;
	int is_ms;

	log_open("%s.IoDopen(%08X,%s)",DEV_NAME,(int)arg,path);

	Lock();
	ptr = get_path_list(arg);
	while(result<0)
	{
		ptr = createPathOne(path,ptr,arg,&is_ms);
		if(is_ms<0) break;
		if(is_ms) result = ms_share_open(arg,drvString,0,MODE_DOPEN);
		else result = (ioctrl_orig_table->IoDopen)(arg,drvString);
	}
	Unlock();

	log_open(":RET(%08X)\n",result);
	return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int IoDclose(PspIoDrvFileArg *arg)
{
	int result;
	log_dev("%s.IoDclose(%08X)",DEV_NAME,(int)arg);

	Lock();

	// try ms_share
	result = ms_share_close(arg);
	if(result == MSS_ERR_OTHER_DEVICE)
	{
		// if other devive then flash device
		result = (ioctrl_orig_table->IoDclose)(arg);
	}
	result = 0;
	Unlock();

	log_dev(":RET(%08X)\n",result);

	return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int IoDread(PspIoDrvFileArg *arg, SceIoDirent *dir)
{
	int result;
	log_dev("%s.IoDread(%08X,%s)",DEV_NAME,(int)arg,dir);

	Lock();
	result = ms_share_dread(arg,dir);
	if(result == MSS_ERR_OTHER_DEVICE)
	{
		result = (ioctrl_orig_table->IoDread)(arg,dir);
	}

	Unlock();

	log_dev(":RET(%08X)\n",result);

	return result;
}

/////////////////////////////////////////////////////////////////////////////
// stack overflow no stack extended in 2.60
// return 0x80010016 case 2.71 and buf is user mode area > patched
/////////////////////////////////////////////////////////////////////////////
static int IoGetstatCore(u32 *argv)
{
	const char *file = (const char *)argv[0];
	SceIoStat *stat = (SceIoStat *)argv[1];
	int result = sceIoGetstat(file,stat);
	return result;
}

static int IoGetstat(PspIoDrvFileArg *arg, const char *path, SceIoStat *stat)
{
	int result = ERR_NOT_SUPPORT;
	const char *ptr;
	int is_ms;

	log_dev("%s%d.IoGetstat(%08X,%s,%08X)",DEV_NAME,arg->fs_num,(int)arg,path,(int)stat);

	Lock();
	ptr = get_path_list(arg);
	while(result<0)
	{
		ptr = createPathOne(path,ptr,arg,&is_ms);
		if(is_ms<0) break;
		if(is_ms)
		{
			// stack overflow OUTRUN2006
			//result = sceIoGetstat(file,stat);
			u32 argv[2];
			argv[0] = (u32)drvString;
			argv[1] = (u32)stat;
			result = (int)sceKernelExtendKernelStack(EXT_STACK_SIZE,(void *)IoGetstatCore,(void *)argv);
		}
		else result = (ioctrl_orig_table->IoGetstat)(arg,drvString,stat);
	}
	Unlock();

	log_dev(":RET(%08X)\n",result);
	return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int IoChstat(PspIoDrvFileArg *arg, const char *path, SceIoStat *stat, int bits)
{
	int result = ERR_NOT_SUPPORT;
	const char *ptr;
	int is_ms;

	log_dev("%s.IoChstat(%08X,%s,%08X,%08X)",DEV_NAME,(int)arg,path,(int)stat,bits);

	Lock();
	ptr = get_path_list(arg);
	while(result<0)
	{
		ptr = createPathOne(path,ptr,arg,&is_ms);
		if(is_ms<0) break;
		if(is_ms) result = sceIoChstat(drvString,stat,bits);
		else result = (ioctrl_orig_table->IoChstat)(arg,drvString,stat,bits);
	}
	Unlock();

	log_dev(":RET(%08X)\n",result);
	return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int IoRename(PspIoDrvFileArg *arg, const char *path, const char *newname)
{
	int result = ERR_NOT_SUPPORT;
	const char *ptr;
	int is_ms;

	log_dev("%s.IoRename(%08X,%s,%s)",DEV_NAME,(int)arg,path,newname);

	Lock();
	ptr = get_path_list(arg);
	while(result<0)
	{
		ptr = createPathOne(path,ptr,arg,&is_ms);
		if(is_ms<0) break;
		if(is_ms) result = sceIoRename(drvString,newname);
		else result = (ioctrl_orig_table->IoRename)(arg,drvString,newname);
	}
	Unlock();

	log_dev(":RET(%08X)\n",result);
	return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int IoChdir(PspIoDrvFileArg *arg, const char *path)
{
	int result = ERR_NOT_SUPPORT;
	const char *ptr;
	int is_ms;

	log_dev("%s.IoChdir(%08X,%s)",DEV_NAME,(int)arg,path);

	Lock();
	ptr = get_path_list(arg);
	while(result<0)
	{
		ptr = createPathOne(path,ptr,arg,&is_ms);
		if(is_ms<0) break;
		if(is_ms) result = sceIoChdir(drvString);
		else result = (ioctrl_orig_table->IoChdir)(arg,drvString);
	}
	Unlock();

	log_dev(":RET(%08X)\n",result);
	return result;
}

/****************************************************************************
called from sceIoAssign

asgn_name : "flash0:" / "flash1:"
dev_name  : "lflash0:0,0" / "lflash0:0,1"
wr_mode   : 1 = IOASSIGN_RDONLY , 0 = IOASSIGN_RDWR

"lflash0:0,0","flashfat0:" : partation 0
"lflash0:0,1","flashfat1:" : partation 1
"lflash0:0,2","flashfat2:" : partation 2
"lflash0:0,3","flashfat3:" : partation 3
"lflash0:0,4","flashfat4:" : none
"lflash0:0,5","flashfat5:" : none

****************************************************************************/
static int IoMount(PspIoDrvFileArg *arg,const char *asgn_name,const char *dev_name,int wr_mode,int unk)
{
	int result;

	log_dev("%s.IoMount(%08X,'%s','%s',%d)",DEV_NAME,(int)arg,(int)asgn_name,(int)dev_name,wr_mode);

//Kprintf("%s.IoMount(%08X,'%s','%s',%d)",DEV_NAME,(int)arg,(int)asgn_name,(int)dev_name,wr_mode);

	 // force READ mode to "flash0:"
	if(arg->fs_num==0 && (wr_mode==0) )
	{
Kprintf("force READ MOUNT to flash0:\n");
		 wr_mode = 1;
	}

	// called when 2.71 initialize
//	if(!ioctrl_orig_table->IoMount) result = -1;
//	else
	result = (ioctrl_orig_table->IoMount)(arg,asgn_name,dev_name,wr_mode,unk);

#if BLOCK_FORMAT
	// release format access
	if(arg->fs_num==1 && ioctrl_lflash_table)
	{
		// hook Write Access
//		ioctrl_lflash_table->IoIoctl = org_lflash_IoIoctl;
		ioctrl_lflash_table->IoWrite = org_lflash_IoWrite;

		ioctrl_lflash_table = NULL;
Kprintf("Release lflash format\n");
	}
#endif

	log_dev(":RET(%08X)\n",result);
	return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int IoUmount(PspIoDrvFileArg *arg)
{
	int result;
	log_dev("%s%d.IoUmount(%08X)",DEV_NAME,arg->fs_num,(int)arg);
//Kprintf("%s%d.IoUmount(%08X)",DEV_NAME,arg->fs_num,(int)arg);

	// called when 2.71 initialize
	// mss close
	ms_share_close_all();

	// flash
	if(!ioctrl_orig_table->IoUmount) result = -1;
	else result = (ioctrl_orig_table->IoUmount)(arg);

#if BLOCK_FORMAT
	// hook format access
	if(arg->fs_num==1 && ioctrl_lflash_table==NULL)
	{
		PspIoDrv *lflash_dev = search_device_io("lflash",0x200,0x0004,NULL);
		if(lflash_dev)
		{
			ioctrl_lflash_table = lflash_dev->funcs;
			// hook Write Access
//			org_lflash_IoIoctl = ioctrl_lflash_table->IoIoctl;
//			ioctrl_lflash_table->IoIoctl = hook_lflash_IoIoctl;

			org_lflash_IoWrite = ioctrl_lflash_table->IoWrite;
			ioctrl_lflash_table->IoWrite = hook_lflash_IoWrite;

Kprintf("Hook lflash format\n");
		}
	}
#endif

	log_dev(":RET(%08X)\n",result);
	return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int IoDevctl(PspIoDrvFileArg *arg,const char *auther,u32 cmd,void *idata,int isize,void *odata,int osize)
{
	int result;

	log_dev("%s%d.IoDevctl(%08X,%s,%08X,%08X,%06X, %08X,%06X)\n",DEV_NAME,arg->fs_num,(int)arg,auther,cmd,(int)idata,isize,(int)odata,osize);

	// MSS
	switch(cmd)
	{
	// if cmd==5802 and return not 0,registry break
	case 0x5802:	// write buffer flash ?
		// sync_all_unit()
		// FlushWriteCache()
		log_dev("%s%d.IoDevctl:Unknown command %08X\n",DEV_NAME,arg->fs_num,cmd);
		ms_share_close_all();

		// flash NANDflash  too
		result = (ioctrl_orig_table->IoDevctl)(arg,auther,cmd,idata,isize,odata,osize);
		break;
		
	default:
		Kprintf("%s%d.IoDevctl:Unknown command %08X\n",DEV_NAME,arg->fs_num,cmd);
		while(1);
	}

	// update FLASH too
	result = (ioctrl_orig_table->IoDevctl)(arg,auther,cmd,idata,isize,odata,osize);

	log_dev(":RET(%08X)\n",result);

	return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int IoUnk21(PspIoDrvFileArg *arg)
{
	Kprintf("%s%d.IoUnk21 %08X\n",DEV_NAME,arg->fs_num,(int)arg);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// emulate IOCTRL entry tables
/////////////////////////////////////////////////////////////////////////////
static PspIoDrvFuncs ioctrl_hook_table = 
{
  IoInit,
  IoExit,
  IoOpen,
  IoClose,
  IoRead,
  IoWrite,
  IoLseek,
  IoIoctl,
  IoRemove,
  IoMkdir,
  IoRmdir,
  IoDopen,
  IoDclose,
  IoDread,
  IoGetstat,
  IoChstat,
  IoRename,
  IoChdir,
  IoMount,
  IoUmount,
  IoDevctl,
  IoUnk21
};

/////////////////////////////////////////////////////////////////////////////
// hook device name
/////////////////////////////////////////////////////////////////////////////
const char flashfat[] = "flashfat";
const char flash_name150[] = "FAT over USB Mass";
const char flash_name260[] = "FAT over Flash";

/////////////////////////////////////////////////////////////////////////////
// uninst
/////////////////////////////////////////////////////////////////////////////
int remove_flash_emu(void)
{
	PspIoDrvFuncs *removed = NULL;

//Kprintf("remove_flash_emu()\n");

	if(ioctrl_orig_table == NULL) return 0;

	Lock(); // wait for hooked file access
	// 2.60+
	removed = hook_device(flashfat,0x01,0x001e0010,flash_name260,ioctrl_orig_table);
	if(!removed)
		// 1.50 - 2.00
		removed = hook_device(flashfat,0x01,0x10,flash_name150,ioctrl_orig_table);
	Unlock();

	// wait for hooked file access
	sceKernelDelayThread(50000);
	Lock();
	Unlock();

	// shutdown opened MS share
	// opened handle are danger
	ms_share_close_all();

	// free semaphore
	if(semaid)
	{
		sceKernelDeleteSema(semaid);
		semaid = 0;
	}

	// report
	if(removed)
	{
		ioctrl_orig_table = NULL;
		Kprintf("NAND: release\n");
	}
	else
	{
		Kprintf("NAND: Can't release\n");
	}

	return removed ? 0 : -1;
}
/////////////////////////////////////////////////////////////////////////////
// install
/////////////////////////////////////////////////////////////////////////////
int install_flash_emu(void)
{
//Kprintf("install_flash_emu()\n");

	// create lock semaphore
	if(semaid==0)
	{
		semaid = sceKernelCreateSema("flash", 0, 1, 1, 0);
	}

	// installed ?
	if(ioctrl_orig_table) return 0;

	// get flash redirect path list
	path_list[0][0] = path_list[1][0] = 0;
	dhGetRegistry("FLASH0",path_list[0],sizeof(path_list[0]));
	dhGetRegistry("FLASH1",path_list[1],sizeof(path_list[1]));

//Kprintf("FLASH0=%s\n",path_list[0]);
//Kprintf("FLASH1=%s\n",path_list[1]);

	// when no-redirect then no-hook
	if(path_list[0][0]==0x00 && path_list[1][0]==0x00) return 0;

	// for 2.71
	// user level is fix to kernel mode for bypass IoGetstat error
	u32 *lp = (u32 *)getAPIEntry((void *)IoFileMgrForKernel_bd17474f);
	if(lp) lp[1] = MIPS_ORI(2,0,0x0008);
	// clear_cache();

	// hoook IOCTRL table directry
	ioctrl_orig_table = hook_device(flashfat,0x01,0x001e0010,flash_name260,&ioctrl_hook_table); // 2.60+
	if(ioctrl_orig_table == NULL)
	{
		// -2.00
		ioctrl_orig_table = hook_device(flashfat,0x01,0x10,flash_name150,&ioctrl_hook_table);
	}
//Kprintf("WAIT MS\n");
	// if ms0: found , install ms_share
	Lock();
	wait_device("ms0:/",200);
	if( strchr(path_list[0],':') || strchr(path_list[1],':') )
	{
		 ms_share_install();
	}
	Unlock();

	// wait for flash1 enable
//	wait_device("flash1:/",200);

	if(ioctrl_orig_table==NULL)
	{
Kprintf("NAND: Can't hook\n");
		remove_flash_emu();
		return -1;
	}
Kprintf("NAND: hook\n");
	return 0;
}
