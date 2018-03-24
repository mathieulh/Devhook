/****************************************************************************

  DEVHOOK 0.5x launcher sample

****************************************************************************/

#include "common.h"

// current directry
const char base_directry[128] = "ms0:/dh/05beta";
const char confg_file_path[128] = "config.txt";

// enable Kprintf to REMOTE in launcher
// #define KPRINTF_IN_LAUNCHER

PSP_MODULE_INFO("PSP DEVHOOK 0.4x FEP", 0x1000, 1, 1);   // 0x1000 = Kernel MODE
PSP_MAIN_THREAD_ATTR(0); // 0 for kernel mode too

extern int sceKernelExitDeleteThread(int id);

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	int dh_bootmode = 0;

	// PAD init
	sceCtrlSetSamplingCycle( 0 );
	sceCtrlSetSamplingMode( 0 );
	Kprintf("pad initialized\n");

	// Screen Init
	pspDebugScreenInit();

	InitRegistry();
	sceIoChdir(base_directry);

	// load current setting
	pspDebugScreenPrintf("loading current setting\n");
	script_execFile(confg_file_path);

	// install font system
//	pspDebugScreenPrintf("loading font file\n");
//	text_init("ms0:/fbm/1/shnm6x12r.fbm", "ms0:/fbm/2/shnmk12.fbm");

#ifdef KPRINTF_IN_LAUNCHER
	// debug mesage for remote controll I/F 
	pspDebugSioInit();
	pspDebugSioSetBaud(115200);
	pspDebugSioInstallKprintf();
	/* pspDebugInstallStdoutHandler(pspDebugSioPutData); */
#endif

	// GUI menu
	menu_main(dh_bootmode);

	// remove from memory
	sceKernelExitDeleteThread(sceKernelGetThreadId());

	return 0;
}
