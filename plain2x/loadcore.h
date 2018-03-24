#ifndef __LOADCORE_H_
#define __LOADCORE_H_
/*
	PSP LOADCORE patch database
*/

//#define HOOK_LOADEXECUTABLE
//#define HOOK_PROBEEXEC
//#define SHOW_CHECKEXEC_INFO

typedef struct patch_loadcore
{
#ifdef MSREBOOT_MODE
	u32 moduleStart_OFFSET;
#endif
	u32 sceKernelModInfo_OFFSET;
	u32 sceKernelModInfo_VER;
// CheckExecFile
	u32 sceKernelCheckExecFile_LIB;
	u32 sceKernelCheckExecFile_CALL1;
	u32 sceKernelCheckExecFile_CALL2;
	u32 sceKernelCheckExecFile_CALL3;
#ifdef PLAIN2X_MODE
// ProbeExecutableObject
	u32 sceKernelProbeExecutableObject_LIB;
	u32 sceKernelProbeExecutableObject_OFFSET;
// LoadCoreForKernel_Unkonow_54ab2675 : Type 7 check switch table address
	u32 switch_table_54ab2675;
#ifdef HOOK_LOADEXECUTABLE
// sceKernelLoadExecutableObject
	u32 sceKernelLoadExecutableObject_LIB;
#endif
#endif // PLAIN2X_MODE
	u32 KernelMode_SYSCALL_link;
//
	const char *name;
}LC_PP;

