#ifndef __CONFIG_H_
#define __CONFIG_H_

// preload address
#define SUPPORT_PRELOAD 1
// preload address
#define SUPPORT_SFO_VER 1
// for debug , dump kernel and user memory
#define DUMP_KMEM 0
// return to normal Firmware
#define SUPPORT_NORMALFW 1

////////////////////////////////////////////////////
// Do not supported functions , please waiting
///////////////////////////////////////////////////

// select other firmware setting
#define SUPPORT_FW_SEL  1

// remote controll / Kprintf port
#define SUPPORT_REMOTE  1

////////////////////////////////////////////////////
// Can not supported functions , Help me
////////////////////////////////////////////////////
#define SUPPORT_REBOOT 0

#endif
