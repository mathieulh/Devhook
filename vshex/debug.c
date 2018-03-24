/*
	PSP VSH extender for devhook 0.50+

	debug function
*/
#include "common.h"

#if DUMP_KMEM
/////////////////////////////////////////////////////////////////////////////
// change CPU clock
/////////////////////////////////////////////////////////////////////////////
char g_data[0x1000] __attribute__((aligned(64)));

/* Well what would you expect ? :) */
void dump_memregion(const char* file, void *addr, int len)
{
	int fd;

	fd = sceIoOpen(file, PSP_O_CREAT | PSP_O_TRUNC | PSP_O_WRONLY, 0777);
	if(fd >= 0)
	{
		printf("Writing %s\n", file);
		while(len>0)
		{
			memcpy(g_data, addr, 0x1000);
			sceIoWrite(fd, g_data, 0x1000);
			len -= 0x1000;
			addr += 0x1000;
		}
		sceIoClose(fd);
	}
}
#endif
