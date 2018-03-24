
extern void (*Kprintf)(const char *format,...);
extern int (*cache_clr1)(void);
extern void(*cache_clr2)(int);
void cache_clear(void);

// subroutine
void patch_jal(int patch_addr , void *hook_addr,void **org_addr);
void cache_clear(void);

// flash access
extern int (*org_sceBootLfatOpen)(char *arg1,int arg2,int arg3);
int hook_sceBootLfatOpen(char *arg1,int arg2,int arg3);
extern int (*org_sceBootLfatRead)(int arg1,int arg2,int arg3);
int hook_sceBootLfatRead(int arg1,int arg2,int arg3);
extern int (*org_sceBootLfatClose)(int arg1);
int hook_sceBootLfatClose(int arg1);


// 1st load/start
void hook_start_loadcore(int arg1,void **arg2,int arg3,void *entry);
int hook_start_sysmem(int arg1,void **arg2);

