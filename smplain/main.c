/*
	smart media no encrypt patch for PSP 3.00+
 */

#include <pspkerneltypes.h>
#include <pspmoduleinfo.h>
#include <pspthreadman.h>
#include <stdlib.h>
#include <string.h>

#include "patch.h"

PSP_MODULE_INFO("sm plain", 0x3007, 1, 2);
PSP_MAIN_THREAD_ATTR(0);

extern void sceNand_driver_0bee8f36(int mode);

int module_start(SceSize args, void *argp)
{
	u32 *lp = (u32 *)sceNand_driver_0bee8f36;

	if( (lp[0] & 0xfc000000)==MIPS_J(0))
	{
		lp = (u32 *)(0x80000000 + ((lp[0]&0x03ffffff)<<2) );
		// kill this
		lp[0] = MIPS_JR(31);
		lp[1] = MIPS_LUI(2,0);

		sceKernelDcacheWritebackAll();
		sceKernelIcacheClearAll();
	}
	return 1; // To stay isn't necessary.
}
