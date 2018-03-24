#ifndef __HOOK_DEV__
#define __HOOK_DEV__

typedef PspIoDrvFuncs IO_DEVICE_CTRL_ENTRY;

// search Device
PspIoDrv *search_device_io(const char *dev_name,int block_size,int flags,const char *drv_name);

// hook Device funcs
// return : NULL = error , other = original table
PspIoDrvFuncs *hook_device(const char *dev_name,int block_size,int flags,const char *drv_name,IO_DEVICE_CTRL_ENTRY *entry_table);

// search BLK controll block (UMD9660 special)
unsigned int *search_umd9660_blk_entry(void);

#endif
