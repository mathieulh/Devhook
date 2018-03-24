/*
  RAM DISK for reboot

1.ドライブ名指定:フルパス
  ファイルパスに':'が無い時はパス

2.[dhコモンパス]
  "ms0:/dh"や"flash0:/dh"を指す

3.flash0:の[エミュパス](cache用）
  エミュパスに':'が無い時はパス

4.[エミュパス]
  MSとflashへのパス

5. 1.50Fの時、"flash0:"から読む

*/

#include "common.h"

#define OUT_LOG_OUT  0
#define OUT_LOG_FILE 0
#define OUT_LOG_ERR  0
#define OUT_LOG_DBG  0

#define AFTER_LOAD 0

#define FORCE_MEMORY 0x89000000
//#define FORCE_MEMORY2 0x89840000
//#define USE_VRAM

//////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////

#if OUT_LOG_OUT
#define log_out Kprintf
#else
#define log_out(text,...) {}
#endif

#if OUT_LOG_FILE
#define log_file Kprintf
#else
#define log_file(text,...) {}
#endif

#if OUT_LOG_ERR
#define log_err Kprintf
#else
#define log_err(text,...) {}
#endif


#if OUT_LOG_DBG
#define log_dbg Kprintf
#else
#define log_dbg(text,...) {}
#endif


//////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////
static const char rd_name[] = "ramdisk";

static RD_HDR *rd_top = NULL;

//////////////////////////////////////////////////////////////////
// allocate memory
//
// p2 : 08804000-
// p4 : 08830000-
// p3 : 08830000-

//////////////////////////////////////////////////////////////////

#ifdef FORCE_MEMORY2
static u32 force_alloc_top = 0;
#endif

#ifdef FORCE_MEMORY
static void *force_alloc_top = (void *)FORCE_MEMORY;
static u32 force_alloc_size = 0;
#else
  #ifdef USE_VRAM
    static void *force_alloc_top = NULL;
  #endif
#endif

static void *alloc_memory(const char *name,u32 mpid,int size)
{
	void *ptr;
#ifdef FORCE_MEMORY
	// align 64
	size = ((size+63)/64)*64;

	if(force_alloc_size < size)
	{
#ifdef USE_VRAM
		if(force_alloc_top >= 0x88000000)
		{
			// VRAM
			force_alloc_top  = (void *)0x84000000;
			force_alloc_size = 0x00080000;
//			sceGeEnd();
//			sceDisplayDisable();
//			sceDisplayEnd();
//			sceGeInit();
//			sceDisplayInit();
		}
		else
#endif
#if 1
		while(mpid)
		{
//Kprintf("ALLOC %d\n",mpid);
			int memid =  sceKernelAllocPartitionMemory(mpid & 0x0f,rd_name,PSP_SMEM_Low, size, NULL);
			if(memid>0)
			{
				ptr = (void *)( ((u32)sceKernelGetBlockHeadAddr(memid))|0x80000000 );
Kprintf("ramdisk alloc %d:%08X %s\n",mpid&0x0f,(int)ptr,name);
				return ptr;
			}
			// next try
			mpid >>= 4;
		}
Kprintf("ramdisk can't alloc\n");
		return NULL;
#endif
		// forece memory
		force_alloc_top  = (void *)0x89200000;
		force_alloc_size = 8*1024*1024;
	}

	// ramdisk_set_top_addr()が呼ばれたときだけ～
	if(force_alloc_top)
	{
		// align 64
//		tmp = ((int)force_alloc_top) & 0x3f;
//		if(tmp) force_alloc_top += 0x40-tmp;

		// メモリ取得して次のポインタへ
		ptr = force_alloc_top;
		force_alloc_top += size;
		force_alloc_size -= size;

//Kprintf("allocate %08X:%s\n",(int)ptr,name);

		return ptr;
	}
#else

#ifdef FORCE_MEMORY2
	if(force_alloc_top)
	{
		// direct map
		RD_HDR *cur;
		int ta;
		cur = (RD_HDR *)force_alloc_top;
again:
		ta = ((u32)cur) - size;
		ta &= 0xffffff40; // align 64
//Kprintf("Try Direct map %08X\n",ta);
		for(cur=rd_top;cur;cur = cur->next)
		{
			int ca = (u32)cur & 0x0fffffff;
			if( ta > ca)
			{
				if( (ca+cur->size) > ta ) goto again;
			}else if( ta < ca)
			{
				if( (ta+size) > ca ) goto again;
			}else goto again; // same = err
		}
		// okay
		force_alloc_top = ta;
		return (void *)ta;
	}
#endif
	int memid;

	while(mpid)
	{
//Kprintf("ALLOC %d\n",mpid);
		memid =  sceKernelAllocPartitionMemory(mpid & 0x0f,rd_name,PSP_SMEM_Low, size, NULL);
		if(memid>0)
		{
			ptr = (void *)( ((u32)sceKernelGetBlockHeadAddr(memid))|0x80000000 );
//Kprintf("ramdisk alloc %d:%08X %s\n",mpid,(int)ptr,name);
			return ptr;
		}
		// next try
		mpid >>= 4;
	}
#ifdef FORCE_MEMORY2
	force_alloc_top = (void *)FORCE_MEMORY2;
	return alloc_memory(name,0,size);
#endif

#endif
Kprintf("ramdisk can't allocate memory for %s\n",name);
	return NULL;
}

