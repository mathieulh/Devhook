	.set noreorder

#include "pspimport.s"

#ifdef F_pspAPI_1000
        IMPORT_START	"DevHookForKernel",0x00010000
#endif
#ifdef F_dhAPI_1001
        IMPORT_FUNC	"DevHookForKernel",0xc72f2944,dhGetVersion
#endif
#ifdef F_dhAPI_1002
        IMPORT_FUNC	"DevHookForKernel",0x3acbe058,dhGetConfig
#endif
#ifdef F_dhAPI_1003
        IMPORT_FUNC	"DevHookForKernel",0x37a665be,dhSetConfig
#endif
#ifdef F_dhAPI_1004
        IMPORT_FUNC	"DevHookForKernel",0x07fd07ed,dhUMDAddDevice
#endif
#ifdef F_dhAPI_1005
        IMPORT_FUNC	"DevHookForKernel",0x5abf4545,dhUMDDelDevice
#endif

#ifdef F_dhAPI_1006
        IMPORT_FUNC	"DevHookForKernel",0x077d1281,dhReadFileRetry
#endif
#ifdef F_dhAPI_1007
        IMPORT_FUNC	"DevHookForKernel",0xef08a4ad,dhWriteFileRetry
#endif
#ifdef F_dhAPI_1008
        IMPORT_FUNC	"DevHookForKernel",0x99873c8c,dhCloseFile
#endif

#ifdef F_dhAPI_1009
        IMPORT_FUNC	"DevHookForKernel",0x6356C255,dhUMDMount
#endif
#ifdef F_dhAPI_100A
        IMPORT_FUNC	"DevHookForKernel",0xD5836409,dhUMDUnmount
#endif

#ifdef F_dhAPI_100B
        IMPORT_FUNC	"DevHookForKernel",0x3347c22f,dhCLKSet
#endif

///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////

#ifdef F_dhAPI_0000
        IMPORT_START	"DevHookForUser",0x40090000
#endif
#ifdef F_dhAPI_0001
        IMPORT_FUNC	"DevHookForUser",0xc72f2944,dhGetVersion
#endif
#ifdef F_dhAPI_0002
        IMPORT_FUNC	"DevHookForUser",0x3acbe058,dhGetConfig
#endif
#ifdef F_dhAPI_0003
        IMPORT_FUNC	"DevHookForUser",0x37a665be,dhSetConfig
#endif
#ifdef F_dhAPI_0004
        IMPORT_FUNC	"DevHookForUser",0x6356C255,dhUMDMount
#endif
#ifdef F_dhAPI_0005
        IMPORT_FUNC	"DevHookForUser",0xD5836409,dhUMDUnmount
#endif

///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////
#ifdef F_dhAPI_1000
        IMPORT_START	"DHRegistryForKernel",0x00010000
#endif
#ifdef F_dhAPI_1101
        IMPORT_FUNC	"DHRegistryForKernel",0x0b138f6b,dhGetRegistry
#endif
#ifdef F_dhAPI_1102
        IMPORT_FUNC	"DHRegistryForKernel",0x744c7eed,dhSetRegistry
#endif

///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////
#ifdef F_dhAPI_0100
        IMPORT_START	"DHRegistryForUser",0x40090000
#endif
#ifdef F_dhAPI_0101
        IMPORT_FUNC	"DHRegistryForUser",0x0b138f6b,dhGetRegistry
#endif
#ifdef F_dhAPI_0102
        IMPORT_FUNC	"DHRegistryForUser",0x744c7eed,dhSetRegistry
#endif

