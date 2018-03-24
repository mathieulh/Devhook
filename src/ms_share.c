/*
	ms0: device handle sharing
*/

// *** INCLUDES ***

#include "common.h"

/* max handle */
#define MAX_HANDLE 24

#define HOOK_OPEN      1
#define HOOK_DOPEN     1
#define HOOK_UMOUNT    1
#define HOOK_MKDIR     1
#define HOOK_DREAD     1

#define HOOK_IOCTRL    0
#define HOOK_DEVCTRL   0

#define MSSTOR_DEVCTRL 0
#define MSSTOR_IOCTRL  0

// change log
//#define LOG_SHARE
//#define LOG_OPEN
//#define DUMP_FD

// return after USB connect / MS eject
#define ERR_DELETED        0x80010013
#define ERR_HANDLE_FULL    0x80010018
#define ERR_NOT_FOUND      0x80010002

// return after resume
#define ERR_UNKNOWN_DEVICE 0x80020321

static const char dev_name[] = "ms_share";

#define STACK_SIZE_OPEN   0x4000
#define STACK_SIZE_CLOSE  0x4000
#define STACK_SIZE_READ   0x4000
#define STACK_SIZE_WRITE  0x4000
#define STACK_SIZE_SEEK   0x4000
#define STACK_SIZE_DREAD  0x4000
#define STACK_SIZE_CLOSE1 0x4000
#define STACK_SIZE_CLOSEA 0x4000

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

#ifdef LOG_SHARE
#define log_share Kprintf
#else
#define log_share(text,...) {}
#endif
#ifdef LOG_OPEN
#define log_open Kprintf
#else
#define log_open(text,...) {}
#endif

/////////////////////////////////////////////////////////////////////////////
// fd shareing database
/////////////////////////////////////////////////////////////////////////////

typedef struct share_fd_struct
{
	SceUID fd;
	int flags;
	SceMode mode;
	u32 fpos;
	unsigned char valid;
	char name[64];
}SHARE_FD;

static SHARE_FD flash_fd[MAX_HANDLE];
static int share_semaid = 0;
//static int sema_lock;

/////////////////////////////////////////////////////////////////////////////
// semaphore
/////////////////////////////////////////////////////////////////////////////

static void mss_lock(void)
{
	sceKernelWaitSema(share_semaid, 1, 0);
}

static void mss_unlock(void)
{
	sceKernelSignalSema(share_semaid, 1);
}

/////////////////////////////////////////////////////////////////////////////
// ロック状態で読んではいけない
// df_open内でロックするため
/////////////////////////////////////////////////////////////////////////////
static SceUID IoOpenRetry(const char *file, int flags, SceMode mode)
{
	int retry , result;

	// 80ms NODEVICE error after resume
	for(retry=10;retry>0;retry--)
	{
		result = sceIoOpen(file,flags,mode);
		switch(result)
		{
#if 0
		case ERR_UNKNOWN_DEVICE:
			goto retry;
#endif
		case ERR_NOT_FOUND: // file not found
		case ERR_HANDLE_FULL:
			goto exit;
		default:
			if(result>0) goto exit;
		}
log_share("IoOpenRetry %08x : Retry\n",result);
		sceKernelDelayThread(50000); // 50msec
	}
exit:
log_share("IoOpenRetry result %08x\n",result);
	return result;
}

/////////////////////////////////////////////////////////////////////////////
// ロック状態で呼ぶ
/////////////////////////////////////////////////////////////////////////////
static SceUID msIoClose(SHARE_FD *fp)
{
	int result = 0;

	if(fp->fd>0)
	{
log_share("msIoClose %s fd %d\n",fp->name,fp->fd);
#if 0
		// update fpos
		int fpos = sceIoLseek32(fp->fd,0,PSP_SEEK_CUR);
		if(fpos>=0) fp->fpos = fpos;
#endif
		result = (fp->mode==MODE_DOPEN) ? sceIoDclose(fp->fd) : sceIoClose(fp->fd);
		fp->fd = -1;
	}
	return result;
}