//////////////////////////////////////////////////////////////////
// search text
//////////////////////////////////////////////////////////////////
static char *get_next_line(char *ptr)
{
	char c;

	while(1)
	{
		c = *ptr;
		if(c==0) return NULL; // EOD
		if(c<0x20) break;
		ptr++;
	}
	*ptr = 0x00; // line EOL
	return ptr+1;
}

//////////////////////////////////////////////////////////////////
//	search ramdisk file
//////////////////////////////////////////////////////////////////
void *ramdisk_search(const char *name,int *psize)
{
	RD_HDR *cur;

//Kprintf("search %s :",name);

	for(cur=rd_top ; cur ; cur=cur->next)
	{
		if(strcmp(cur->name,name)==0)
		{
			if(psize)
				*psize = cur->size;

log_dbg("ramdisk found '%s'\n",name);

			return (void *)(cur+1); // data pointer
		}
	}
	// not found
log_err("ramdisk not found '%s'\n",path_buf);

	return NULL;
}

//////////////////////////////////////////////////////////////////
//	regist ramdisk file
//////////////////////////////////////////////////////////////////
int ramdisk_regist(RD_HDR *reg,const char *name,int size)
{
	RD_HDR *cur;
	u32 reg_addr,cur_addr;

	// ファイル名登録
	strcpy(reg->name,name);
	reg->size = size;

//Kprintf("regist %08X:%s size %d\n",(int)reg,reg->name,reg->size);

	if(rd_top==NULL)
	{
#if 0
		// メモリ確保テスト
		alloc_memory("dummy",0x02,2*1024*1024);
#endif
		//登録無し
		rd_top    = reg;
		reg->prev = NULL;
		reg->next = NULL;
		return 0;
	}

	reg_addr = (((u32)reg & 0xf8000000)==0x88000000) ? (u32)reg : 0xf0000000;

	for(cur=rd_top ; cur ; cur=cur->next)
	{
//Kprintf("Scan %s\n",cur->name);
		// userメモリのポインタ順にソート
		cur_addr = (((u32)cur & 0xf8000000)==0x88000000) ? (u32)cur : 0xf0000000;

		if( reg_addr < cur_addr )
			break;

		if(cur->next==NULL)
		{
			// 最後の後ろ
			reg->prev = cur;
			reg->next = NULL;
			cur->next = reg;
			return 0;
		}
	}

	// 頭か、中間
	if(cur->prev==NULL)
	{
		rd_top    = reg;
	}
	reg->prev = cur->prev;
	cur->prev = reg;
	reg->next = cur->next;
	cur->next = reg;
	return 0;
}

//////////////////////////////////////////////////////////////////
//	ガベージコレクション
//////////////////////////////////////////////////////////////////
void ramdisk_collect(void *new_ptr)
{
	RD_HDR *cur,*next;
	int size;

//Kprintf("\n\ngavage collect\n");

	cur=rd_top;
	while(cur)
	{
		// change chain
		if(cur->prev)
			cur->prev->next = (RD_HDR *)new_ptr;
		else
			rd_top = (RD_HDR *)new_ptr;

		next = cur->next;
		if(next)
			next->prev = (RD_HDR *)new_ptr;

		// total size
		size = sizeof(RD_HDR) + cur->size;
		// align 64
		if(size & 0x3f) size += 0x40-(size & 0x3f);

//Kprintf("MOVE %08X[%05X] > %08X : %s\n",(int)cur,size,(int)new_ptr,cur->name);

		// move data
		memmove(new_ptr,cur,size);
		new_ptr += size;

		cur = next;
	}
	clear_cache();
}

