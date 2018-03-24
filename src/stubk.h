#if 1

/////////////////////////////////////////////////////////////////////////////
// IoFileMgrForKernel
/////////////////////////////////////////////////////////////////////////////
SceUID KsceIoOpen(const char *file, int flags, SceMode mode);
int KsceIoClose(SceUID fd);
int KsceIoRead(SceUID fd, void *data, SceSize size);
int KsceIoWrite(SceUID fd, const void *data, SceSize size);
int KsceIoLseek32(SceUID fd, int offset, int whence);

SceUID KsceIoDopen(const char *dirname);
int KsceIoDread(SceUID fd, SceIoDirent *dir);
int KsceIoDclose(SceUID fd);

int KsceIoAddDrv(PspIoDrv *drv);
int KsceIoDelDrv(const char *drv_name);

/////////////////////////////////////////////////////////////////////////////
//  utilsForKernel
/////////////////////////////////////////////////////////////////////////////
int KsceKernelDeflateDecompress(void *dst,int dsize,const void *src,int *psize);

/////////////////////////////////////////////////////////////////////////////
// ThreadManForKernel
/////////////////////////////////////////////////////////////////////////////
int KsceKernelGetThreadmanIdType(int);
int KsceKernelExtendKernelStack(int stack_size,int (*func)(void *param),void *param);

/////////////////////////////////////////////////////////////////////////////
//  KsceUmd
/////////////////////////////////////////////////////////////////////////////

//extern int KsceUmdIsUMD(void);
extern int KsceUmdCheckMedium(void);
//extern int KsceUmdRegisterDevice(void *func,int *param);
//extern int KsceUmdUnRegisterDevice(void);

extern int KsceUmdRegisterUMDCallBack(int cbid);

extern int KsceUmdSetDriveStatus(int stat);
extern int KsceUmdClearDriveStatus(int stat);
extern int KsceUmdGetDriveStatus(void);

extern int KsceUmdGetUmdCallBack(void);

extern int KsceUmdGetUMDPower(void);

extern unsigned int * KsceUmdCallBackInit;

extern unsigned int * KsceUmd_Unkonow_be17b77c ;
extern unsigned int * KsceUmd_Unkonow_28355079 ;
extern unsigned int * KsceUmd_Unkonow_7850f057 ;
extern unsigned int * KsceUmd_Unkonow_f6ac1dba ;
extern unsigned int * KsceUmd_Unkonow_1fb77367 ;
extern unsigned int * KsceUmd_Unkonow_84231fcf ;
extern unsigned int * KsceUmd_Unkonow_18624052 ;
extern unsigned int * KsceUmd_Unkonow_8ef268ac ;
extern unsigned int * KsceUmd_Unkonow_e71270fa ;
extern unsigned int * KsceUmd_Unkonow_04d1aad9 ;
//extern unsigned int * KsceUmdUnRegisterUMDCallBack ;
extern unsigned int * KsceUmd_Unkonow_27a764a1 ;
extern unsigned int * KsceUmd_Unkonow_7aa26c9a ;
extern unsigned int * KsceUmd_Unkonow_f8fd25e7 ;
extern unsigned int * KsceUmd_Unkonow_a140dec2 ;
extern unsigned int * KsceUmd_Unkonow_725dfb14 ;
extern unsigned int * KsceUmd_Unkonow_7f40cc41 ;
extern unsigned int * KsceUmd_Unkonow_1a2485d2 ;
extern unsigned int * KsceUmd_Unkonow_3ba4ec53 ;
extern unsigned int * KsceUmd_Unkonow_08709f2d ;
extern unsigned int * KsceUmd_Unkonow_ad18c797 ;
extern unsigned int * KsceUmdSetActivateData ;
extern unsigned int * KsceUmdSetDeactivateData ;
extern unsigned int * KsceUmd_Unkonow_075f1e0b ;
extern unsigned int * KsceUmdTableCodecBind ;
//extern unsigned int * KsceUmdCheckMedium ;
//extern unsigned int * KsceUmdSetDriveStatus ;
//extern unsigned int * KsceUmdClearDriveStatus ;
//extern unsigned int * KsceUmdGetDriveStatus ;
extern unsigned int * KsceUmd_Unkonow_3d0decd5 ;
extern unsigned int * KsceUmd_Unkonow_d01b2dc6 ;
extern unsigned int * KsceUmd_Unkonow_3925cbd8 ;
//extern unsigned int * KsceUmdGetUMDPower ;
extern unsigned int * KsceUmd_Unkonow_0f7e578a ;
extern unsigned int * KsceUmdSetResumeStatus ;
extern unsigned int * KsceUmdSetSuspendStatus ;
extern unsigned int * KsceUmd_Unkonow_3ce40626 ;
extern unsigned int * KsceUmd_Unkonow_1e62cca3 ;
extern unsigned int * KsceUmd_Unkonow_6a41ed25 ;
extern unsigned int * KsceUmd_Unkonow_4c952acf ;

