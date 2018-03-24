/*
	based by

 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * crt0_prx.c - Pure PRX startup code.
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 *
 * $Id: crt0.c 1526 2005-12-06 21:56:06Z tyranid $

*/

#include <pspkerneltypes.h>
#include <pspmoduleinfo.h>
#include <pspthreadman.h>
#include <stdlib.h>
#include <string.h>

#include "../include/devhook.h"

PSP_MODULE_INFO("UMD_ISO_CISO", 0x3007, 1, 2);
PSP_MAIN_THREAD_ATTR(0); // 0 for kernel mode too

int module_start(SceSize args, void *argp) __attribute__((alias("_start")));

int _start(SceSize args, void *argp)
{
#if 0
	if( (dhGetVersion() & 0xffffff00)!= 0x00004200)
	{
		Kprintf("unsupported devhook version\n");
		return 0;
	}
#endif
	// regist ISO handler (low priority 1st egist)
extern const DH_UMD_PLUGIN iso_plugin;
	dhUMDAddDevice(&iso_plugin);

	// internal CISO handler (high priority)
extern const DH_UMD_PLUGIN ciso_plugin;
	dhUMDAddDevice(&ciso_plugin);

	return 0;
}

int module_stop(SceSize args, void *argp)
{
	return 0;
}
