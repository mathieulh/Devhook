TARGET = smplain
OBJS = main.o stubkk.o

BUILD_PRX=1
PRX_EXPORTS=exports.exp

USE_KERNEL_LIBS = 1
#USE_KERNEL_LIBC = 1
#USE_PSPSDK_LIBC = 1

INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS = -nostartfile -nostdlib -nodefaultlibs

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