/*
  STUB_FUNC 0x3342000c, KsceUmdIsUMD 
  STUB_FUNC 0xb8479844, KsceUmdCallBackInit 
  STUB_FUNC 0xbe17b77c, KsceUmd_Unkonow_be17b77c 
  STUB_FUNC 0x28355079, KsceUmd_Unkonow_28355079 
  STUB_FUNC 0x7850f057, KsceUmd_Unkonow_7850f057 
  STUB_FUNC 0xf6ac1dba, KsceUmd_Unkonow_f6ac1dba 
  STUB_FUNC 0xaee7404d, KsceUmdRegisterUMDCallBack 
  STUB_FUNC 0x1fb77367, KsceUmd_Unkonow_1fb77367 
  STUB_FUNC 0x84231fcf, KsceUmd_Unkonow_84231fcf 
  STUB_FUNC 0x18624052, KsceUmd_Unkonow_18624052 
  STUB_FUNC 0x8ef268ac, KsceUmd_Unkonow_8ef268ac 
  STUB_FUNC 0xe71270fa, KsceUmd_Unkonow_e71270fa 
  STUB_FUNC 0x04d1aad9, KsceUmd_Unkonow_04d1aad9 
  STUB_FUNC 0xbd2bde07, KsceUmdUnRegisterUMDCallBack 
  STUB_FUNC 0x27a764a1, KsceUmd_Unkonow_27a764a1 
  STUB_FUNC 0x7aa26c9a, KsceUmd_Unkonow_7aa26c9a 
  STUB_FUNC 0xf8fd25e7, KsceUmd_Unkonow_f8fd25e7 
  STUB_FUNC 0xa140dec2, KsceUmd_Unkonow_a140dec2 
  STUB_FUNC 0x725dfb14, KsceUmd_Unkonow_725dfb14 
  STUB_FUNC 0x7f40cc41, KsceUmd_Unkonow_7f40cc41 
  STUB_FUNC 0x1a2485d2, KsceUmd_Unkonow_1a2485d2 
  STUB_FUNC 0x3ba4ec53, KsceUmd_Unkonow_3ba4ec53 
  STUB_FUNC 0x08709f2d, KsceUmd_Unkonow_08709f2d 
  STUB_FUNC 0xad18c797, KsceUmd_Unkonow_ad18c797 
  STUB_FUNC 0x9cadbf19, KsceUmdSetActivateData 
  STUB_FUNC 0x7f38693e, KsceUmdSetDeactivateData 
  STUB_FUNC 0x075f1e0b, KsceUmd_Unkonow_075f1e0b 
  STUB_FUNC 0xeb56097e, KsceUmdTableCodecBind 
  STUB_FUNC 0x46ebb729, KsceUmdCheckMedium 
  STUB_FUNC 0x230666e3, KsceUmdSetDriveStatus 
  STUB_FUNC 0xae53dc2d, KsceUmdClearDriveStatus 
  STUB_FUNC 0xd45d1fe6, KsceUmdGetDriveStatus 
  STUB_FUNC 0x3d0decd5, KsceUmd_Unkonow_3d0decd5 
  STUB_FUNC 0xd01b2dc6, KsceUmd_Unkonow_d01b2dc6 
  STUB_FUNC 0x3925cbd8, KsceUmd_Unkonow_3925cbd8 
  STUB_FUNC 0x003e3396, KsceUmdGetUMDPower 
  STUB_FUNC 0x0f7e578a, KsceUmd_Unkonow_0f7e578a 
  STUB_FUNC 0x9dcb2da6, KsceUmdSetResumeStatus 
  STUB_FUNC 0xfd3878d6, KsceUmdSetSuspendStatus 
  STUB_FUNC 0x3ce40626, KsceUmd_Unkonow_3ce40626 
  STUB_FUNC 0x1e62cca3, KsceUmd_Unkonow_1e62cca3 
  STUB_FUNC 0x6a41ed25, KsceUmd_Unkonow_6a41ed25 
  STUB_FUNC 0x4c952acf, KsceUmd_Unkonow_4c952acf 
*/

