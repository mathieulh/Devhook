#
# stub normaly Kernel to Kernel link
#
	.set noreorder

#include "pspstub.s"

STUB_START "sceNand_driver",0x40010000,0x00010005 
  STUB_FUNC 0x0BEE8F36,sceNand_driver_0bee8f36 
STUB_END 

