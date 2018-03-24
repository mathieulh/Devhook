#
# stub user area to Kernel LIB
#
	.set noreorder

#include "common.s"

#
# usermode API for TSR
#

STUB_START "LoadExecForKernel",0x40010000,0x00150005 
  STUB_FUNCK 0xbd2f1094,KsceKernelLoadExec 
  STUB_FUNCK 0x2ac9954b,KsceKernelExitGameWithStatus 
  STUB_FUNCK 0x05572a5f,KsceKernelExitGame 
  STUB_FUNCK 0xac085b9e,KsceKernelLoadExecVSHFromHost 
  STUB_FUNCK 0x1b97bdb3,LoadExecForKernel_Unkonow_1b97bdb3 
  STUB_FUNCK 0x821be114,LoadExecForKernel_Unkonow_821be114 
  STUB_FUNCK 0x015da036,KsceKernelLoadExecBufferVSHUsbWlan 
  STUB_FUNCK 0x4f41e75e,LoadExecForKernel_Unkonow_4f41e75e 
  STUB_FUNCK 0x31df42bf,LoadExecForKernel_Unkonow_31df42bf 
  STUB_FUNCK 0x28d0d249,LoadExecForKernel_Unkonow_28d0d249 
  STUB_FUNCK 0x70901231,LoadExecForKernel_Unkonow_70901231 
  STUB_FUNCK 0xa3d5e142,KsceKernelExitVSHVSH 
  STUB_FUNCK 0x62c459e1,KsceKernelLoadExecBufferVSHPlain 
  STUB_FUNCK 0x918782e8,KsceKernelLoadExecBufferVSHFromHost 
  STUB_FUNCK 0xbb28e9b7,KsceKernelLoadExecBufferPlain0 
  STUB_FUNCK 0x71a1d738,KsceKernelLoadExecBufferPlain 
  STUB_FUNCK 0x4d5c75be,KsceKernelLoadExecFromHost 
  STUB_FUNCK 0x4ac57943,KsceKernelRegisterExitCallback 
  STUB_FUNCK 0xd9739b89,KsceKernelUnregisterExitCallback 
  STUB_FUNCK 0x659188e1,KsceKernelCheckExitCallback 
  STUB_FUNCK 0x62a27008,KsceKernelInvokeExitCallback 
STUB_END 