/////////////////////////////////////////////////////////////////////////////
//  KscePower
/////////////////////////////////////////////////////////////////////////////
int  KscePowerSetCpuClockFrequency(int);
int  KscePowerSetClockFrequency(int,int,int); // 333,333,166
int  KscePowerGetCpuClockFrequency(void);
int  KscePowerGetBusClockFrequency(void);

int  KscePowerGetCpuClockFrequencyInt(void);
int  KscePowerGetBusClockFrequencyInt(void);

int  KscePowerGetBatteryVolt(void);

#else

/////////////////////////////////////////////////////////////////////////////
// for User , unsupported SDK , etc. 
/////////////////////////////////////////////////////////////////////////////
int  sceKernelDevkitVersion(void);
int  sceUmdGetDiscInfo(unsigned int *);
int sceUmdDeactivate(int unit, const char *drive);

int sceKernelGetThreadmanIdType(unsigned int);

// 
//int sceKernelStopModule(int,int,int,int,int);
int sceKernelUnloadModule(int);

/* extern */
//unsigned int sceKernelDevkitVersion(void);

/////////////////////////////////////////////////////////////////////////////
// IoFileMgrForKernel
/////////////////////////////////////////////////////////////////////////////
SceUID KsceIoDopen(const char *dirname);
int KsceIoDread(SceUID fd, SceIoDirent *dir);
int KsceIoDclose(SceUID fd);

/////////////////////////////////////////////////////////////////////////////
// LoadExecForKernel
/////////////////////////////////////////////////////////////////////////////

extern int KsceKernelLoadExec(const char *path,void *param);

/////////////////////////////////////////////////////////////////////////////
// LoadCoreForKernel
/////////////////////////////////////////////////////////////////////////////
extern void KsceKernelIcacheClearAll(void);

/////////////////////////////////////////////////////////////////////////////
//KDebugForKernel
/////////////////////////////////////////////////////////////////////////////
void KsceKernelRegisterDebugPutchar(void (*handler)(int,int));
void KsceKernelRegisterKprintfHandler(void *func);
void KKprintf(const char *formar,...);

/////////////////////////////////////////////////////////////////////////////
//  KsceUmdMan_driver
/////////////////////////////////////////////////////////////////////////////