/////////////////////////////////////////////////////////////////////////////
// ロック状態で呼ぶ
/////////////////////////////////////////////////////////////////////////////
static SHARE_FD *get_older_fp(void)
{
	SHARE_FD *older_fp;
	SHARE_FD *fp;
	int i;
	int valid = 0;
	int older_valid;

	older_fp    = NULL;
	older_valid = 0x100;

	for(fp=flash_fd,i=0;i<MAX_HANDLE;fp++,i++)
	{
		//if( (fp->name[0]!=0) && (fp->fd>0) )
		if( (fp->name[0]!=0) && (fp->fd>0) && (fp->mode!=MODE_DOPEN) ) // dopenは閉じちゃダメ
		{
			valid++;
			if(older_valid > fp->valid)
			{
				// found more older file
				older_fp = fp;
				older_valid = fp->valid;
			}
		}
	}

log_share("%s.opened nums %d , close '%s'\n",dev_name,valid,older_fp ? older_fp->name : "NO-FILE");

	if(older_fp == NULL)
	{
Kprintf("%s.no opened fd\n",dev_name);
	}
	return older_fp;
}

/////////////////////////////////////////////////////////////////////////////
// ロック状態で呼ぶ
/////////////////////////////////////////////////////////////////////////////
static SHARE_FD *flash_get_free_fp(void)
{
	int i;
	SHARE_FD *fp;

	for(i=1;i<MAX_HANDLE;i++)
	{
		fp = &flash_fd[i];
		if(fp->name[0]==0) return fp;
	}
	// full !
//	log_share("%s.full file handle error\n",dev_name);
Kprintf("%s.full file handle error\n",dev_name);
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// ロック状態で呼ぶ
/////////////////////////////////////////////////////////////////////////////
static void ms_share_close_all_core(int clear_fd)
{
	SHARE_FD *fp;
	int i;

	for(fp=flash_fd,i=0;i<MAX_HANDLE;fp++,i++)
	{
		// fd=00はopen時でロックしているので除外
		if( (fp->name[0]) && (fp->fd>0) )
		{
			msIoClose(fp);
		}
		if(clear_fd)
			fp->name[0] = 0x00; // celar FD
	}
}

/////////////////////////////////////////////////////////////////////////////
// ロック状態で呼ぶ
/////////////////////////////////////////////////////////////////////////////
static void ms_share_close_one(void)
{
	SHARE_FD *fp;

	// 一番古いfd
	fp = get_older_fp();
	if(!fp)
	{
		log_share("%s.close_one : empty shared handler\n",dev_name);
		return;
	}
log_share("%s.close_one : fp %08X : %X\n",dev_name,(int)fp,fp->fd);
	msIoClose(fp);
}

/////////////////////////////////////////////////////////////////////////////
// dopenのポジショニング
/////////////////////////////////////////////////////////////////////////////
static int dopen_reseek(SHARE_FD *fp)
{
	SceIoDirent dummy; // 256bytes 以上サイズがある
	int i;
	for(i=0;i<fp->fpos;i++)
	{
		sceIoDread(fp->fd,&dummy);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// fd確認
/////////////////////////////////////////////////////////////////////////////
static int ms_share_check_fd_core(SHARE_FD *fp)
{
	if( ( (u32)fp < (u32)flash_fd) || (u32)fp > (u32)&(flash_fd[MAX_HANDLE]) )
	{
		return MSS_ERR_OTHER_DEVICE; // non mss device
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// 外部参照
/////////////////////////////////////////////////////////////////////////////
int ms_share_check_fd(PspIoDrvFileArg* arg)
{
	int result;

	mss_lock();
	result = ms_share_check_fd_core( (SHARE_FD *)arg->arg );
	mss_unlock();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
// ロック状態で呼ぶこと
/////////////////////////////////////////////////////////////////////////////
static int ms_share_get_fd_core(PspIoDrvFileArg* arg)
{
	SHARE_FD *fp = (SHARE_FD *)arg->arg;
	int i;
	int fd;
	int result;

	// check ms share'd device
	result = ms_share_check_fd_core(fp);
	if(result < 0) return result;

	// decrement valid flag
	for(i=0;i<MAX_HANDLE;i++)
	{
		if(flash_fd[i].valid)
			flash_fd[i].valid--;
	}
	// refresh & lock current access
	fp->valid = 0xff;

#if 1
	// check deleted device first
	if(fp->fd > 0)
	{
		int result = sceIoLseek32(fp->fd,0,PSP_SEEK_CUR);

		if(fp->fpos != result)
		{
			// pre-close fd
			msIoClose(fp);

			// all close after USB Connect
			if(result == ERR_DELETED)
			{
				ms_share_close_all_core(0);
			}
log_share("%s.pre-close '%s'\n",dev_name,fp->name);
		}
	}
#endif

	// re-open when closed
	if(fp->fd < 0)
	{
		int result = 0;

		fp->fd = 0x00; // lock FD !!

		mss_unlock();
		fd = (fp->mode==MODE_DOPEN) ? sceIoDopen(fp->name) : IoOpenRetry(fp->name,fp->flags,fp->mode);
		mss_lock();
		fp->fd = fd;

		if(fd >0)
		{
			// re-seek

			if(fp->mode==MODE_DOPEN)
			{
				dopen_reseek(fp);
			}
			else
			{
				result = sceIoLseek32(fp->fd,fp->fpos,PSP_SEEK_SET);
				if(result != fp->fpos)
				{
					// close fd
					msIoClose(fp);
				}
			}
		}
log_share("%s.re-open '%s' , fd=%X\n",dev_name,fp->name,fp->fd);
	}

	return fp->fd;
}

/////////////////////////////////////////////////////////////////////////////
// 外部参照
/////////////////////////////////////////////////////////////////////////////
int ms_share_get_fd(PspIoDrvFileArg* arg)
{
	int result;

	mss_lock();
	result = ms_share_get_fd_core(arg);
	mss_unlock();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

int ms_share_open_core(int *argv)
{
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)argv[0];
	const char *file = (const char *)argv[1];
	int flags = argv[2];
	SceMode mode = (SceMode)argv[3];

//int ms_share_open(PspIoDrvFileArg *arg,const char *file,int flags,SceMode mode)
//{
	SHARE_FD *fp;
	int result;

//int ms_share_open(PspIoDrvFileArg *arg,const char *file,int flags,SceMode mode)

#ifdef DUMP_FD
Kprintf("ms_share_open(%08X,%s,%08X,%08X)\n",arg,file,flags,mode);
	int i;
Kprintf("mfs.DUMP_FD\n");
	for(i=0;i<MAX_HANDLE;i++)
	{
		if(flash_fd[i].name[0])
Kprintf("%2d:%08X:%02X:%s\n",i,flash_fd[i].fd,flash_fd[i].valid,flash_fd[i].name);
	}

#endif
	mss_lock();
	fp = flash_get_free_fp();
	if(fp)
	{

//		if(mode==0) mode= 0x777;

		// power_lock フラグでエラーになる？
		flags &= ~0x02000000;
/*
auth.dat : f = 04000002,m= 00000000 error , again f = 06000602, m = 000001a4
*/
		// auth.datでエラーになる？
		//flags &= ~0x04000000;

		// cross link error by 'auth.dat','cookie.dat' write
//		if(flags & (PSP_O_CREAT|PSP_O_WRONLY)) flags |= PSP_O_RDONLY;
		if(flags & PSP_O_WRONLY) flags |= PSP_O_RDONLY;

		// hold this handler
		strcpy(fp->name,file);
		fp->valid = 0xff;
		fp->fd    = 0;    // do not close now!
		fp->flags = flags & ~(PSP_O_CREAT|PSP_O_TRUNC|PSP_O_APPEND); // CERATはopen時のみ
		fp->mode  = mode;
		fp->fpos  = 0;

		// open file
		mss_unlock();
		result = (mode==MODE_DOPEN) ? sceIoDopen(file) : IoOpenRetry(file,flags,mode);
		mss_lock();
		// if(result != ERR_UNKNOWN_DEVICE) break;

		if(result > 0)
		{
			// all OK
			arg->arg = (void *)fp;
			// regist fd
			fp->fd = result;
			fp->valid = 0xfe; // close OK

			if(flags & (PSP_O_CREAT|PSP_O_TRUNC) )
			{
				// 新規作成　ｏｒ　trunc時、閉じてsyncする。
//Kprintf("CREAT open %s\n",fp->name);
#if 0
				char dummy[16]={0};
				sceIoWrite(result,dummy,16);
				sceIoLseek32(result,0,PSP_SEEK_SET);
#endif
#if 0
				//
				msIoClose(fp);
#endif
			}
			else
			{
				if(mode!=MODE_DOPEN)
				{
					// APPENDの場合があるのでカレントfposをGET
					fp->fpos = sceIoLseek32(result,0,PSP_SEEK_CUR);
				}
			}
			// result OK
			result = 0;
		}
		else
		{
			// open error
			fp->fd      = -1;
			fp->name[0] = 0; // release handler
		}
	}
	else
		result = ERR_HANDLE_FULL;
	mss_unlock();

//log_share("regist emu %08X, fd = %X\n",(int)fp,fp->fd);

	return result;
}

int ms_share_open(PspIoDrvFileArg *arg,const char *file,int flags,SceMode mode)
{
	int argv[4];

	argv[0] = (int)arg;
	argv[1] = (int)file;
	argv[2] = (int)flags;
	argv[3] = (int)mode;
	return (int)sceKernelExtendKernelStack(STACK_SIZE_OPEN,(void *)ms_share_open_core,(void *)argv);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int ms_share_close(PspIoDrvFileArg *arg)
{
	SHARE_FD *fp = (SHARE_FD *)arg->arg;
	int result;

	result = ms_share_check_fd_core(fp);
	if(result<0) return result;

log_share("ms_share_close fd %d\n",fp->fd);
	mss_lock();

	// close fd when open
	//msIoClose(fp);
	sceKernelExtendKernelStack(STACK_SIZE_CLOSE,(void *)msIoClose,(void *)fp);

	fp->valid   = 0;
	fp->name[0] = 0; // fd開放

	arg->arg = NULL;
	mss_unlock();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// read / write  / seek
/////////////////////////////////////////////////////////////////////////////

int ms_share_read_core(int *argv)
{
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)argv[0];
	void *buf = (void *)argv[1];
	int size = argv[2];
//int ms_share_read(PspIoDrvFileArg *arg,void *buf,int size)
//{

log_share("ms_share_read %08X %08X %08X\n",(int)arg,(int)buf,size);
	mss_lock();
	int result = ms_share_get_fd_core(arg);
	if(result>0)
	{
		result = sceIoRead(result,buf,size);
		// update fpos
		if(result >=0)
		{
			SHARE_FD *fp = (SHARE_FD *)arg->arg;
			fp->fpos += result;
		}
	}
	mss_unlock();

	return result;
}

int ms_share_read(PspIoDrvFileArg *arg,void *buf,int size)
{
	int argv[3];
	argv[0] = (int)arg;
	argv[1] = (int)buf;
	argv[2] = (int)size;
	return (int)sceKernelExtendKernelStack(STACK_SIZE_READ,(void *)ms_share_read_core,(void *)argv);
}

int ms_share_write_core(int *argv)
{
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)argv[0];
	void *buf = (void *)argv[1];
	int size = argv[2];
//int ms_share_write(PspIoDrvFileArg *arg,const void *buf,int size)
//{

log_share("ms_share_write %08X %08X %08X\n",(int)arg,(int)buf,size);

	mss_lock();
	int result = ms_share_get_fd_core(arg);
	if(result>0)
	{
		result = sceIoWrite(result,buf,size);
		// update fpos
		if(result >=0)
		{
			SHARE_FD *fp = (SHARE_FD *)arg->arg;
			fp->fpos += result;
#if 0
			// すぐとじないとflashが壊れる
			msIoClose(fp);
#endif
		}

	}
	mss_unlock();
	return result;
}

int ms_share_write(PspIoDrvFileArg *arg,const void *buf,int size)
{
	int argv[3];
	argv[0] = (int)arg;
	argv[1] = (int)buf;
	argv[2] = (int)size;
	return (int)sceKernelExtendKernelStack(STACK_SIZE_WRITE,(void *)ms_share_write_core,(void *)argv);
}

int ms_share_seek_core(int *argv)
{
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)argv[0];
	SceOff ofs = (u32)argv[1];
	int whence = argv[2];

//SceOff ms_share_seek(PspIoDrvFileArg *arg, u32 unk, SceOff ofs, int whence)
//{
	mss_lock();
	int result = ms_share_get_fd_core(arg);
	if(result>0)
	{
		result = sceIoLseek32(result,ofs,whence);
		if(result >=0)
		{
			SHARE_FD *fp = (SHARE_FD *)arg->arg;
			fp->fpos = result;
		}
		else
			Kprintf("ms_share_seek err %08X\n",result);
	}
	mss_unlock();
	return result;
}

SceOff ms_share_seek(PspIoDrvFileArg *arg, u32 unk, SceOff ofs, int whence)
{
	int argv[3];
	argv[0] = (int)arg;
	argv[1] = (int)ofs;
	argv[2] = (int)whence;
	return (SceOff)sceKernelExtendKernelStack(STACK_SIZE_SEEK,(void *)ms_share_seek_core,(void *)argv);
}

/****************************************************************************
	dread
****************************************************************************/
static int ms_share_dread_core(u32 *argv)
{
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)argv[0];
	SceIoDirent *dir = (SceIoDirent *)argv[1];
	int result;

	result = ms_share_get_fd_core(arg);
	if(result > 0)
	{
		result = sceIoDread(result,dir);
		if(result >=0)
		{
			SHARE_FD *fp = (SHARE_FD *)arg->arg;
			fp->fpos++;
		}
	}
	return result;
}

int ms_share_dread(PspIoDrvFileArg *arg, SceIoDirent *dir)
{
	int argv[2];
	argv[0] = (int)arg;
	argv[1] = (int)dir;

	mss_lock();
	int result = (int)sceKernelExtendKernelStack(STACK_SIZE_DREAD,(void *)ms_share_dread_core,(void *)argv);
	mss_unlock();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int ms_share_flush(PspIoDrvFileArg *arg)
{
	SHARE_FD *fp = (SHARE_FD *)arg->arg;

log_share("ms_share_flush fd %d\n",fp->fd);
	mss_lock();
	// close fd when open
	sceKernelExtendKernelStack(STACK_SIZE_CLOSE,(void *)msIoClose,(void *)fp);
	mss_unlock();
	return 0;
}

#if HOOK_IOCTRL
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int (*fatms_df_IoIoctl)(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);
static int hook_fatms_df_IoIoctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
	Kprintf("hook_fatms_df_IoIoctl(%08X)\n",cmd);
	return fatms_df_IoIoctl(arg,cmd,indata,inlen,outdata,outlen);
}
#endif

/////////////////////////////////////////////////////////////////////////////
// ms flash controll
/////////////////////////////////////////////////////////////////////////////
static int (*fatms_df_open)(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode) = NULL;
#if HOOK_OPEN
static int hook_fatms_df_open(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode)
{
	int result;

	log_open("ms.df_open('%s',%X,%X)\n",file,flags,mode);
	mss_lock();
	result = fatms_df_open(arg,file,flags,mode);

//	if(result==ERR_HANDLE_FULL)
log_open("ms.df_open result %08X\n",result);

	if(result==ERR_HANDLE_FULL)
	{
		// close one mfs file
		// cause GTA hang?
		ms_share_close_one();
		//sceKernelExtendKernelStack(STACK_SIZE_CLOSE1,(void *)ms_share_close_one,(void *)NULL);
		// again
		result = fatms_df_open(arg,file,flags,mode);
	}
	mss_unlock();

	return result;
}
#endif

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#if HOOK_DOPEN
static int (*fatms_df_dopen)(PspIoDrvFileArg *arg, const char *dirname); 
static int hook_fatms_df_dopen(PspIoDrvFileArg *arg, const char *dirname)
{
	int result;

log_open("ms.df_dopen('%s')\n",dirname);
	// prfile_mod.c:df_open_unicode_main:fopen error. 24で枯渇するので
	// 全部閉じる
	mss_lock();
	sceKernelExtendKernelStack(STACK_SIZE_CLOSEA,(void *)ms_share_close_all_core,(void *)0);
	mss_unlock();

	result = fatms_df_dopen(arg,dirname);
	if(result<0)
log_open("ms.df_dopen error %08X\n",result);
	return result;
}
#endif

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#if HOOK_DREAD
static int (*fatms_df_dread)(PspIoDrvFileArg *arg, SceIoDirent *dir);
static int hook_fatms_df_dread(PspIoDrvFileArg *arg, SceIoDirent *dir)
{
	int result;

//	mss_lock();

log_open("ms.df_dread(%X)\n",(int)dir);
	result = fatms_df_dread(arg,dir);

log_open("ms.df_dread result %08X\n",result);

#if 0
	if(result==ERR_HANDLE_FULL)
	{
		// close one mfs file
		//ms_share_close_one();
		sceKernelExtendKernelStack(STACK_SIZE_CLOSE1,(void *)ms_share_close_one,(void *)NULL);
		// again
		result = fatms_df_dread(arg,dir);
	}
	//mss_unlock();
#endif
	return result;
}
#endif

#if HOOK_MKDIR
/////////////////////////////////////////////////////////////////////////////
// mkdir controll
/////////////////////////////////////////////////////////////////////////////
static int (*IoMkdir)(PspIoDrvFileArg *arg, const char *name, SceMode mode); 
static int hook_IoMkdir(PspIoDrvFileArg *arg, const char *name, SceMode mode)
{
	int result = IoMkdir(arg,name,mode);
log_share("ms.IoMkdir:%08X\n",result);
	if(result<0)
	{
		mss_lock();
		sceKernelExtendKernelStack(STACK_SIZE_CLOSE1,(void *)ms_share_close_one,(void *)NULL);
		result = IoMkdir(arg,name,mode);
		mss_unlock();
log_share("ms.IoMkdir retry:%08X\n",result);
	}
	return result;
}
#endif

#if HOOK_UMOUNT
///////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int (*IoUmount)(PspIoDrvFileArg *arg); 
static int hook_IoUmount(PspIoDrvFileArg *arg)
{
	int result;
	Kprintf("ms.IoUmount(%08X)\n",(int)arg);

	ms_share_close_all();

	result = IoUmount(arg);
	return IoUmount(arg);
}
#endif

#if HOOK_DEVCTRL
///////////////////////////////////////////////////////////////////////////
// ioctrl catch remove ms
/////////////////////////////////////////////////////////////////////////////
int (*IoDevctl)(PspIoDrvFileArg *arg, const char *auther,u32 cmd, void *indata, int inlen, void *outdata, int outlen); 
int hook_IoDevctl(PspIoDrvFileArg *arg, const char *auther,u32 cmd, void *indata, int inlen, void *outdata, int outlen)
{
	Kprintf("%s%d.IoDevctl(%08X,%s,%08X,%08X,%06X, %08X,%06X)\n","MS",arg->fs_num,(int)arg,auther,cmd,(int)indata,inlen,(int)outdata,outlen);
//	return IoDevctl(arg,  auther,cmd, indata,  inlen, outdata,  outlen); 

ms_share_close_all();

	int result = IoDevctl(arg,  auther,cmd, indata,  inlen, outdata,  outlen); 
	Kprintf("result %08X\n",result);
	return result;
}
#endif

#if MSSTOR_IOCTRL
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/*

"get medium protect state"

USB connecnt
0210D016 , 0 , 0
0211D817 , 0 , 4
0211D817 , 0 , 4
0211D818 , 4 , 0
0210D017 , 0 , 0
02125008 , 0 , 4 get medium proitect state
02125801 , 0 , 5 device shutdown ?
DevCtrl 02125801
02125803 , 0 , 60
02125006 , 0 , 4

USB disconnect
02125008,0,4 get medium proitect state
0212d817,0,4
DevCtrl 02125802,0,4
DevCtrl 0212D819,0,4
loop;
0210D016,0,0
0211D817,0,4 x 4 or 5

MS access from USB PC

0211D032,0,8000 :access ?
0210D033,0,0    :flash  ?
0211D032,0,BA00 :access ?
0210D033,0,0    :flash  ?

*/

static int (*msstor_df_IoIoctl)(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);
static int hook_msstor_df_IoIoctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
	Kprintf("%s%d.mssotr_IoIoctl(%08X,%08X,%08X,%06X, %08X,%06X)\n","MS",arg->fs_num,(int)arg,cmd,(int)indata,inlen,(int)outdata,outlen);

	switch(cmd)
	{
	case 0x0210D016:
Kprintf("LOCKOUT MS\n");
		mss_lock();
		ms_share_close_all_core(0);
		mss_unlock();
		break;

#if 0
	case 0x02125801:
Kprintf("LOCKOUT MS\n");
		mss_lock();
		ms_share_close_all_core(0);
		mss_unlock();
		break;
#endif
	}
	return msstor_df_IoIoctl(arg,cmd,indata,inlen,outdata,outlen);
}
#endif

#if MSSTOR_DEVCTRL
/////////////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////////////
int (*mssotr_IoDevctl)(PspIoDrvFileArg *arg, const char *auther,u32 cmd, void *indata, int inlen, void *outdata, int outlen); 
int hook_mssotr_IoDevctl(PspIoDrvFileArg *arg, const char *auther,u32 cmd, void *indata, int inlen, void *outdata, int outlen)
{
/*
起動時、USB切断、MS挿入、resume
-MS0.mssotr_IoDevctl(88051B50,,02125802,00000000,000000, 882FF164,000004)
-result 00000000:40_00_00_00

  1GB:0x00000040(ProDuo?)
256MB:0x00000040(ProDuo?)
 32MB:0x00000010(Duo?)

-MS0.mssotr_IoDevctl(88051B50,,0212D819,00000000,000000, 882FF168,000004)
-result 00000000:0F_00_00_00

  1GB:0x0000000F
256MB:0x0000000F
 32MB:0x0000000F

USB接続
-MS0.mssotr_IoDevctl(88051DF8,02125801,00000000,000000, 882F94B0,000005)
-result 00000000:01_00_00_01_00

*/
	int result;
	Kprintf("%s%d.mssotr_IoDevctl(%08X,%s,%08X,%08X,%06X, %08X,%06X)\n","MS",arg->fs_num,(int)arg,auther,cmd,(int)indata,inlen,(int)outdata,outlen);

#if 0
	switch(cmd)
	{
	case 0x02125801:
Kprintf("LOCKOUT MS\n");
		sceKernelDelayThread(1000000); // wati for MS finish
		mss_lock();
		ms_share_close_all_core(0);
		break;
	case 0x02125802:
Kprintf("RECONNECT MS\n");
		mss_unlock();
		break;
	}
#endif

	result = mssotr_IoDevctl(arg,  auther,cmd, indata,  inlen, outdata,  outlen); 
	Kprintf("result %08X:%02X_%02X_%02X_%02X_%02X\n",result,((unsigned char *)outdata)[0],((unsigned char *)outdata)[1],((unsigned char *)outdata)[2],((unsigned char *)outdata)[3],((unsigned char *)outdata)[4]);
	return result;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// install 
/////////////////////////////////////////////////////////////////////////////

static const char fatms[] = "fatms";
static const char fatms2[] = "FATMS";

int ms_share_install(void)
{
	// hook ms open
	PspIoDrv *ms_dev;

	// installed
	if(share_semaid) return 0;

	// 2.60
	ms_dev = search_device_io(fatms,0x800,0x000a0010,fatms2);
	if(ms_dev==NULL)
	{
		ms_dev = search_device_io(fatms,0x800,0x10,fatms2);
	}

	// install
	if(ms_dev)
	{
#if HOOK_OPEN
		fatms_df_open = ms_dev->funcs->IoOpen;
		ms_dev->funcs->IoOpen = hook_fatms_df_open;
#endif
#if HOOK_DOPEN
		fatms_df_dopen = ms_dev->funcs->IoDopen;
		ms_dev->funcs->IoDopen = hook_fatms_df_dopen;
#endif
#if HOOK_DREAD
		fatms_df_dread = ms_dev->funcs->IoDread;
		ms_dev->funcs->IoDread = hook_fatms_df_dread;
#endif
#if HOOK_IOCTRL
		fatms_df_IoIoctl = ms_dev->funcs->IoIoctl;
		ms_dev->funcs->IoIoctl = hook_fatms_df_IoIoctl;
#endif
#if HOOK_MKDIR
		IoMkdir = ms_dev->funcs->IoMkdir;
		ms_dev->funcs->IoMkdir = hook_IoMkdir;
#endif
#if HOOK_UMOUNT
		IoUmount = ms_dev->funcs->IoUmount;
		ms_dev->funcs->IoUmount = hook_IoUmount;
#endif
#if HOOK_DEVCTRL
		IoDevctl = ms_dev->funcs->IoDevctl;
		ms_dev->funcs->IoDevctl = hook_IoDevctl;
#endif

#if MSSTOR_DEVCTRL || MSSTOR_IOCTRL
/*
	msstorのフック
;IoAdd
  dw 00008478 // "msstor"
  dw 00000004
  dw 00000000
  dw 00008480 // "MSstor whole dev"
  dw 0000009c
;
;IoAdd
  dw 00008494 // "msstor0p"
  dw 00000004
  dw 00000000
  dw 000084a0 // "Msstor partation #1"
  dw 000000F4
*/
	ms_dev = search_device_io("msstor",0x00000000,0x00000004,"MSstor whole dev");
//	ms_dev = search_device_io("msstor0p",0x00000000,0x00000004,"Msstor partation #1");
	if(ms_dev)
	{
#if MSSTOR_DEVCTRL
		mssotr_IoDevctl = ms_dev->funcs->IoDevctl;
		ms_dev->funcs->IoDevctl = hook_mssotr_IoDevctl;
#endif
#if MSSTOR_IOCTRL
		msstor_df_IoIoctl = ms_dev->funcs->IoIoctl;
		ms_dev->funcs->IoIoctl = hook_msstor_df_IoIoctl;
#endif
	}
#endif
		// create semaphore
		//sema_lock = 0;
		share_semaid = sceKernelCreateSema("msShare", 0, 1, 1, 0);

//Kprintf("hook fatms.df_open\n");
		clear_cache();
		return 0;
	}

	Kprintf("Can't hook fatms device\n");
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void ms_share_close_all(void)
{
log_share("ms_share_close_all\n");
	if(share_semaid==0) return;

	mss_lock();
	ms_share_close_all_core(0);
	mss_unlock();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void ms_share_shutdown(void)
{
	PspIoDrv *ms_dev;

//log_share("ms_share_shutdown\n");

	if(share_semaid==0) return;

	mss_lock();

	// close all handle
	ms_share_close_all_core(1);

	// stop hook ntry
	// 2.60
	if(fatms_df_open)
	{
		ms_dev = search_device_io(fatms,0x800,0x000a0010,fatms2);
		if(ms_dev==NULL)
		{
			ms_dev = search_device_io(fatms,0x800,0x10,fatms2);
		}
		if(ms_dev)
		{
#if HOOK_OPEN
			ms_dev->funcs->IoOpen = fatms_df_open;
#endif
#if HOOK_DOPEN
			ms_dev->funcs->IoDopen = fatms_df_dopen;
#endif
#if HOOK_DREAD
			ms_dev->funcs->IoDread = fatms_df_dread;
#endif
#if HOOK_IOCTRL
			ms_dev->funcs->IoIoctl = fatms_df_IoIoctl;
#endif
#if HOOK_MKDIR
			ms_dev->funcs->IoMkdir = IoMkdir;
#endif
#if HOOK_UMOUNT
			ms_dev->funcs->IoUmount = IoUmount;
#endif
#if HOOK_DEVCTRL
			ms_dev->funcs->IoDevctl = IoDevctl;
#endif
			//Kprintf("hook fatms.df_open\n");
			clear_cache();
		}
	}
	mss_unlock();

	// セマフォ解放
	if(share_semaid)
	{
		sceKernelDeleteSema(share_semaid);
		share_semaid = 0;
	}

}

