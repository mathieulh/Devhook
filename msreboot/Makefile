TARGET = msreboot
OBJS = \
  crt0.o \
  msreboot.o \
  psp_uart.o \
  patch150.o \
  patch200.o \
  patch250.o \
  patch260.o \
  patch271.o \
  patch280.o \
  patch300.o \
  loadcore.o \
  sysmem.o \
  registry.o \
  ihex.o \

INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = -lc
LIBDIR =
#LDFLAGS = -nostartfiles -T msreboot.x -nostdlib
LDFLAGS = -nostartfiles -T msreboot.x

#EXTRA_TARGETS = EBOOT.PBP
#PSP_EBOOT_TITLE = PRX LOAD TEST

PSPSDK=$(shell psp-config --pspsdk-path)
#include $(PSPSDK)/lib/build.mak
include build.mak
