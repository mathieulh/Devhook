/*
	DEV HOOK MAIN PRX
 */
#include "common.h"

#define DEVHOOK_VERSION 0x00520000

#define USE_NEW_THREAD 1

#define WALKER_INTERVAL 100000

/////////////////////////////////////////////////////////////////////////////
// TSR thread
/////////////////////////////////////////////////////////////////////////////
int dh_walker_tern = 0;

int TSRThread(SceSize args, void *argp)
{
	static int cnt_sec = -10*(1000000/WALKER_INTERVAL); // disable clock controll during first 10sec.

	// lowast priority
	sceKernelChangeThreadPriority(0,46);

	while(1)
	{
		sceKernelDelayThread(WALKER_INTERVAL);

		// halt check
		if(dh_walker_tern) break;

		// UMD mount / monitor
		umdemu_walker(WALKER_INTERVAL);

		// about 1sec timer
		if(++cnt_sec >= (1000000/WALKER_INTERVAL) )
		{
			cnt_sec = 0;
			// Clock changer
			clock_checker(1000000);
		}
	}
	// terminate
	dh_walker_tern++;
	return 1;
}

#if USE_NEW_THREAD
int setupThread(void)
{
  int thid = 0;

  thid = sceKernelCreateThread("DH_WALKER", TSRThread, 0x11, 0xFA0, 0, 0);
  if(thid >= 0)
  {
    sceKernelStartThread(thid, 0, 0);
  }
  return thid;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// myKprintf
/////////////////////////////////////////////////////////////////////////////
int vfprintf_(FILE_*fp,const char *format,va_list arg);

static FILE_ uart_file =
{
  (void *)uart_dbg_putc,
  NULL,
  NULL,
  NULL
};

void myKprintf(const char *text,...)
{
	va_list arg;
//	u32 *p;
	uart_dbg_putc(0,0x200);
	va_start(arg,text);
	vfprintf_(&uart_file,text,arg);
	va_end(arg);
	uart_dbg_putc(0,0x201);
}

/////////////////////////////////////////////////////////////////////////////
// load config from ...
// StartModule()コールで初期化すべき処理
/////////////////////////////////////////////////////////////////////////////
void dhLoadConfig(void)
{
	u32 val32;

//Kprintf("dhLoadConfig\n");

//Kprintf("load config\n");
	// コンフィグデータをロード
	dhLoadRegistry((void *)DH_CONFIG_PARAM);

//Kprintf("load umdemu_init\n");
	// umdエミュを初期化,plugin 受付開始のため、ここで行う必要あり
	umdemu_init();

	// BIOSモードでは、待ってはいけない！
	// waiting ms0: ready
//	wait_device("ms0:/",200);

//Kprintf("KPRINT_UART\n");
	// hook Kprintf
	if(!dhGetRegistry("KPRINT_UART",&val32,4) && val32)
	{
		/* catch Kprintf */
		hookAPIDirect((void *)Kprintf,myKprintf,NULL);
	}

	// install flash emulation
	install_flash_emu();

	// install UMD emulation
//Kprintf("hook UMD driver\n");
	umdemu_reset();
	hook_umd9660_driver();

	// load boot prx after devhook (no-use)
	ramdisk_load_after_boot();

#if 1
	// 2.80 音無フリーズ対策
extern void sceClockgen_driver_Unkonow_5f8328fd(void);
	sceClockgen_driver_Unkonow_5f8328fd();
#endif
}

/////////////////////////////////////////////////////////////////////////////
// Entry
/////////////////////////////////////////////////////////////////////////////
//int main(int argc, char **argv){ return 0; }

//int StartModule(int argc, char **argv)
int main(int argc, char **argv)
{
	// hook reboot
//	Kprintf("Install my reboot\n");
	install_my_reboot(0);

	// power hook
	// install_power_callback();

	// get clock value
	clock_init();

#if USE_NEW_THREAD
	 setupThread();
#else
	TSRThread(0,NULL);
#endif
//	Kprintf("Finish DEVHOOK ENTRY\n");
	return 1;
}

////////////////////////////////////
////////////////////////////////////
unsigned int dhGetVersion(void)
{
	return DEVHOOK_VERSION;
}

