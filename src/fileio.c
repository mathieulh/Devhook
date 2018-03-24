/*
	PSP FILE CONTROLL
*/

#include "common.h"

/****************************************************************************
****************************************************************************/
char path_buf[256];

/****************************************************************************
	path handling
****************************************************************************/
char *make_path(char *buf,const char *boot_path,const char *target_name)
{
  int pos,dpos;
  char ch;

  //clear first
  for(pos=0;pos<128;pos++) path_buf[pos]=0;

  pos = dpos = 0;
  do
  {
    ch = *boot_path++;
    buf[pos++] = ch;
    if(ch=='/') dpos = pos;
  }while(ch!=0);

	if(target_name==0)
	{
		buf[dpos++] = 0;
		return buf;
	}

  do
  {
    ch = *target_name++;
    buf[dpos++] = ch;
  }while(ch!=0);
  return buf;
}

/****************************************************************************
****************************************************************************/
int get_path_len(const char *path)
{
  int pos = 0;
  while(*path++ != 0)
    pos++;
  return pos;
}

/****************************************************************************
	File Access Core
****************************************************************************/
struct psp_file_param
{
	DH_FILE *fp;
	SceOff pos;
	void *buf;
	int size;
	int wmode;
};

/****************************************************************************
****************************************************************************/
static int dhFileRetry(struct psp_file_param *param)
{
	int cnt;
	int result = 0x8000000;

	DH_FILE *fp = param->fp;
	SceOff pos  = param->pos;
	void *buf   = param->buf;
	int size    = param->size;
	int wmode   = param->wmode;

//Kprintf("AF-ENTER\n");
	for(cnt=0;cnt<fp->retry_cnt;cnt++)
	{
		// open
		if(fp->fd < 0)
		{
//Kprintf("AF-OPEN\n");
			fp->fd = result = sceIoOpen(fp->path,fp->flags,fp->mode);

			if(result < 0)
			{
				switch(result)
				{
				case 0x80010002: // file not found
					return result;
#if 0
				case 0x80020321: // suspend/resume
#define ERR_UNKNOWN_DEVICE 0x80020321
#endif
				}

				// with retry error
				goto retry2;
			}
		}

		// seek
		result = sceIoLseek32(fp->fd,(int)pos,PSP_SEEK_SET);
//Kprintf("AF-SEEK\n");
		if(result < 0) goto retry1;

		if(result != (int)pos) return 0x80000000;

		if(size==0) return 0; /* SEEK ONLY */

		if(wmode)
		{
//Kprintf("AF-WRITE\n");
			result = sceIoWrite(fp->fd,buf,size);
			if(result > 0)
			{
				return result;
			}
		}
		else
		{
		// read
//Kprintf("AF-READ\n");
			result = sceIoRead(fp->fd,buf,size);
			if(result > 0)
			{
				return result;
			}
		}
retry1:
		sceIoClose(fp->fd);
		fp->fd = -1;
retry2:
		sceKernelDelayThread(fp->retry_interval);
	}
	return result;
}

/****************************************************************************
	Read File
****************************************************************************/
int dhReadFileRetry(DH_FILE *fp,SceOff pos,void *buf,int size)
{
	struct psp_file_param param;
	int stack_size;

	param.fp    = fp;
	param.pos   = pos;
	param.buf   = buf;
	param.size  = size;
	param.wmode = 0;
	stack_size = fp->stack_size;
	if(stack_size)
		return (int)sceKernelExtendKernelStack(stack_size,(void *)dhFileRetry,(void *)(&param) );
	return dhFileRetry(&param);
}

/****************************************************************************
	Write File
****************************************************************************/
int dhWriteFileRetry(DH_FILE *fp,SceOff pos,void *buf,int size)
{
	struct psp_file_param param;
	int stack_size;

	param.fp    = fp;
	param.pos   = pos;
	param.buf   = buf;
	param.size  = size;
	param.wmode = 1;
	stack_size = fp->stack_size;
	if(stack_size)
		return (int)sceKernelExtendKernelStack(stack_size,(void *)dhFileRetry,(void *)(&param) );
	return dhFileRetry(&param);
}

/****************************************************************************
	Close File
****************************************************************************/
int dhCloseFile(DH_FILE *fp)
{
	if(fp->fd >= 0)
	{
		sceIoClose(fp->fd);
		fp->fd = SCE_KERNEL_ERROR_NODEV;
	}
	return 0;
}

/****************************************************************************
	Waiting Device Ready
****************************************************************************/
int wait_device(const char *dev_path,int timeout_10msec)
{
	int fd;

	// device‚ª‚ª‹N“®‚·‚é‚Ü‚Å‘Ò‚Â
	while( (timeout_10msec-=5) > 0)
	{
		fd = sceIoDopen(dev_path);
		if(fd>=0)
		{
//Kprintf("device %s ready\n",dev_path);
			sceIoDclose(fd);
			return 0;
		}
//Kprintf("waiting for device %s ready\n",dev_path);
		// 10msec wait
		sceKernelDelayThread(50000);
	}
	// timeout
Kprintf("device %s not ready\n",dev_path);
	return -1;
}

/****************************************************************************
	get one of path in path list
****************************************************************************/
const char *get_path_one(char *buf,const char *path_list)
{
	char c;

	*buf=0;
	if(path_list==NULL) return NULL;

	while(1)
	{
		*buf = c = *path_list++;

		if(c==';')
		{
			*buf = 0x00;
			return path_list;
		}
		if(c==0x00) break;
		buf++;
	}
	// last of pathlist
	return NULL;
}

/****************************************************************************
	Waiting Device Ready
****************************************************************************/
SceUID sceIoOpen_multipath(const char *path_list,const char *file, int flags, SceMode mode)
{
	char path[128];
	const char *plist;
	int plen;
	SceUID result;

	//
	plist=path_list;
	while(plist!=NULL)
	{
		plist = get_path_one(path,plist);
		plen = strlen(path);
		// '???/'+'/???' -> '???/???'
		if(plen>0 && path[plen-1]=='/' && file[0]=='/') plen--;
		// add file name
		strcpy(&path[plen],file);

		result = sceIoOpen(path,flags,mode);
//Kprintf("TRY MULTI OPEN '%s':%08X\n",path,result);
		if(result > 0) return result;
	}
	return result;
}
