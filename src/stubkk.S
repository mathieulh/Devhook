#
# stub normaly Kernel to Kernel link
#
	.set noreorder

#include "pspstub.s"

STUB_START "InterruptManagerForKernel",0x40010000,0x00010005 
  STUB_FUNC 0xcdc86b64,sceKernelCallSubIntrHandler 
STUB_END 

STUB_START "SysMemForKernel",0x40010000,0x00030005 
  STUB_FUNC 0x6d8e0cdf,SysMemForKernel_6d8e0cdf 
  STUB_FUNC 0xc7e57b9c,SysMemForKernel_c7e57b9c 

  STUB_FUNC 0xa089eca4,SysMemForKernel_a089eca4 

STUB_END 

STUB_START "sceGpio_driver",0x40010000,0x00010005 
  STUB_FUNC 0x4250d44a,sceGpioPortRead 
STUB_END 

STUB_START "sceUmd9660_driver",0x40010000,0x00020005 
  STUB_FUNC 0x0ec10f9f,sceUmd9660GetDrive 
  STUB_FUNC 0x85F6776E,sceUmd9660Init 
STUB_END

STUB_START	"LoadCoreForKernel",0x40010000,0x00010005
  STUB_FUNC	0x929b5c69,LoadCoreForKernel_929b5c69
STUB_END

STUB_START "sceUmd",0x40010000,0x00010005 
  STUB_FUNC 0xd45d1fe6,sceUmdGetDriveStatus 
STUB_END 

##########################################################

STUB_START "sceUmdMan_driver",0x40010000,0x00050005 
  STUB_FUNC 0xcc80cfc6,sceUmdMan_driver_cc80cfc6
  STUB_FUNC 0x4fb913a3,sceUmdMan_driver_4fb913a3
  STUB_FUNC 0xbf8aed79,sceUmdMan_driver_bf8aed79
  STUB_FUNC 0xe192c10a,sceUmdMan_driver_Unkonow_e192c10a
  STUB_FUNC 0x8609D1E4,sceUmdManGetDiscInfo
STUB_END 

#2.71+
STUB_START "IoFileMgrForKernel",0x40010000,0x00010005 
  STUB_FUNC 0xbd17474f,IoFileMgrForKernel_bd17474f 

#need 2.80+
STUB_START "sceClockgen_driver",0x40010000,0x00010005 
  STUB_FUNC 0x5F8328FD,sceClockgen_driver_Unkonow_5f8328fd 
#  STUB_FUNC 0xB9FE99D1,sceClockgen_driver_Unkonow_b9fe99d1 
STUB_END 


#SDK lib are buggy