//////////////////////////////////////////////////////////////////
// open file
//////////////////////////////////////////////////////////////////
static int ramdisk_open_file(const char *fw_dir,const char *name,int flash0_flag)
{
	// if include drive later , check physical path only
	if(strchr(name,':'))
	{
		strcpy(path_buf,name);
log_dbg("TRY '%s'\n",path_buf);
		return sceIoOpen(path_buf, PSP_O_RDONLY, 0x777);
	}

	// multi pass search
	return sceIoOpen_multipath(fw_dir,name,PSP_O_RDONLY,0777);
}

//////////////////////////////////////////////////////////////////
// ファイルをRAMDISKに登録
//////////////////////////////////////////////////////////////////
#ifdef USE_VRAM
void *filebuf=NULL;
#endif

static int ramdisk_save_file_core(const char *fw_dir,const char *name,int noload,int flash0_flag)
{
	int fd;
	int bytes_read = 0;
	unsigned char *ptr;
	int file_size;
	int mem_size;
	RD_HDR *reg;

#ifdef USE_VRAM
	// vram用tempがなければ作成
	if(!filebuf)
	{
		filebuf = (RD_HDR *)alloc_memory( name , 0x0201,0x8000);
	}
#endif

	// 重複ファイルをbypass
	ptr = (unsigned char *)ramdisk_search(name,&file_size);
	if(ptr) return 0; /* already loaded */

	log_file("Load RAM DISK '%s','%s' to %08X\n",fw_dir,name,(int)ptr);

	// open
	fd = ramdisk_open_file(fw_dir,name,flash0_flag);
	if(fd<0)
	{
Kprintf("RAMDISK %s open error %08X\n",name,fd);
		return -1; // ファイルエラー
	}

	// ファイルサイズ取得
	file_size = sceIoLseek32(fd,0,PSP_SEEK_END);
	reg = NULL;
	if(file_size < 0)
	{
Kprintf("RAMDISK %s seek error\n",name);
		sceIoClose(fd);
		return -2; // シークエラー
	}
	sceIoLseek32(fd,0,PSP_SEEK_SET);

	// メモリサイズ
	mem_size = noload ? sizeof(RD_HDR) : sizeof(RD_HDR)+file_size;

	// メモリブロック割り当て
	reg = (RD_HDR *)alloc_memory( name , 0x0302,mem_size);
//	if(reg==NULL)
	if( (u32)reg < 0x88000000)
	{
#ifdef USE_VRAM
		// VRAMに配置、ロード
		int tmp;
		if(!noload)
		{
			// バッファ経由でロード
			file_size = 0;
			for(tmp=sizeof(RD_HDR);;tmp += 0x8000)
			{
				bytes_read = sceIoRead(fd, filebuf, 0x8000);
				if( bytes_read < 0)
				{
					sceIoClose(fd);
					return -4; // リードエラー
				}
				if(bytes_read==0) break;
#if 1
				memcpy(((void *)reg) + tmp , filebuf, bytes_read);
#else
{
	int j;
	for(j=0;j<bytes_read;j+=4)
	{
		*(u32 *)(((u32)reg) + tmp + j) = (u32 *)(((u32)filebuf)+j);
	}
}
#endif
				file_size += bytes_read;
			}
			bytes_read = file_size;
		}
#else
		sceIoClose(fd);
		return -3; // メモリ割り当てエラー
#endif
	}
	else
	{
		if(!noload)
		{
log_dbg("read %08X\n",(int)(ptr+0x40));
			bytes_read = sceIoRead(fd, reg+1, file_size);
		}
	}
	sceIoClose(fd);

	if(noload)
	{
Kprintf("RAMDISK %s bypass\n",name);
		reg->memid = 0xffffffff; // noload mark
		return ramdisk_regist(reg,name,file_size);
	}

	if(bytes_read != file_size)
	{
Kprintf("RAMDISK %s read error\n",name);
		return -4; // リードエラー
	}

log_file("READ %s %08X,%5X\n",path_buf,(int)(ptr+0x40),bytes_read);

	// regist data
	return ramdisk_regist(reg,name,file_size);
}

/****************************************************************************
	ファイルをRAMDISKにセーブ
****************************************************************************/
int ramdisk_save_file(const char *fw_dir,const char *name)
{
	return ramdisk_save_file_core(fw_dir,name,0,0);
}

/****************************************************************************
	RAMDISKのりネーム
****************************************************************************/
int ramdisk_rename(const char *old_path,const char *new_path)
{
	RD_HDR *old;

//Kprintf("Rename %s > %s\n",old_path,new_path);

	old = (RD_HDR *)ramdisk_search(old_path,NULL);
	if(old==NULL) return -1;
	old--;

//Kprintf("Found %s\n",old->name);

	strcpy(old->name,new_path);
	return 0;
}

