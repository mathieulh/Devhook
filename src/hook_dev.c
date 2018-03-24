/*
  PSP I/O device hook
*/
#include "common.h"

#define __VERBOSE__ 0

#define LOG_FOUND 0
#define LOG_ERR   0

/////////////////////////////////////////////////////////////////////////////
// pointer area check
/////////////////////////////////////////////////////////////////////////////
static int area_check(unsigned int pos,unsigned int base,unsigned int range)
{
  if(pos&3) return 0;
  if(pos < 0x88000000)  return 0;
  if(pos > 0x88400000)  return 0;
//  if(pos > (base+range)) return 0;
//  if(pos < (base-range)) return 0;
  return 1;
}

/////////////////////////////////////////////////////////////////////////////
// string compare with NULL
/////////////////////////////////////////////////////////////////////////////
static int strcmp_n(const char *p1,const char *p2)
{
	if(p1==NULL && p2==NULL) return 0;
	if(p1==NULL || p2==NULL) return -1;

	return strcmp(p1,p2);
}

/////////////////////////////////////////////////////////////////////////////
// search DEVICE IO BLOCK in kernel memory
/////////////////////////////////////////////////////////////////////////////
static int is_string(const char *s)
{
	// NULL ?
	if(s==NULL) return 1;

	// kernel area check
	if(!area_check( (unsigned int)s  , (int)s , 0x400000)) return 0;

	while(*s)
	{
		char c = *s++;
		switch(c)
		{
		case '%':
		case '$':
		case '\r':
		case '\n':
			break;
		default:
		  if( (c<0x20) || (c<0)) return 0;
		}
	}
	return 1;
}

static int is_device_block(PspIoDrv *dev)
{
  int i;

  if(!area_check( (unsigned int)dev->funcs , (int)dev , 0x20000)) return 0;

	for(i=0;i<16;i++)
	{
		unsigned int entry = ((unsigned int *)dev->funcs)[i];
		if(entry && !area_check(entry,(int)dev, 0x20000) ) return 0;
	}

	if(!is_string(dev->name))  return 0;
	if(!is_string(dev->name2)) return 0;

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// search DEVICE IO BLOCK in kernel memory
/////////////////////////////////////////////////////////////////////////////
PspIoDrv *search_device_io(const char *dev_name,int block_size,int flags,const char *drv_name)
{
	unsigned int kernel_mem_base = 0x88000000;
	unsigned int kernel_mem_size = 0x00300000;

#if LOG_FOUND
	Kprintf("search_device_io\n");
#endif

	while(kernel_mem_size>sizeof(PspIoDrv))
	{
		PspIoDrv *dev = (PspIoDrv *)kernel_mem_base;

	if(
		(dev->block_size == block_size) && 
		(dev->dev_type == flags)        &&
		is_device_block(dev)            &&
		(strcmp_n(dev_name,dev->name)==0) &&
		1)
//      (strcmp_n(drv_name,dev->name2)==0) )
    {
#if LOG_FOUND
 Kprintf("DEVICE IO block found %08X[%s,%X,%X,%s,%08X]\n",(int)kernel_mem_base,
  dev->name,dev->dev_type,dev->block_size,dev->name2?dev->name2:"NULL",(int)dev->funcs);
#endif
        return dev;
    }
    kernel_mem_base+=4;
    kernel_mem_size-=4;
  }
#if LOG_ERR
Kprintf("DEVICE IO block %s not found \n",dev_name);
#endif
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// HOOK DEVICE entry table
/////////////////////////////////////////////////////////////////////////////
PspIoDrvFuncs *hook_device(const char *dev_name,int block_size,int flags,const char *drv_name,IO_DEVICE_CTRL_ENTRY *entry_table)
{
  PspIoDrv *device;
  IO_DEVICE_CTRL_ENTRY *org_table;

  device = search_device_io(dev_name,block_size,flags,drv_name);
  if(device==NULL)
  {
    return NULL;
  }
  /* hook entry & return original */ 
  org_table     = device->funcs;
  device->funcs = entry_table;
  clear_cache();
  return org_table;
}
