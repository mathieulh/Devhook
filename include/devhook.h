#ifndef __DEVHOOK_H__
/****************************************************************************
	devhook SDK header file version 0.42c
****************************************************************************/

/* get devhook driver version code */
/* 0.42b == 0x00420002 */
/* 0.43  == 0x00430000 */
unsigned int dhGetVersion(void);

/****************************************************************************
	setting registry
****************************************************************************/

int dhGetRegistry(const char *name,void *data,int size);
int dhSetRegistry(const char *name,const void *data,int size);

/****************************************************************************
	UMD controll
****************************************************************************/

int dhUMDMount(const char *path);
int dhUMDUnmount(void);

/****************************************************************************
	UMD emulator plugin

note:
	plugin which can be added is 8 devices.
	device which was registed last is called first.
	wehen return Mount callback
	If Mount callback returns less than 0, next device are called.
	It is Unmounted until it is inserted when MemoryStick is pulled out.

	see umd_iso.c for template.
****************************************************************************/

// plugin interface
typedef struct devhook_umd_plugin
{
	const char *devname;
	int (*Mount)(const char *path);                        // unmount event callback
	int (*Unmount)(void);                                  // mount event callback
	int (*GetCapacity)(void);                              // get number of sectors
	int (*Read)(void *buf,int top_sector,int num_bytes);   // read request
}DH_UMD_PLUGIN;

/*************************************
	add UMD emulator plugin
*************************************/
int dhUMDAddDevice(const DH_UMD_PLUGIN *interface);

/*************************************
	delete UMD emulator plugin
*************************************/
int dhUMDDelDevice(const DH_UMD_PLUGIN *interface);

/****************************************************************************
	usualy file access
note:
	file access with auto open/reopen/retry and stack extend.
****************************************************************************/
typedef struct dh_fileio_struct
{
	const char *path;		// file path      : arg1 of sceIoOpen
	int flags;				// file flag mode : arg2 of sceIoOpen
	SceMode mode;			// file open mode : arg3 of sceIoOpen
//
	SceUID fd;				// file dsscripter number
	int retry_cnt;			// retry counter
	int retry_interval;		// sleep time if error retry
	int stack_size;			// number of extended stack size,0 is no extend stack
}DH_FILE;

/*************************************
read with auto open/retry
*************************************/
int dhReadFileRetry(DH_FILE *fp,SceOff pos,void *buf,int size);

/*************************************
write with auto open/retry
*************************************/
int dhWriteFileRetry(DH_FILE *fp,SceOff pos,void *buf,int size);

/*************************************
close 
*************************************/
int dhCloseFile(DH_FILE *fp);

/****************************************************************************
	CLOCK controll
****************************************************************************/
int dhCLKSet(int cpuclk,int busclk);

#endif
