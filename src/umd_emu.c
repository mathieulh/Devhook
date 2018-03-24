/*
  PSP UMD emulation
*/
#include "common.h"

extern int sceKernelNotifyCallback(int cbid,int event);

#define DEV_NAME "UMDEMU"

#define MS_CALLBACK 0

#define __VERBOSE__ 0

/* ログ切り換え */
#define OUT_LOG_DEV 0
#define OUT_LOG_EVT 1


// show DISK INFO
//#define LOG_INFO

/* モニタモード */
#define MONITOR_MODE 0

/* スタック増加 */
#define IOCTRL_ADD_STACK    1

/////////////////////////////////////////////////////////////////////////////
#if OUT_LOG_DEV
#define log_dev Kprintf
#else
#define log_dev(text,...) {}
#endif

#if OUT_LOG_EVT
#define log_evt Kprintf
#else
#define log_evt(text,...) {}
#endif

/****************************************************************************
	UMD emu device entry
****************************************************************************/

static int (*UmdUnMount)(void);
static int (*UmdGetCapacity)(void);
static int (*UmdRead)(void *buf,int sector,int size);

#define is_mounted() (UmdUnMount)

#define MAX_PLUGIN 8
static int num_emu_dev;
// static const DH_UMD_PLUGIN *cur_dev;
static const DH_UMD_PLUGIN *emu_dev[MAX_PLUGIN];

char cur_umd_path[128];

/****************************************************************************
****************************************************************************/

static int initial_pending;
static int remount_timer;

/* UMD sector buffer */
static unsigned char iso_sector_buf[0x800] __attribute__((aligned(64)));
static unsigned int iso_cur_sector = -1;

///////////////////////////////////////////////////////////////////
// emulation work
///////////////////////////////////////////////////////////////////

// BLOCK MODE handle table
#define MAX_UMD_FP 8
typedef struct emu_state{
  int fd;
  int pos;
}UMD_EMU_STATE;
static UMD_EMU_STATE emu_state[MAX_UMD_FP];

// original IOCTRL entry table
IO_DEVICE_CTRL_ENTRY *umd_ioctrl_orig_table = 0;

// semaphore for UMD ISO access
static int umd_semaid;

///////////////////////////////////////////////////////////////////
//  umd enulation dummy callback
///////////////////////////////////////////////////////////////////
static int dummy_UmdGetCapacity(void)
{
	Kprintf("Unmountred UmdGetCapacity called\n");
	return 0;
//	return 0x80010163;
}

static int dummy_UmdRead(void *buf,int sector,int size)
{
	Kprintf("Unmountred UmdRead called\n");
	return 0x80010163;
}

///////////////////////////////////////////////////////////////////
//  semaphore
///////////////////////////////////////////////////////////////////
void lock_umde(void)
{
	sceKernelWaitSema(umd_semaid, 1, 0);
}
void unlock_umde(void)
{
	sceKernelSignalSema(umd_semaid, 1);
}

/**************************************
	read in sector buffer
**************************************/
static int UmdRead_secbuf(int lba)
{
	int result;
	if(iso_cur_sector != lba)
	{
		result = UmdRead(iso_sector_buf,lba,0x800);
		if(result < 0 ) return result;
		//if(result != 0x800) return 0x80010163;
	}
	return 0;
}

