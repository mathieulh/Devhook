#-----------------------------------------------
# DEVHOOK sample launcher makefile
#
# use DevkitPSP release 7
#-----------------------------------------------
TARGET = DH052
DUMMY_TARGET = NULL
OBJS = \
  main.o \
  command.o\
  dh_load.o \
  dh_inst.o \
  ui.o \
  stubuk.o \
  text.o \
  fileio.o \
  registry.o \
  script.o\
  ../mok/fbm_print.o \

USE_PSPSDK_LIBC = 1

INCDIR = 
CFLAGS = -O2 -G0 -Wall

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti

ASFLAGS = $(CFLAGS)

#LIBS   = -ldevhook -lpspdebug -lpsplibc -lpspkernel -lpspumd -lpsppower
LIBS   =  -ldevhooku -lpspdebug -lpsppower
LIBDIR =  ../lib
#LDFLAGS = -T script/pspbtm2.x
LDFLAGS = -T script/psptop.x

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = DEVHOOK0.52 launcher
PSP_EBOOT_ICON = icon/icon0.png
#PSP_EBOOT_UNKPNG = icon/pic1.png

PSPSDK=$(shell psp-config --pspsdk-path)
#include build.mak
include $(PSPSDK)/lib/build.mak

LIBS += -lpsphprm_driver

