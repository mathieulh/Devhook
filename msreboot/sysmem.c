/*
  PSP sysmem patch for msreboot.bin
*/

#include "common.h"

/****************************************************************************
****************************************************************************/
extern void (*Kprintf)(const char *format,...);

/****************************************************************************
****************************************************************************/
unsigned int *get_kprintf_entry(void *func)
{
	unsigned int *sysmem_cseg;

	sysmem_cseg =(unsigned int *)0x88000000;;
	// 1.50
	if(
		(sysmem_cseg[0x0000b550/4] == 0x27BDFFD0) &&
		(sysmem_cseg[0x0000b554/4] == 0xAFB20008)
	)
	{
		Kprintf("sysmem is FW1.50\n");
		return &sysmem_cseg[0x0000b550/4];
	}
	// 2.00
	if(
		(sysmem_cseg[0x0000B868/4] == 0x27BDFFC0) &&
		(sysmem_cseg[0x0000B86C/4] == 0xAFB3000C)
	)
	{

		Kprintf("sysmem is FW2.00\n");
		return &sysmem_cseg[0x0000b868/4];
	}
	// 2.50
	if(
		(sysmem_cseg[0x0000d0bc/4] == 0x27BDFFC0) &&
		(sysmem_cseg[0x0000d0c0/4] == 0xAFB3000C)
	)
	{
		Kprintf("sysmem is FW2.50\n");
		return &sysmem_cseg[0x0000d0bc/4];
	}
	// 2.60
	if(
		(sysmem_cseg[0x00010C78/4] == 0x01031007)
	)
	{
		Kprintf("sysmem is FW2.60\n");
		return &sysmem_cseg[0x0000E4BC/4];
	}
	// 2.1
	if(
		(sysmem_cseg[0x00010988/4] == 0x01041007)
	)
	{
		Kprintf("sysmem is FW2.71\n");
		return &sysmem_cseg[0x0000D78C/4];
	}
	// 2.80
/*
Kprintf         0000DC14
M0_ModuleInfo : 11348
*/
	if(
		(sysmem_cseg[0x00011348/4] == 0x01041007)
	)
	{
		Kprintf("sysmem is FW2.80\n");
		return &sysmem_cseg[0x0000DC14/4];
	}
	// 3.00/3.01
	if(
		(sysmem_cseg[0x00010F88/4] == 0x01041007) // M0_ModuleInfo
	)
	{
		Kprintf("sysmem is FW3.00/3.01\n");
		return &sysmem_cseg[0x0000D978/4];
	}

	// 1.00
	sysmem_cseg =(unsigned int *)0x88000000;;
	if(
		(sysmem_cseg[0x00007D4C/4] == 0x27BDFFD0) &&
		(sysmem_cseg[0x00007D50/4] == 0xAFB20008)
	)
	{
		return &sysmem_cseg[0x00007d4c/4];
		Kprintf("sysmem is FW1.00\n");
	}
	Kprintf("unknown sysmem\n");
	return 0;
}
