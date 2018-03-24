#ifndef __MY_BOOT_H__
#define __MY_BOOT_H__

// flash0,1のリダイレクトパス
#define MS_FLASH0_PATH "/flash0"
#define MS_FLASH1_PATH "/flash1"

/* reboot.bin address area */
#define REBOOT_ADDR 0x88c00000
#define REBOOT_SIZE 0x00300000

/* PATCHER code */
#define PATCHER_ADDR 0x88f80000
#define PATCHER_SIZE 0x00050000

/* PATCHER parameters */
#define DH_CONFIG_PARAM   0x88fd0000
#define RAMDISK_TOP_PARAM 0x88fdfff8

/* RAMDISK address AREA */
/*
	キャッシュ容量はFW2.50 DH042aで、約2MB程度必要
*/
#define RAMDISK_TOP          0x89400000

#define RAMDISK_PRELOAD_DEF_ADDR 0x89000000

#if 0
/* after load data */
#define RAMDISK_NOLOAD_LIST  0x88fe0000
#define RAMDISK_NOLOAD_MAGIC 0x5555aaaa
#endif

#if 0
/* flash preload emulation */
#define RAMDISK_OPEN_ENTRY  0x88bfffe0
#define RAMDISK_READ_ENTRY  0x88bfffe4
#define RAMDISK_CLOSE_ENTRY 0x88bfffe8
#endif

/*
  code_base     : Loadexec code base address == entry_of_sceKernelLoadExec-0x000010d0;
                  an entry of sceKernelLoadExec can get by stub of 'LoadExecForKernel'.
  boot_img_name : boot image 'PLANE BINALY FORMAT' (example "fatms0:/myboot.img" )
  address       : top of boot image (64K orign,original = 0x88c00000)
  size          : maximum program size (original = 0x00400000)

note:
  kernel monde only

  if return value == 0 then ok, use sceKernelLoadExec() to boot your image.
*/
int install_my_reboot(const char *sm_path);

int reboot_preload(void);
void *reboot_load(void);

#endif
