#ifndef __REGISTRY__H_
#define __REGISTRY__H_

int regGetClock(u32 *cpu_mhz,u32 *bus_mhz);
int regSetClock(u32 cpu_mhz,u32 bus_mhz);

int regGetAutoMenu(void);
int regSetAutoMenu(int val);

int regGetUmdDelaymount(void);
int regSetUmdDelaymount(int val);

int regGetUmdPath(void *path,int max_size);
int regSetUmdPath(void *path);

int regGetSfoVer(void);
int regSetSfoVer(int sfo_ver);

int regGetPreloadAddr(void);
int regSetPreloadAddr(int addr);

int regGetKprintfUart(void);
int regSetKprintfUart(int val);

#endif
