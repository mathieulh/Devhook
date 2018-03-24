#include <pspkernel.h>
#include <pspdebug.h>
#include <stdio.h>
#include "patch.h"

/*****************************************************************************
 パッチ後のキャッシュクリア
*****************************************************************************/
void cache_clear(void)
{
	sceKernelDcacheWBinvAll();
	sceKernelIcacheClearAll();
}


