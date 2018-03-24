TARGET = vshex
OBJS = \
  main.o \
  blit.o \
  clock.o \
  debug.o \
  font.o \
  menu.o \
  patch.o \
  registry.o \
  ui.o \
  umd.o \
  stubkk.o \

BUILD_PRX=1
PRX_EXPORTS=exports.exp

USE_KERNEL_LIBS = 1
USE_KERNEL_LIBC = 1

INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR  = ../lib
LIBS    = -ldevhook -lpsppower

LDFLAGS = -nostdlib -nodefaultlibs

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

