#
	.set noreorder

#include "pspstub.s"

STUB_START "SysMemForKernel",0x40010000,0x00010005 
  STUB_FUNC 0x2F808748,SysMemForKernel_2f808748 
STUB_END 

