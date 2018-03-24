TARGET = devhook

OBJS = \
  src/crt0_prx.o \
  src/main.o \
  src/patch.o \
  src/registry.o \
  src/anyumd.o \
  src/umd_emu.o \
  src/hook_dev.o \
  src/msboot.o \
  src/fileio.o \
  src/ramdisk.o \
  src/flash_em.o \
  src/ms_share.o \
  src/clock.o \
  src/psp_uart.o \
  src/vsprintf.o \
  src/stubkk.o \
  src/setk1.o

# Define to build this as a prx (instead of a static elf)
BUILD_PRX=1
# Define the name of our custom exports (minus the .exp extension)
PRX_EXPORTS=src/exports.exp

#when choice this boot 2.80 but hang on X
USE_KERNEL_LIBS = 1

USE_KERNEL_LIBC = 1
#USE_PSPSDK_LIBC = 1

INCDIR = 
CFLAGS = -O2 -Os -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
#LIBS    = -lpspumd  -lpsppower
LIBS    = -lpsppower
LDFLAGS =  -nostdlib -nodefaultlibs

#EXTRA_TARGETS = EBOOT.PBP
#PSP_EBOOT_TITLE = PSP DEVHOOK
#PSP_EBOOT_ICON = icon0.png
#PSP_EBOOT_UNKPNG = pic1.png

PSPSDK=$(shell psp-config --pspsdk-path)
include build.mak
#include $(PSPSDK)/lib/build.mak

#LIBS += -lpsphprm_driver