/****************************************************************************
	version number
****************************************************************************/
enum{
	LOADCORE_200,
	LOADCORE_250,
	LOADCORE_260,
	LOADCORE_271,
	LOADCORE_28x,
	LOADCORE_30x,
//
	MAX_LCPP
};
/****************************************************************************
	patch point database
****************************************************************************/
static const LC_PP patch_point[MAX_LCPP] = {
//	FW2.00
{
//
#ifdef MSREBOOT_MODE
	0x00000BF0, // u32 moduleStart_OFFSET;
#endif
	0x00006a28, // u32 sceKernelModInfo_OFFSET;
	0x01023007, // u32 sceKernelModInfo_VER;
// CheckExecFile
	0x00006b8c, // u32 sceKernelCheckExecFile_LIB;
	0x00001d6c, // u32 sceKernelCheckExecFile_CALL1;
	0x00001dc0, // u32 sceKernelCheckExecFile_CALL2;
	0x00003820, // u32 sceKernelCheckExecFile_CALL3;
#ifdef PLAIN2X_MODE
// ProbeExecutableObject
	0x00006b90, // u32 sceKernelProbeExecutableObject_LIB;
	0x00003498, // u32 sceKernelProbeExecutableObject_OFFSET;
// LoadCoreForKernel_Unkonow_54ab2675 : Type 7 check switch table address
	0x000000000, // 2.00Ç≈ÇÕÉGÉâÅ[Ç≈Ç»Ç¢
#ifdef HOOK_LOADEXECUTABLE
// sceKernelLoadExecutableObject
	0x000006b94, // u32 sceKernelLoadExecutableObject_LIB;
#endif
#endif // PLAIN2X_MODE
	0x00000000, // u32 KernelMode_SYSCALL_link
	"2.00",
},
/*
	FW 2.50
*/
{
//
#ifdef MSREBOOT_MODE
	0x00000BF0, // u32 moduleStart_OFFSET;
#endif
	0x00006f18, // u32 sceKernelModInfo_OFFSET;
	0x01023007, // u32 sceKernelModInfo_VER;
// CheckExecFile
	0x00007084, // u32 sceKernelCheckExecFile_LIB;
	0x00001da0, // u32 sceKernelCheckExecFile_CALL1;
	0x00001df0, // u32 sceKernelCheckExecFile_CALL2;
	0x00003ba4, // u32 sceKernelCheckExecFile_CALL3;
#ifdef PLAIN2X_MODE
// ProbeExecutableObject
	0x00007088, // u32 sceKernelProbeExecutableObject_LIB;
	0x00003824, // u32 sceKernelProbeExecutableObject_OFFSET;
// LoadCoreForKernel_Unkonow_54ab2675 : Type 7 check switch table address
	0x00007C34, // u32 switch_table_54ab2675
#ifdef HOOK_LOADEXECUTABLE
// sceKernelLoadExecutableObject
	0x000006b94,// u32 sceKernelLoadExecutableObject_LIB;
#endif
#endif // PLAIN2X_MODE
	0x00000000, // u32 KernelMode_SYSCALL_link
	"2.50",
},

/*
	FW 2.60
*/
{
//
#ifdef MSREBOOT_MODE
	0x00000BC4, // u32 moduleStart_OFFSET;
#endif
	0x00006C10, // u32 sceKernelModInfo_OFFSET;
	0x01033007, // u32 sceKernelModInfo_VER;
// CheckExecFile
	0x00006DC4, // u32 sceKernelCheckExecFile_LIB;
	0x00001F9C, // u32 sceKernelCheckExecFile_CALL1;
	0x00001FEC, // u32 sceKernelCheckExecFile_CALL2;
	0x00004110, // u32 sceKernelCheckExecFile_CALL3;
#ifdef PLAIN2X_MODE
// ProbeExecutableObject
	0x00006DF4, // u32 sceKernelProbeExecutableObject_LIB;
	0x00003D90, // u32 sceKernelProbeExecutableObject_OFFSET;
// LoadCoreForKernel_Unkonow_54ab2675 : Type 7 check switch table address
	0x00007A18, // u32 switch_table_54ab2675
#ifdef HOOK_LOADEXECUTABLE
// sceKernelLoadExecutableObject
	0x00006DBC, // u32 sceKernelLoadExecutableObject_LIB;
#endif
#endif // PLAIN2X_MODE
	0x00000000, // u32 KernelMode_SYSCALL_link
	"2.60"
},
/*
	FW 2.71
*/
{
//
#ifdef MSREBOOT_MODE
	0x00000B84, // u32 moduleStart_OFFSET;
#endif
	0x000067F0, // u32 sceKernelModInfo_OFFSET;
	0x01033007, // u32 sceKernelModInfo_VER;
// CheckExecFile
	0x000069AC, // u32 sceKernelCheckExecFile_LIB;
	0x00001EC4, // u32 sceKernelCheckExecFile_CALL1;
	0x00001F14, // u32 sceKernelCheckExecFile_CALL2;
	0x00003CC0, // u32 sceKernelCheckExecFile_CALL3;
#ifdef PLAIN2X_MODE
// ProbeExecutableObject
	0x000069E0, // u32 sceKernelProbeExecutableObject_LIB;
	0x00003914, // u32 sceKernelProbeExecutableObject_OFFSET;
// LoadCoreForKernel_Unkonow_54ab2675 : Type 7 check switch table address
	0x00007040,   // u32 switch_table_54ab2675
#ifdef HOOK_LOADEXECUTABLE
// sceKernelLoadExecutableObject
	0x000069A4, // u32 sceKernelLoadExecutableObject_LIB;
#endif
#endif // PLAIN2X_MODE
	0x00000000, // u32 KernelMode_SYSCALL_link
	"2.71"
},

/*
	FW 2.80 / 2.82
*/
{
//
#ifdef MSREBOOT_MODE
	0x00000BB8, // u32 moduleStart_OFFSET;
#endif
	0x00006A20, // u32 sceKernelModInfo_OFFSET;
	0x01033007, // u32 sceKernelModInfo_VER;
// CheckExecFile
	0x00006BDC, // u32 sceKernelCheckExecFile[_LIB;
	0x00001EF8, // u32 sceKernelCheckExecFile_CALL1;
	0x00001F48, // u32 sceKernelCheckExecFile_CALL2;
	0x00003DAC, // u32 sceKernelCheckExecFile_CALL3;
#ifdef PLAIN2X_MODE
// ProbeExecutableObject
	0x00006C10, // u32 sceKernelProbeExecutableObject_LIB;
	0x00003A08, // u32 sceKernelProbeExecutableObject_OFFSET;
// LoadCoreForKernel_Unkonow_54ab2675 : Type 7 check switch table address
	0x000072e4, // u32 switch_table_54ab2675
#ifdef HOOK_LOADEXECUTABLE
// sceKernelLoadExecutableObject
	0x00006BD4, // u32 sceKernelLoadExecutableObject_LIB;
#endif
#endif // PLAIN2X_MODE
	0x00003008, // u32 KernelMode_SYSCALL_link
	"2.8x"
},

/*
	FW 3.01
*/
{
//
#ifdef MSREBOOT_MODE
	0x00000BB8, // u32 moduleStart_OFFSET;
#endif
	0x00006C40, // u32 sceKernelModInfo_OFFSET;
	0x01043007, // u32 sceKernelModInfo_VER;
// CheckExecFile
	0x00006e10, // u32 sceKernelCheckExecFile_LIB;
	0x0000143c, // u32 sceKernelCheckExecFile_CALL1;
	0x0000148c, // u32 sceKernelCheckExecFile_CALL2;
	0x00003b80, // u32 sceKernelCheckExecFile_CALL3;
#ifdef PLAIN2X_MODE
// ProbeExecutableObject
	0x00006e44, // u32 sceKernelProbeExecutableObject_LIB;
	0x000037DC, // u32 sceKernelProbeExecutableObject_OFFSET;
// LoadCoreForKernel_Unkonow_54ab2675 : Type 7 check switch table address
	0x0000751c, // u32 switch_table_54ab2675
#ifdef HOOK_LOADEXECUTABLE
// sceKernelLoadExecutableObject
	0x00006BD4, // u32 sceKernelLoadExecutableObject_LIB;
#endif
#endif // PLAIN2X_MODE
	0x00003390, // u32 KernelMode_SYSCALL_link
	"3.01"
},

};

#endif