/******************************************************************************

	UMD access RAW routine

	lba_param[0] = 0 , unknown
	lba_param[1] = cmd,3 = ctrl-area , 0 = data-read
	lba_param[2] = top of LBA
	lba_param[3] = total LBA size
	lba_param[4] = total byte size
	lba_param[5] = byte size of center LBA
	lba_param[6] = byte size of start  LBA
	lba_param[7] = byte size of last   LBA

******************************************************************************/
int umd_read_block(void *drvState,unsigned char *buf,int read_size,u32 *lba_param)
{
	int result;
	int fpos;
	int size;
	int boffs , bsize;
	int lba;

    // size
	size = lba_param[4];

    // LBA -> offset
    fpos = lba_param[2]*0x800;

    // セクタ内、オフセット
	if( lba_param[6] && (lba_param[5] || lba_param[7]))
	{
		// セクタをまたぐ時だけ、Tはのこりバイト数となる。
		boffs = 0x800-lba_param[6];
	}
	else
	{
		// １セクタ内の終結の時はTはオフセット位置
		boffs = lba_param[6];
	}
	fpos += boffs;
	lba  = lba_param[2];

log_dev("READ UMD[%X,%X,LBA:%08X,NUM:%06X,LEN:%08X(C:%06X,T:%03X,B:%03X)]\n"
	,lba_param[0],lba_param[1],lba_param[2],lba_param[3],lba_param[4],lba_param[5],lba_param[6],lba_param[7]);

	// read 1st sector
	if(boffs > 0)
	{
		bsize = 0x800 - boffs;
		if(bsize > size) bsize = size;

		if(bsize>0)
		{
			// read
			result = UmdRead_secbuf(lba);
			if(result < 0 ) return result;

			// copy
			memcpy(buf,&iso_sector_buf[boffs],bsize);
			buf  += bsize;
			size -= bsize;
			lba++;
		}
	}
	// centor sector
//	if(lba_param[5])
#if 0
	// emulate UMD seek !
	if(size>=0x800)
		sceKernelDelayThread(20000);
#endif

#if 1
	int burst_size = size & 0xfffff800;
	if(burst_size)
	{
		result = UmdRead(buf,lba,burst_size);
		if(result < 0 ) return result;
//		if(result != 0x800) return 0x80010163;
		buf  += burst_size;
		size -= burst_size;
		lba  += burst_size/0x800;
	}
#else
	while(size>=0x800)
	{
		result = UmdRead(buf,lba,0x800);
		if(result < 0 ) return result;
//		if(result != 0x800) return 0x80010163;
		buf  += 0x800;
		size -= 0x800;
		lba  ++;
	}
#endif

	// lat sector
//	if(lba_param[7])
	if(size>0)
	{
		// read
		result = UmdRead_secbuf(lba);
		if(result < 0 ) return result;

		// copy
		memcpy(buf,iso_sector_buf,size);
		//odata += bsize;
		//size -= bsize;
		//lba++
	}
	return 0;
}

#if 0
/****************************************************************************

int sceUmdMan_driver_4fb913a3(void);

sceAta_driver_1c29566b();をコールするだけ

ATAドライブイベントが返り、Ready/Ejectの処理を行う

0x80 : Disc Inserted (to read PVD)
0x40 : 
0x20 : DiscReady (Readable)
0x01 : DiscOut
0x02 : DiscIn (not ready)

****************************************************************************/
int hook_sceUmdMan_driver_4fb913a3(void)
{
	int status;

	status = sceUmdGetDriveStatus();

Kprintf("Sts %02X\n",status);

	if(status & 0x20) return 0x00; // eventなし
	if(status & 0x01) return 0x02; // eventなし
	if(status & 0x10) return 0x60; // READ PVS + READABLE

//	log_evt("ATA EVT STS=%02X:result= %02X\n",status,result);

	return 0x00;
}
#endif

/*****************************************************************************
call InsertEject callback
// FW1.50 : umdman.c : L0000A6A4
// FW2.00 : umdman.c : L0000A9B4

ここをコールバックしないと、どうにもならない

*****************************************************************************/
static void (*InsertEjectCallback)(int,void *) = 0;
static int InsertEjectCallback_sts;
static void *InsertEjectCallback_arg;

#if 1
/* for cactch sceIsofsIntrRegisterMediumCallBack */

