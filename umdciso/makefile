TARGET = umdciso
OBJS = crt0_prx.o umd_ciso.o umd_iso.o

# Define to build this as a prx (instead of a static elf)
BUILD_PRX=1
# Define the name of our custom exports (minus the .exp extension)
PRX_EXPORTS=exports.exp

USE_KERNEL_LIBS = 1
USE_KERNEL_LIBC = 1

INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR  = ../lib
LIBS    = -lpspumd -ldevhook
LDFLAGS =  -nostdlib -nodefaultlibs

#EXTRA_TARGETS = EBOOT.PBP
#PSP_EBOOT_TITLE = PSP DEVHOOK
#PSP_EBOOT_ICON = icon0.png
#PSP_EBOOT_UNKPNG = pic1.png

PSPSDK=$(shell psp-config --pspsdk-path)
#include build.mak
#include $(PSPSDK)/lib/build_ns.mak
include $(PSPSDK)/lib/build.mak

LIBS += -lpsphprm_driver