extern unsigned int * KsceUmdMan_driver_cc80cfc6;

  extern unsigned int * KsceUmdManInit ;
  extern unsigned int * KsceUmdManTerm ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_7ed141fe ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_5a302102 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_b4692d7f ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_40beb9d1 ;
  extern unsigned int * KsceUmdManDriveStop ;
  extern unsigned int * KsceUmdManStart ;
  extern unsigned int * KsceUmdManStop ;
  extern unsigned int * KsceUmdManGetUmdDrive ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_6a77a311 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_b511f821 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_736ae133 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_5c026599 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_cd48f9c2 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_b3311b6e ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_c1d7c860 ;
  extern unsigned int * KsceUmdManCheckDeviceReady ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_e2b0fb78 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_e779ecef ;
  extern unsigned int * KsceUmdManRandTranslationReply ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_1b1bf9fd ;
  extern unsigned int * KsceUmdManThreadManagerReadAsync ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_3d44babf ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_e3f448e0 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_1b19a313 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_2cbe959b ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_2a39569b ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_cee55e3e ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_e5b7edc5 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_65e1b97e ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_5aa96415 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_250e6975 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_2a08fe9a ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_68577709 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_f819e17c ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_61c32a52 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_7094e3a7 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_d31dad7e ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_108b2322 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_98345381 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_bf88476f ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_485d4925 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_73e49f8f ;
  extern unsigned int * KsceUmdExecTestCmd ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_92f1cc33 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_61eb07a5 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_eca9476e ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_bea40117 ;
  extern unsigned int * KsceUmdManGpioSetup ;
  extern unsigned int * KsceUmdManGpioIntrStart ;
  extern unsigned int * KsceUmdManGpioIntrStop ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_b2dde9f8 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_b989e127 ;
  extern unsigned int * KsceUmdManGetDriveMode ;
  extern unsigned int * KsceUmdManGetPowerStat; 
  extern unsigned int * KsceUmdManGetPower ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_93539196 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_c8d45a7b ;
  extern unsigned int * KsceUmdManChangePowerMode ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_f4aff62d ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_84410a8e ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_39704b6e ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_63acfd28 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_cea5c857 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_8634ffc7 ;
  extern unsigned int * KsceUmdManSyncState ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_c634697d ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_7e2a680c ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_1cb85f05 ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_43255ad6 ;
  extern unsigned int * KsceUmdManAbort ;
  extern unsigned int * KsceUmdManReset ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_e192c10a ;
  extern unsigned int * KsceUmdManGetDiscInfo ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_aaa4ed91 ;
  extern unsigned int * KsceUmdManPowerOnOff ;
  extern unsigned int * KsceUmdManProbeRemainIcache ;
  extern unsigned int * KsceUmdMan_driver_Unkonow_815aa6d3; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_f6be91fc; 
  extern unsigned int * KsceUmdManValidateUMD; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_77e6c03a; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_a869cab3; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_3a437ad7; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_f7c603a2; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_4ea8ea5d; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_7ebaea9f; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_c984e1cf; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_a0b257a7; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_4c6bf421; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_6e7fc8f0; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_66cb0cc4; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_a8359e04; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_73966de9; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_fd1a113a; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_23c3c3d6; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_7c3d307c; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_58e3718d; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_08eb09c8; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_5304ca4a; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_6cb4a8d6; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_a58946ca; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_d1d4f296; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_0bdd5754; 
  extern unsigned int * KsceUmdManSetAlarm; 
  extern unsigned int * KsceUmdManCancelAlarm; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_51a9ac49; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_43aa300a; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_10089e13; 
  extern unsigned int * KsceUmdMan_driver_Unkonow_94a857c1; 

/////////////////////////////////////////////////////////////////////////////
//  KsceSyscon_driver
/////////////////////////////////////////////////////////////////////////////
extern int KsceSysconSetUmdSwitchCallback(int cbid);
extern int KsceSysconGetUmdSwitch(void);

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int sceKernelNotifyCallback(int,int);

/////////////////////////////////////////////////////////////////////////////
// sceNand_driver
/////////////////////////////////////////////////////////////////////////////
void *KsceNandWriteAccess;
void *KsceNandEraseBlock;


/////////////////////////////////////////////////////////////////////////////
// scePower_driver
/////////////////////////////////////////////////////////////////////////////

int KscePowerSetCpuClockFrequency(int cpufreq);
int KscePowerSetBusClockFrequency(int busfreq);
int KscePowerGetCpuClockFrequency(void);
int KscePowerGetCpuClockFrequencyInt(void);
int KscePowerSetClockFrequency(int cpufreq, int ramfreq, int busfreq);

#endif