#STUB_START "scePower_driver",0x40010000,0x003d0005 
STUB_START "scePower_driver",0x40010000,0x00030005 
#  STUB_FUNC 0x9ce06934,scePowerInit 
#  STUB_FUNC 0xad5bb433,scePowerEnd 
#  STUB_FUNC 0x6d2ca84b,scePowerWlanActivate 
#  STUB_FUNC 0x23bb0a60,scePowerWlanDeactivate 
#  STUB_FUNC 0x2b51fe2f,scePower_driver_Unkonow_2b51fe2f 
#  STUB_FUNC 0x442bfbac,scePower_driver_Unkonow_442bfbac 
#  STUB_FUNC 0xe8685403,scePower_driver_Unkonow_e8685403 
#  STUB_FUNC 0xefd3c963,scePowerTick 
#  STUB_FUNC 0xedc13fe5,scePowerGetIdleTimer 
#  STUB_FUNC 0x1ba2fcae,scePowerSetIdleCallback 
#  STUB_FUNC 0x7f30b3b1,scePowerIdleTimerEnable 
#  STUB_FUNC 0x972ce941,scePowerIdleTimerDisable 
#  STUB_FUNC 0x27f3292c,scePowerBatteryUpdateInfo 
#  STUB_FUNC 0xe8e4e204,scePower_driver_Unkonow_e8e4e204 
#  STUB_FUNC 0xb999184c,scePower_driver_Unkonow_b999184c 
#  STUB_FUNC 0x166922ec,scePower_driver_Unkonow_166922ec 
#  STUB_FUNC 0xdd3d4dac,scePower_driver_Unkonow_dd3d4dac 
#  STUB_FUNC 0x87440f5e,scePowerIsPowerOnline 
#  STUB_FUNC 0x0afd0d8b,scePowerIsBatteryExist 
#  STUB_FUNC 0x1e490401,scePowerIsBatteryCharging 
#  STUB_FUNC 0xb4432bc8,scePowerGetBatteryChargingStatus 
#  STUB_FUNC 0xd3075926,scePowerIsLowBattery 
#  STUB_FUNC 0x78a1a796,scePower_driver_Unkonow_78a1a796 
#  STUB_FUNC 0x94f5a53f,scePower_driver_Unkonow_94f5a53f 
#  STUB_FUNC 0xfd18a0ff,scePower_driver_Unkonow_fd18a0ff 
#  STUB_FUNC 0x2085d15d,scePowerGetBatteryLifePercent 
#  STUB_FUNC 0x8efb3fa2,scePowerGetBatteryLifeTime 
#  STUB_FUNC 0x28e12023,scePowerGetBatteryTemp 
#  STUB_FUNC 0x862ae1a6,scePowerGetBatteryElec 
#  STUB_FUNC 0x483ce86b,scePowerGetBatteryVolt 
#  STUB_FUNC 0x23436a4a,scePower_driver_Unkonow_23436a4a 
#  STUB_FUNC 0x0cd21b1f,scePower_driver_Unkonow_0cd21b1f 
#  STUB_FUNC 0x165ce085,scePower_driver_Unkonow_165ce085 
#  STUB_FUNC 0xd6d016ef,scePowerLock 
#  STUB_FUNC 0xca3d34c1,scePowerUnlock 
#  STUB_FUNC 0x79db9421,scePowerRebootStart 
#  STUB_FUNC 0xdb62c9cf,scePowerCancelRequest 
#  STUB_FUNC 0x7fa406dd,scePowerIsRequest 
#  STUB_FUNC 0x2b7c7cf4,scePowerRequestStandby 
#  STUB_FUNC 0xac32c9cc,scePowerRequestSuspend 
#  STUB_FUNC 0x2875994b,scePower_driver_Unkonow_2875994b 
#  STUB_FUNC 0x3951af53,scePowerEncodeUBattery 
#  STUB_FUNC 0x0074ef9b,scePowerGetResumeCount 
#  STUB_FUNC 0xf535d928,scePower_driver_Unkonow_f535d928 
#  STUB_FUNC 0x04b7766e,scePowerRegisterCallback 
#  STUB_FUNC 0xdfa8baf8,scePowerUnregisterCallback 
#  STUB_FUNC 0xdb9d28dd,scePowerUnregitserCallback 
#  STUB_FUNC 0xd24e6beb,scePower_driver_Unkonow_d24e6beb 
#  STUB_FUNC 0x35b7662e,scePowerGetSectionDescriptionEntry 
#  STUB_FUNC 0xf9b4dea1,scePowerLimitPllClock 
#  STUB_FUNC 0x843fbf43,scePowerSetCpuClockFrequency 
#  STUB_FUNC 0xb8d7b3fb,scePowerSetBusClockFrequency 
  STUB_FUNC 0xfee03a2f,scePowerGetCpuClockFrequency 
  STUB_FUNC 0x478fe6f5,scePowerGetBusClockFrequency 
#  STUB_FUNC 0xfdb5bfe9,scePowerGetCpuClockFrequencyInt 
#  STUB_FUNC 0xbd681969,scePowerGetBusClockFrequencyInt 
#  STUB_FUNC 0xb1a52c83,scePowerGetCpuClockFrequencyFloat 
#  STUB_FUNC 0x9badb3eb,scePowerGetBusClockFrequencyFloat 
  STUB_FUNC 0x737486f2,scePowerSetClockFrequency 
#  STUB_FUNC 0xe0b7a95d,scePower_driver_Unkonow_e0b7a95d 
#  STUB_FUNC 0xc23ac778,scePower_driver_Unkonow_c23ac778 
STUB_END 
