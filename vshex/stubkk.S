#
# stub normaly Kernel to Kernel link
#
	.set noreorder

#include "pspstub.s"

STUB_START "scePower_driver", 0x40010000, 0x00030005
	STUB_FUNC	0x0442D852, scePower_driver_0442D852
	STUB_FUNC	0xFEE03A2F, scePowerGetCpuClockFrequency
	STUB_FUNC	0x478FE6F5, scePowerGetBusClockFrequency
STUB_END


STUB_START "sceSuspendForKernel",0x40010000,0x000d0005 
  STUB_FUNC 0xbde686cd,sceKernelRegisterPowerHandlers 
  STUB_FUNC 0xeadb1bd7,sceKernelPowerLock 
  STUB_FUNC 0xb53b2147,sceKernelPowerLockForUser 
  STUB_FUNC 0x3aee7261,sceKernelPowerUnlock 
  STUB_FUNC 0xc7c928c7,sceKernelPowerUnlockForUser 
  STUB_FUNC 0x090ccb3f,sceKernelPowerTick 
  STUB_FUNC 0x98a1d061,sceKernelPowerRebootStart 
  STUB_FUNC 0x91a77137,sceKernelRegisterSuspendHandler 
  STUB_FUNC 0xb43d1a8c,sceKernelRegisterResumeHandler 
  STUB_FUNC 0x67b59042,sceSuspendForKernel_Unkonow_67b59042 
  STUB_FUNC 0xb2c9640b,sceSuspendForKernel_Unkonow_b2c9640b 
  STUB_FUNC 0x8f58b1ec,sceKernelDispatchSuspendHandlers 
  STUB_FUNC 0x0ab0c6f3,sceKernelDispatchResumeHandlers 
STUB_END 

STUB_START "LoadExecForKernel",0x40010000,0x00070005 
  STUB_FUNC 0x1B97BDB3,LoadExecForKernel_Unkonow_1b97bdb3 
  STUB_FUNC 0x28D0D249,LoadExecForKernel_Unkonow_28d0d249 
  STUB_FUNC 0x31DF42BF,LoadExecForKernel_Unkonow_31df42bf 
  STUB_FUNC 0x4F41E75E,LoadExecForKernel_Unkonow_4f41e75e 
  STUB_FUNC 0x70901231,LoadExecForKernel_Unkonow_70901231 
  STUB_FUNC 0x7B7C47EF,LoadExecForKernel_7b7c47ef 
  STUB_FUNC 0x821BE114,LoadExecForKernel_Unkonow_821be114 
STUB_END 

STUB_START "sceAudio_driver",0x40010000,0x00010005 
  STUB_FUNC 0x136CAF51,sceAudioOutputBlocking 
STUB_END 

