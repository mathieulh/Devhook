/*
  devhook config controll

  KprintfÇ™msrebootÇ≈ìÆÇ©Ç»Ç¢Ç±Ç∆Ç…íçà”
  
 */
#include "common.h"

/****************************************************************************
****************************************************************************/
#define MAX_CONFIG_AREA 8192

static u8 config_buf[MAX_CONFIG_AREA];
static u32 config_size = 0;

#ifndef MSREBOOT_MODE
static int semaid = 0;
#endif

/****************************************************************************
****************************************************************************/
static inline void lock(void)
{
#ifndef MSREBOOT_MODE
	sceKernelWaitSema(semaid, 1, 0);
#endif
}
static inline void unlock(void)
{
#ifndef MSREBOOT_MODE
	sceKernelSignalSema(semaid, 1);
#endif
}

/****************************************************************************
****************************************************************************/
static u8 *search_name(const char *name)
{
	u8 *p  = config_buf;
	u8 *ep = &config_buf[config_size];
	int nlen,dlen;
	u8 *next;
	int slen = strlen(name);

	while(p<ep)
	{
		// compare name
		nlen = p[0];
		dlen = p[1+nlen];
		next = p+1+nlen+1+dlen;
		if( (nlen==slen) && memcmp(name,p+1,nlen)==0 ) return p;
		p = next;
	}
	return NULL;
}

/****************************************************************************
****************************************************************************/
int dhGetRegistry(const char *name,void *data,int size)
{
	u8 *p  = config_buf;
//	u8 *ep = &config_buf[config_size];

	int nlen,dlen;

//Kprintf("dhGetRegistry(%s,%08X,%d):",name,(u32)data,size);

	lock();
#if 1
	p = search_name(name);
	if(p)
	{
		nlen = p[0];
		dlen = p[1+nlen];
		if(data && size>0)
		{
			memcpy(data,p+1+nlen+1,size);
		}
		unlock();
//Kprintf("DL=%d\n",dlen);
		return 0;
	}
#else
	while(p<ep)
	{
		// compare name
		nlen = p[0];
		dlen = p[1+nlen];
	
		if(memcmp(name,p+1,nlen)==0)
		{
			if(data && size>0)
			{
				memcpy(data,p+1+nlen+1,size);
			}
			unlock();
//Kprintf("DL=%d\n",dlen);
			return 0;
		}
		p += 1+nlen+1+dlen;
	}
#endif
	unlock();
//Kprintf("no-data\n");
	return -1;
}

/****************************************************************************
****************************************************************************/
int dhSetRegistry(const char *name,const void *data,int size)
{
	u8 *p  = config_buf;
	u8 *ep = &config_buf[config_size];
	u8 *next;
	int nlen,dlen;

//Kprintf("dhSetRegistry(%s,%08X,%d)\n",name,(u32)data,size);

	lock();
#if 1
	p = search_name(name);
	if(p)
	{
		nlen = p[0];
		dlen = p[1+nlen];

		// delete data
		next = p+1+nlen+1+dlen;
		memmove(p,next,ep-next);
		config_size -= next-p;
	}
#else
	while(p<ep)
	{
		// compare name
		nlen = p[0];
		dlen = p[1+nlen];
		next = p+1+nlen+1+dlen;

		if(memcmp(name,p+1,nlen)==0)
		{
			// delete data
			next = p+1+nlen+1+dlen;
			memmove(p,next,ep-next);
			config_size -= next-p;
			break;
		}
		p = next;
	}
#endif
	if(data && size>0)
	{
		nlen = strlen(name);
		config_buf[config_size] = nlen;
		memcpy(&config_buf[config_size+1],name,nlen);

		config_buf[config_size+1+nlen]   =  size;

		config_size += 1+nlen+1;

		memcpy(&config_buf[config_size],data,size);
		config_size += size;
	}
	unlock();

	return 0;
}

/****************************************************************************
****************************************************************************/
#define DH_MAGIC1 ('D'|('H'<<8)|('0'<<16)|('4'<<24))
#define DH_MAGIC2 0x12345678

void dhLoadRegistry(void *buf)
{
//Kprintf("dhLoadRegistry(%08X)\n",(u32)buf);

#ifndef MSREBOOT_MODE
	// create lock semaphore
	if(semaid==0)
	{
		semaid = sceKernelCreateSema("hdreg", 0, 1, 1, 0);
	}
#endif

	if( (*(u32 *)(buf+0) != DH_MAGIC1) ||
	    (*(u32 *)(buf+4) != DH_MAGIC2))
	{
//Kprintf("no dh Registry data\n");
		return;
	}
	buf += 8;

	config_size = *(u32 *)buf;
	buf += 4;
	memcpy(config_buf,buf,config_size);
}

/****************************************************************************
****************************************************************************/
int dhSaveRegistry(void *buf)
{
//Kprintf("dhSaveRegistry(%08X)\n",(u32)buf);

	*(u32 *)(buf+0) = DH_MAGIC1;
	*(u32 *)(buf+4) = DH_MAGIC2;
	buf += 8;

	*(u32 *)buf = config_size;
	buf += 4;
	memcpy(buf,config_buf,config_size);
	return config_size;
}

