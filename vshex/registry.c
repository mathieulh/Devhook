/*
	PSP devhook registry contyroll
*/
#include "common.h"

int regGetClock(u32 *cpu_mhz,u32 *bus_mhz)
{
	u32 clock[2];
	clock[0] = 0;
	clock[1] = 0;
	dhGetRegistry("CLOCK",clock,sizeof(clock));
	if(cpu_mhz) *cpu_mhz = clock[0];
	if(bus_mhz) *bus_mhz = clock[1];
	return 0;
}

int regSetClock(u32 cpu_mhz,u32 bus_mhz)
{
	u32 clock[2];

	clock[0] = cpu_mhz;
	clock[1] = bus_mhz;
	// for next reboot
	dhSetRegistry("CLOCK",clock,sizeof(clock));
	return 0;
}

int regGetAutoMenu(void)
{
	u32 val = 0;
	dhGetRegistry("VSHEX_AUTOMENU",&val,4);
	return val ? 1 : 0;
}

int regSetAutoMenu(int val)
{
	if(val) dhSetRegistry("VSHEX_AUTOMENU",&val,4);
	else	dhSetRegistry("VSHEX_AUTOMENU",NULL,0);
	return val;
}

int regGetUmdDelaymount(void)
{
	u32 val = 0;
	dhGetRegistry("VSHEX_UMD_DELAY",&val,4);
	return val;
}

int regSetUmdDelaymount(int val)
{
	if(val) dhSetRegistry("VSHEX_UMD_DELAY",&val,4);
	else    dhSetRegistry("VSHEX_UMD_DELAY",NULL,0);
	return val;
}

int regGetUmdPath(void *path,int max_size)
{
	dhGetRegistry("UMD_PATH",path,max_size);
	return 0;
}

#if 0
int regSetUmdPath(void *path)
{
}
#endif

#if SUPPORT_SFO_VER
int regGetSfoVer(void)
{
	u32 sfo_ver = 0;
	dhGetRegistry("SFO_VER",&sfo_ver,4);
	return sfo_ver ? 1 : 0;
}
int regSetSfoVer(int sfo_ver)
{
	dhSetRegistry("SFO_VER",&sfo_ver,4);
	return sfo_ver;
}
#endif

#if SUPPORT_PRELOAD
int regGetPreloadAddr(void)
{
	u32 paddr = 0;
	dhGetRegistry("PRELOAD_ADDR",&paddr,4);
	return paddr;
}

int regSetPreloadAddr(int addr)
{
	dhSetRegistry("PRELOAD_ADDR",&addr,4);
	return addr;
}
#endif

#if SUPPORT_REMOTE
int regGetKprintfUart(void)
{
	u32 kprint_sw = 0;
	dhGetRegistry("KPRINTF_UART",&kprint_sw,4);
	return kprint_sw;
}

int regSetKprintfUart(int val)
{
	dhSetRegistry("KPRINTF_UART",&val,4);
	return val;
}
#endif

