#ifndef __HOOK_UMD_H__

extern IO_DEVICE_CTRL_ENTRY umd_ioctrl_hook_table;
extern IO_DEVICE_CTRL_ENTRY *umd_ioctrl_orig_table;

// Event 
extern void (*umdManCauseInsertEjectCallback)(void);

/* hook func */
int hook_sceUmdActivate(int unit,char *dev_name);
int hook_sceUmdDeactivate(int unit,char *dev_name);
//int hook_sceUmdRegisterUMDCallBack(int cbid);
int hook_sceUmdGetDiscInfo(unsigned int *p);

extern void (*umdManCauseInsertEjectCallback)(void);

void UmdMountCallback(int event,void *args);
void UmdUnMountCallback(int event,void *args);
int hook_sceUmdMan_driver_4fb913a3(void);
int hook_sceUmdMan_driver_bf8aed79(int status,void *entry,void *args);

// Mount
int dhUMDUnmount(void);
int dhUMDRemount(void);
int dhUMDMount(const char *umd_iso_name);
int umdemu_walker(u32 tickusec);

int umdemu_init(void);
int umdemu_reset(void);

int umd_read_block(void *drvState,unsigned char *buf,int read_size,u32 *lba_param);

int *hook_sceUmdMan_driver_Unkonow_e192c10a(void);
int hook_sceUmdManGetDiscInfo(int *param);

u32 hook_sceGpioPortRead(void);
u32 hook_sceGpioPortRead2(void);

// plugin
int dhUMDAddDevice(const DH_UMD_PLUGIN *interface);
int dhUMDDelDevice(const DH_UMD_PLUGIN *interface);

// Status
extern char cur_umd_path[128];
#define is_real_umd() (cur_umd_path[0]==0)

#endif
