/*
  PSP plain ISO9660 plugin for devhook 0.42
*/

#include "common.h"

// log output switch
#define OUT_LOG_ISO 0

// thread priority in read
#define READ_PRIORITY 47

/////////////////////////////////////////////////////////////////////////////
#if OUT_LOG_ISO
#define log_iso Kprintf
#else
#define log_iso(text,...) {}
#endif

/////////////////////////////////////////////////////////////////////////////

// number of sectors
static int max_sectors;

static DH_FILE iso_fd =
{
	NULL,			// path
	PSP_O_RDONLY,	// flags
	0x777,			// mode
//
	-1,				// current fd
	10,				// retry count    = 10
	100000,			// retry interval = 100msec
	0				// stack no-extend
};

/////////////////////////////////////////////////////////////////////////////


/**************************************
	seek
**************************************/
static int iso_seek(int cur_pos,int sector_pos,int whence)
{
	int result;

	if(iso_fd.fd < 0) return iso_fd.fd;

	if(whence==PSP_SEEK_CUR)		sector_pos += cur_pos;
	else if(whence==PSP_SEEK_END)	sector_pos = max_sectors - sector_pos;

	// limit MAX sector
	if(sector_pos > max_sectors) sector_pos = max_sectors;
	result = sector_pos;

//log_iso("ISO SEEK pos = %08X\n",result);

	return result;
}

/**************************************
	iso read
**************************************/
static int iso_read(void *buf,int lba,int size)
{
	int result;

#ifdef READ_PRIORITY
	int cur_prio = sceKernelGetThreadCurrentPriority();
	sceKernelChangeThreadPriority(0,READ_PRIORITY);
#endif
	result = dhReadFileRetry(&iso_fd,lba * 0x800,buf,size);
#ifdef READ_PRIORITY
	sceKernelChangeThreadPriority(0,cur_prio);
#endif
log_iso("ISO READ POS=0x%08X BUF=0x%08X SIZE=0x%08X : result 0x%08X\n",lba*0x800,(int)buf,size,result);
	return result;
}

/**************************************
**************************************/
static int iso_mount(const char *path)
{
	iso_fd.path = path;
	unsigned char lba_buf[8];
	unsigned char pvd_sig[6];

	u32 lba_max,lba_max2;
	u32 file_size;

	if(
		(dhReadFileRetry(&iso_fd,0x8001,pvd_sig,5) == 5) && 
		(dhReadFileRetry(&iso_fd,0x8050,lba_buf,8) == 8)
	)
	{
		lba_max   = lba_buf[0] | (lba_buf[1]<<8) | (lba_buf[2]<<16) | (lba_buf[3]<<24);
		lba_max2  = lba_buf[7] | (lba_buf[6]<<8) | (lba_buf[5]<<16) | (lba_buf[4]<<24);
		file_size = sceIoLseek32(iso_fd.fd,0,PSP_SEEK_END);
		if(
			!memcmp(pvd_sig,"CD001",5) &&    // PVD SIG
//			(lba_max*0x800 == file_size) &&  // PVD MAX LBA , file size
			((file_size&0x7ff)== 0    ) &&  // file size
			(lba_max       == lba_max2)      // PVD MAX LBA LittleEndian , Bigendian
		){
			max_sectors = file_size / 0x800;
log_iso("ISO mount OK : MAX  LBA PVD/FILE =%08X / %08X\n",lba_max,max_sectors);
			return 0;
		}
	}
log_iso("ISO mount NG\n");
	return 0x80000000;
}

/**************************************
**************************************/
static int iso_unmount(void)
{
	dhCloseFile(&iso_fd);
	return 0;
}

/**************************************
**************************************/
static int iso_getcapacity(void)
{
	return max_sectors;
}

/**************************************
**************************************/
const DH_UMD_PLUGIN iso_plugin =
{
	"ISO9660file",
	iso_mount,
	iso_unmount,
	iso_getcapacity,
	iso_read
};