int hook_sceUmdMan_driver_bf8aed79(int status,void *entry,void *arg1)
{
#if 1
	// fw1.5
	Kprintf("sceUmdMan_driver_bf8aed79(%08X,%08X)\n",status,(int)entry);
	InsertEjectCallback_sts = 0x20;
	InsertEjectCallback     = (void *)status;
	InsertEjectCallback_arg = (void *)entry;
#else
	// fw 2.0
	Kprintf("sceUmdMan_driver_bf8aed79(%02X,%08X,%08X)\n",status,(int)entry,(int)arg1);
	InsertEjectCallback_sts = status;
	InsertEjectCallback = entry;
	InsertEjectCallback_arg = arg1;
#endif
	return 0;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// DriveStatus Hook
/////////////////////////////////////////////////////////////////////////////

#if 0
/* MeidaType Format */
#define SCE_UMD_FMT_UNKNOWN   0x00000 /* UNKNOWN */
#define SCE_UMD_FMT_GAME      0x00010 /* GAME */
#define SCE_UMD_FMT_VIDEO     0x00020 /* VIDEO */
#define SCE_UMD_FMT_AUDIO     0x00040 /* AUDIO */
#define SCE_UMD_FMT_CLEAN     0x00080 /* CLEANNING */
#endif

int hook_sceUmdGetDiscInfo(unsigned int *p)
{
  if(p[0]!=8) return 0x80010016;
  p[1] = 0x10; // GAME
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// UnMount ISO file
/////////////////////////////////////////////////////////////////////////////
/*
#define SCE_UMD_INIT				(0x00)
#define SCE_UMD_MEDIA_OUT			(0x01)
#define SCE_UMD_MEDIA_IN			(0x02)
#define SCE_UMD_MEDIA_CHG			(0x04)
#define SCE_UMD_NOT_READY			(0x08)
#define SCE_UMD_READY				(0x10)
#define SCE_UMD_READABLE			(0x20)

#define SCE_UMD_MODE_POWERON		(0x01)
#define SCE_UMD_MODE_POWERCUR		(0x02)
*/


// set ATA Event Flag
extern void sceAta_driver_7f551d66(int);

extern int SysMemForKernel_6d8e0cdf(u8 *buf); // get UMD ID
extern int SysMemForKernel_c7e57b9c(const u8 *buf); // set UMD ID

static const u8 dummy_umd_id[16] = {
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

extern int sceKernelCallSubIntrHandler(int,int,int,int);

int unmount_umd_core(void)
{
	if(is_mounted())
	{

		log_evt("unmount UMD\n");

		// unmount emu device
		UmdUnMount();

		// unmount emu device
		UmdUnMount     = NULL;
		UmdGetCapacity = dummy_UmdGetCapacity;
		UmdRead        = dummy_UmdRead;

		//  IRQ for GPIO emu
		sceKernelCallSubIntrHandler(0x04,0x1a,0,0);

		sceKernelDelayThread(50000);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int dhUMDUnmount(void)
{
	int result;
	lock_umde();
	result = unmount_umd_core();
	unlock_umde();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
// Mount ISO file
/////////////////////////////////////////////////////////////////////////////

int dhUMDMount(const char *umd_path)
{
	int status;
	int result = -1;
	int i;

	// Lock
	lock_umde();

	// status before unmount
	status = sceUmdGetDriveStatus();

	unmount_umd_core();

	// set new ISO name
	if(umd_path && strcmp(umd_path,cur_umd_path) )
	{
		strcpy(cur_umd_path,umd_path);
		// for next reboot
		dhSetRegistry("UMD_PATH",cur_umd_path,strlen(cur_umd_path)+1);

		// clear prevous UMD-ID for reload PVD
		SysMemForKernel_c7e57b9c(dummy_umd_id);

		// hook if real UMD > mount UMD
		hook_umd9660_driver();
	}

	if(is_real_umd())
		result = 0;
	else
	{
		// re-mount emulation
		UmdUnMount = NULL;
		sceKernelCallSubIntrHandler(0x04,0x1a,0,0);
		sceKernelDelayThread(50000);

		// try & select pulugin emulator
		for(i=num_emu_dev-1;i>=0;i--)
		{
			result = (emu_dev[i]->Mount)(cur_umd_path);
			if(result>=0)
			{
				UmdUnMount     = emu_dev[i]->Unmount;
				UmdGetCapacity = emu_dev[i]->GetCapacity;
				UmdRead        = emu_dev[i]->Read;
				result = 0;

log_evt("mounted UMD '%s' with '%s'\n",cur_umd_path,emu_dev[i]->devname);

				break;
			}
		}
	}

	// UnLock
	unlock_umde();

	if(result==0)
	{
		// NOUMD LUMINUSで0x8001007bエラーになる
		// IRQ for EJECT SW change
		sceKernelCallSubIntrHandler(0x04,0x1a,0,0);

		// initial holdとマウントタイマをクリアする
		remount_timer = 0;
		initial_pending = 0;
	}
	return result;
}

#if MS_CALLBACK
/////////////////////////////////////////////////////////////////////////////
// MS挿抜監視用callback & thread
/////////////////////////////////////////////////////////////////////////////
/*	ms media callback */
static int media_callback(int count, int arg, void *common)
{
	Kprintf("media_callback(%X,%X,%X)\n",count,arg,(int)common);

	switch(arg)
	{
	case 1: // a class driver may ready
		break;
	case 4: // media insertion detected
		break;

	case 2: // the class driver removed
	case 8: // media removal detected
		//ms_share_close_all();

		dhUMDUnmount();
		// 1sec re-mount timer
		remount_timer = 1000000;
		break;
	}
	return 0;
}

static int media_cbthread(int size, void * opt)
{
/*
	callbackでremoveポイントを捕まえる
*/
	SceUID uidCB;
	int ret;

Kprintf("media_cbthread()\n");

	uidCB = sceKernelCreateCallback("media_callback", media_callback, NULL);
	ret = sceMScmRegisterNotifyCallback(uidCB);
	if (ret < 0)
	{
		Kprintf("media_callback registey failed\n");
		//return -1;
	}
	// ずっと監視のみを行う
	while(1)
	{
		sceKernelDelayThreadCB(100000);
		// 100msecごとの処理．．．
	}
#if 0
	ret = sceMScmUnRegisterNotifyCallback(uidCB);
	sceKernelDeleteCallback(uidCB);
	sceKernelExitDeleteThread(0);
#endif
	return 0;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// MS validate check
/////////////////////////////////////////////////////////////////////////////
static int is_ms_mount(void)
{
	int result;
	u32 status;
	result = sceIoDevctl("mscmhc0:",0x02025801,0,0,&status, sizeof(status));
	if(result<0) return result;
	return status==4 ? 1 : 0;
}

/////////////////////////////////////////////////////////////////////////////
// initialize emulation unit
/////////////////////////////////////////////////////////////////////////////
extern u32 sceGpioPortRead(void);

int umdemu_walker(u32 tickusec)
{
	// Bypass if Real UMD
	if(is_real_umd()) return 0;

	// initial & mount retry
	if(is_ms_mount())
	{
		// MS mounted
		if(remount_timer>0)
		{
			remount_timer -= tickusec;
			dhUMDMount(NULL);
		}
	}
	else
	{
		// MS unmounted
		if(is_mounted())
		{
			dhUMDUnmount();
			// 1sec re-mount timer
			remount_timer = 1000000;
#if 0
			// if close ms handles here, "net/http/auth" are broken
			ms_share_close_all();
#endif
log_evt("MS removed\n");
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Disc Info
/////////////////////////////////////////////////////////////////////////////

/*
DEMO Vol.2

-UMD Info
-00:00000000 00000000 00000000 00000000
-10:00000000 00000000 00000000 00000000
-20:00000000 00000000 00000000 00000000
-30:00000000 00000000 00000000 00000000
-40:00000000 00000000 00000000 00000000
-50:00000000 00000000 00000000 00000000
-60:00000000 E0010800 00000001 00000009
-70:0001FBCF 000251BF 0009C0BF 00F6952F
-80:00000001 00000001

天地体験

-UMD Info
-00:00000000 00000000 00000000 00000000
-10:00000000 00000000 00000000 00000000
-20:00000000 00000000 00000000 00000000
-30:00000000 00000000 00000000 00000000
-40:00000000 00000000 00000000 00000000
-50:00000000 00000000 00000000 00000000
-60:00000000 E0000800 00000000 00000000
-70:0002DF3F 0002DF3F 0005DF3F 0005DF3F
-80:00000001 00000001

0x08 : 0=ready , 1=error

*/

extern int sceUmdManGetDiscInfo(int *ptr);
extern int *sceUmdMan_driver_Unkonow_e192c10a(void);
int *hook_sceUmdMan_driver_Unkonow_e192c10a(void)
{
	int *lp = sceUmdMan_driver_Unkonow_e192c10a();

#ifdef LOG_INFO
	Kprintf("call sceUmdMan_driver_Unkonow_e192c10a() from UMD9660:%08X\n",(int)lp);
	int i;
	Kprintf("UMD Info\n");
	for(i=0;i<0x90/4;i+=4)
		Kprintf("%02X:%08X %08X %08X %08X\n",i*4,lp[i],lp[i+1],lp[i+2],lp[i+3]);
//	Kprintf("%02X:%08X %08X\n",i*4,lp[i],lp[i+1]);

	int param[2],result;
	param[0] = 0;
	param[1] = 0;
	result = sceUmdManGetDiscInfo(param);
	Kprintf("sceUmdManGetDiscInfo %08X:[%08X %08X]\n",result,param[0],param[1]);
#endif

// patch DISK INFO

// Kprintf("ST:%08X %08X %08X %08X\n",lp[0x64/4],lp[0x6c/4],lp[0x70/4],lp[0x74/4]);
// -ST:E0000800 00000000 0002DF3F 0002DF3F GAME
// -ST:E0010800 00000009 0001FBCF 000251BF demo vol.2
	if(!is_real_umd())
	{
		int result;

		lock_umde();
		result = UmdGetCapacity();
		unlock_umde();

		if( result > 0)
		{
//Kprintf("Patch Info\n");
			lp[0x64/4] = 0xe0000800;        // 00
			lp[0x68/4] = 0;                 // ?
			lp[0x6c/4] = 0x00000000;        // 08
			lp[0x70/4] = result;            // 1C
			lp[0x74/4] = result;            // 24
			lp[0x80/4] = 1;                 // ?
			lp[0x84/4] = 1;                 // ?
		}
		else
		{
Kprintf("Can't Patch DiscInfo\n");
		}
	}
	return lp;
}

int hook_sceUmdManGetDiscInfo(int *param)
{
// Demo == 0x30
// GAME == 0x10
	param[1] = 0x10;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// bit26がDISC SW
/////////////////////////////////////////////////////////////////////////////
u32 hook_sceGpioPortRead(void)
{
	u32 gpio = *(u32 *)0xbe240004;

	if(!is_mounted() && (initial_pending==0) )
	{
		// HOLD UMD no insert
		gpio &= ~0x04000000;
//Kprintf("GPIO DISCOUT\n");
	}
	return gpio;
}

/////////////////////////////////////////////////////////////////////////////
// plugin
/////////////////////////////////////////////////////////////////////////////
int dhUMDAddDevice(const DH_UMD_PLUGIN *interface)
{
	if(num_emu_dev >= MAX_PLUGIN)
		return -1;
	emu_dev[num_emu_dev++] = interface;
	return 0;
}

int dhUMDDelDevice(const DH_UMD_PLUGIN *interface)
{
	int i;
	
	for(i=0;i<num_emu_dev;i++)
	{
		if(emu_dev[i]==interface)
		{
			num_emu_dev--;
			for(;i<num_emu_dev;i++)
				emu_dev[i] = emu_dev[i]+1;
			return 0;
		}
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
// reset emulation unit
/////////////////////////////////////////////////////////////////////////////
int umdemu_reset(void)
{
	cur_umd_path[0] = 0x00;
	dhGetRegistry("UMD_PATH",cur_umd_path,sizeof(cur_umd_path));

	if(!is_real_umd())
	{
		remount_timer = 2000000;
		initial_pending = 1;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// initialize emulation unit
/////////////////////////////////////////////////////////////////////////////
int umdemu_init(void)
{
	int i;

//Kprintf("umdemu_init()\n");

	for(i=0;i<MAX_UMD_FP;i++)
	{
		emu_state[i].fd  = 0;
		emu_state[i].pos = 0;
	}

//Kprintf("create UMDEMU semaphore\n");
	// create semaphore
	umd_semaid = sceKernelCreateSema("UMDEMU", 0, 1, 1, 0);
//Kprintf("create UMDEMU semaphore OK\n");

	remount_timer = 0;
	initial_pending = 0;

	//
	UmdUnMount     = NULL;
	UmdGetCapacity = dummy_UmdGetCapacity;
	UmdRead        = dummy_UmdRead;

	num_emu_dev = 0;

#if MS_CALLBACK
	// ms event callback
	int tid = sceKernelCreateThread("dh_mediacheck", media_cbthread, 111, 1024 * 16, 0, 0);
	if (tid > 0)
	{
		sceKernelStartThread(tid, 0, NULL);
		// sceKernelWaitThreadEndCB(tid, NULL);
	}
#endif

	return 0;
}
