#
#devicehook API lib for devkitPSP7
#
TARGET_LIB = ../lib/libdevhook.a

OBJS = \
 dhAPI_1000.o\
 dhAPI_1001.o\
 dhAPI_1002.o\
 dhAPI_1003.o\
 dhAPI_1004.o\
 dhAPI_1005.o\
 dhAPI_1006.o\
 dhAPI_1007.o\
 dhAPI_1008.o\
 dhAPI_1009.o\
 dhAPI_100A.o\
 dhAPI_100B.o\
 dhAPI_1100.o\
 dhAPI_1101.o\
 dhAPI_1102.o\

USE_KERNEL_LIBC = 1

INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

$(OBJS): dhAPI.S
	$(CC) -DF_$* $< -c -o $@