/****************************************************************************
	config textと、その中味のファイルをRAMDISKにロード
****************************************************************************/
int build_ramdisk_file(const char *fw_dir,char *file)
{
	char *ptr,*next;
	char c;
	int file_size;
	int noload = 0;

	log_file("----- Cache configfile '%s' -----\n",file);

	// pspcnf_tbl.txtをRAMDISKに登録
	if( ramdisk_save_file_core(fw_dir,file,0,1) <= -3)
	{
		return -1; // メモリ or リードエラー
	}

	// pspcnf_tbl.txtのポインタ取得
	ptr = (char *)ramdisk_search(file,&file_size);
	if(!ptr)
	{
		log_out("Can't open %s\n",file);
		return -1;
	}

	ptr[file_size] = 0;
	while( (next = get_next_line(ptr)) != NULL)
	{
		c = 0;
		while(1)
		{
			c = *ptr;
			if(c=='$' || c=='%' || c==' ')
				ptr++;
			else break;
		}
		if( (c!='#') && (c!=0))
		{
#if 1
			// copy without hash code
			char fname[128];
			char *ptr2 = fname;
			while(*ptr > 0x20)
			{
				*ptr2++ = *ptr++;
			}
			*ptr2 = 0;
			if(ramdisk_save_file_core(fw_dir,fname,noload,1) <= -3)
#else
			if(ramdisk_save_file_core(fw_dir,ptr,noload,1) <= -3)
#endif
			{
				return -1; // メモリ or リードエラー
			}
#if AFTER_LOAD
			// devhookが来たら、以後、ファイルは読まない
			if( strstr(ptr,"devhook.prx") != NULL)
			{
				noload = 1;
			}
#endif
		}
		ptr = next;
	}
	log_file("----- finish configfile '%s' -----\n",file);

	return 0;
}
/****************************************************************************
	RAMDISKの構築
****************************************************************************/
int build_ramdisk(const char *fw_dir,const char *conf_tbl)
{
	char c;
	char *ptr,*next;
	int file_size;

	// pspcnf_tbl.txtをRAMDISKに登録
	ramdisk_save_file_core(fw_dir,conf_tbl,0,1);
	// pspcnf_tbl.txtのポインタ取得
	ptr = (char *)ramdisk_search(conf_tbl,&file_size);

	if(!ptr)
	{
		log_out("Can't open %s\n",path_buf);
		return -1;
	}
	log_out("Build RAM DISK '%s'\n",fw_dir);

#if 0
	// ~PSPなら、flash bootと考える
	if( (u32 *)ptr = 0x5053507e) // ~PSP
	{
		return 1;
	}
#endif

	ptr[file_size] = 0;
	while( (next = get_next_line(ptr)) != NULL)
	{
		// ptr = ptr;

		// 空白の次を頭だし
		while(1)
		{
			c = *ptr;
			if(c<=0x20) break;
			ptr++;
		}
		if(c==0x20)
		{
			ptr++;
			// 各configテキストと、その中味をRAMDISKに登録
			if( build_ramdisk_file(fw_dir,ptr) < 0)
			{
				return -1;
			}
		}
		ptr = next;
	}

	clear_cache();

	log_out("RAM DISK loaded\n");
	return 0;
}

/****************************************************************************
****************************************************************************/
void ramdisk_set_top_addr(void *addr,int size)
{
#ifdef FORCE_MEMORY
	force_alloc_top = (void *)addr;
	force_alloc_size = size;
#endif
}

/****************************************************************************
****************************************************************************/
void *ramdisk_get_top_addr(void)
{
	return (void *)rd_top;
}

/****************************************************************************
	後読みprxのロード
****************************************************************************/
void ramdisk_load_after_boot(void)
{
#if AFTER_LOAD
	RD_HDR *cur = (RD_HDR *)RAMDISK_NOLOAD_LIST;
	char path[128];
	sprintf(path,"%s%s",dhGetRegistry("FLASH_PATH"),MS_FLASH0_PATH);

	// prev == vaid mark 0x5555aaaa
	// next == pointer of load address

	for(cur = (RD_HDR *)RAMDISK_NOLOAD_LIST ; cur->prev==(RD_HDR *)RAMDISK_NOLOAD_MAGIC ; cur++)
	{
		Kprintf("after load %s %s %08X[%08X]\n",path,cur->name,(int)cur->next,cur->size);

		int fd = ramdisk_open_file(path,cur->name,1);
		if(fd > 0)
		{
			int readed = sceIoRead(fd, cur->next, cur->size);
			sceIoClose(fd);
Kprintf("after loaded %08X\n",readed);
		}
	}
#endif
	clear_cache();
}
