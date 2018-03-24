/*
	enable plain prx for PSP
*/
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <pspctrl.h>
#include <stdio.h>
#include <string.h>

PSP_MODULE_INFO("plain prx for 200-301", 0x1000, 1, 1);
PSP_MAIN_THREAD_ATTR(0);

/*****************************************************************************
*****************************************************************************/
extern int loadcore2x_patch(void);
//extern int modulemgr_patch(void);

/*****************************************************************************
 パッチ後のキャッシュクリア
*****************************************************************************/
void clear_cache(void)
{
	sceKernelDcacheWBinvAll();
	sceKernelIcacheClearAll();
}

void patchSysMem(void);

/*****************************************************************************
*****************************************************************************/
int module_start(SceSize args, void *argp)
{
//	int param[2];

	Kprintf("----- enter %s -----\n",module_info.modname);

//	sceKernelExtendKernelStack(0x4000,(void *)loadcore20_patch,(void *)&param);
	loadcore2x_patch();

	patchSysMem();

//	modulemgr_patch();

//	clear_cache();

	return 1;
}
