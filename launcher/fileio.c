/*
	PSP FILE CONTROLL
*/

#include "common.h"

/****************************************************************************
	get one of path in path list
****************************************************************************/
const char *make_path_one(char *buf,const char *path_list,const char *fname)
{
	char c;
	int i = 0;

	buf[0]=0;
	if(path_list==NULL) return NULL;

	while(1)
	{
		c = *path_list++;
		if(c==';')
		{
			break;
		}
		if(c==0x00)
		{
			path_list = NULL; // last of pathlist
			break;
		}
		buf[i++]= c;
	}

	if(fname)
	{
		// add file
		if( (i>0) && (buf[i-1]=='/')  && (fname[0]=='/') )
			fname++;

		strcpy(&buf[i],fname);
	}
	else
	{
		// only directry
		buf[i]=0;
	}

	// lext path list
	return path_list;
}

/****************************************************************************
	Waiting Device Ready
****************************************************************************/
SceUID sceIoOpen_multipath(const char *path_list,const char *file, int flags, SceMode mode)
{
	char path[128];
	const char *plist;
	SceUID result = -1;

	//
	plist=path_list;
	while(plist!=NULL)
	{
		plist = make_path_one(path,plist,file);
		result = sceIoOpen(path,flags,mode);
//Kprintf("TRY MULTI OPEN '%s':%08X\n",path,result);
		if(result > 0) return result;
	}
	return result;
}
