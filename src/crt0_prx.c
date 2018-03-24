/*
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

#include "../include/patch.h"
extern u32 dhGetVersion(void);
extern void dhLoadConfig(void);

//#include "../fep/dh_cfg.h"

PSP_MODULE_INFO("DEVHOOK", 0x3007, 1, 2);
PSP_MAIN_THREAD_ATTR(0); // 0 for kernel mode too

/* The maximum number of arguments that can be passed to main(). */
#define ARG_MAX 19

/* Default thread parameters for the main program thread. */
//#define DEFAULT_THREAD_PRIORITY 32
#define DEFAULT_THREAD_PRIORITY 16
#define DEFAULT_THREAD_ATTRIBUTE 0
#define DEFAULT_THREAD_STACK_KB_SIZE 16
#define DEFAULT_MAIN_THREAD_NAME "devhook_walker"

/* Define us as a prx */
int __pspsdk_is_prx = 1;

/* If these variables are defined by the program, then they override the
   defaults given above. */
extern int sce_newlib_nocreate_thread_in_start __attribute__((weak));
extern unsigned int sce_newlib_priority __attribute__((weak));
extern unsigned int sce_newlib_attribute __attribute__((weak));
extern unsigned int sce_newlib_stack_kb_size __attribute__((weak));
extern const char*  sce_newlib_main_thread_name __attribute__((weak));

/* This is declared weak in case someone compiles an empty program.  That
   program won't work on the PSP, but it could be useful for testing the
   toolchain. */
//extern SceModuleInfo module_info __attribute__((weak));

extern int main(int argc, char *argv[]);

/**
 * Main program thread
 *
 * Initializes runtime parameters and calls the program's main().
 *
 * @param args - Size (in bytes) of the argp parameter.
 * @param argp - Pointer to program arguments.  Each argument is a NUL-terminated string.
 */
void _main(SceSize args, void *argp)
{
        char *argv[ARG_MAX + 1];
        int argc = 0;
        int loc = 0;
        char *ptr = argp;

        /* Turn our thread arguments into main()'s argc and argv[]. */
        while(loc < args)
        {
                argv[argc] = &ptr[loc];
                loc += strlen(&ptr[loc]) + 1;
                argc++;
                if(argc == ARG_MAX)
                {
                        break;
                }
        }

        argv[argc] = NULL;

        /* Call main(). */
//        int res = main(argc, argv);
		main(argc, argv);

        /* Return control to the operating system. */
//        exit(res);
}

int module_start(SceSize args, void *argp) __attribute__((alias("_start")));

/**
 * Startup thread
 *
 * Creates the main program thread based on variables defined by the program.
 *
 * @param args - Size (in bytes) of arguments passed to the program by the kernel.
 * @param argp - Pointer to arguments passed by the kernel.
 */
int _start(SceSize args, void *argp)
{
	Kprintf("----- enter %s %08X -----\n",module_info.modname,dhGetVersion());

	if (&sce_newlib_nocreate_thread_in_start != NULL) {
                /* The program does not want main() to be run in a seperate thread. */
                _main(args, argp);
                return 1;
        }


        int priority = DEFAULT_THREAD_PRIORITY;
        unsigned int attribute = DEFAULT_THREAD_ATTRIBUTE;
        unsigned int stackSize = DEFAULT_THREAD_STACK_KB_SIZE * 1024;
        const char *threadName = DEFAULT_MAIN_THREAD_NAME;

        if (&sce_newlib_priority != NULL) {
                priority = sce_newlib_priority;
        }
        if (&sce_newlib_attribute != NULL) {
                attribute = sce_newlib_attribute;
        }
        if (&sce_newlib_stack_kb_size != NULL) {
                stackSize = sce_newlib_stack_kb_size * 1024;
        }
        if (&sce_newlib_main_thread_name != NULL) {
                threadName = sce_newlib_main_thread_name;
        }

	// laod & hook & setup
	sceKernelExtendKernelStack(0x4000,(void *)dhLoadConfig,NULL);

        SceUID thid;
//        thid = sceKernelCreateThread(threadName, (void *) _main, priority, stackSize, attribute, 0);
	thid = sceKernelCreateThread(threadName, (void *) _main, priority, stackSize, 0, 0);


        sceKernelStartThread(thid, args, argp);
        return 0;
}
