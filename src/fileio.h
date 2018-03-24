#ifndef __FILEIO__
#define __FILEIO__

typedef struct file_struct
{
  void (*put)(struct file_struct *,char data);
  char (*get)(struct file_struct *);
  char *wp;
  char *rp;
} FILE_;

extern char path_buf[256];

char *make_path(char *buf,const char *boot_path,const char *target_name);
int get_path_len(const char *path);

/*************************************
waiting device ready
*************************************/
int wait_device(const char *dev_path,int timeout_10msec);

/*************************************
open file in multi path
*************************************/
const char *get_path_one(char *buf,const char *path_list);

SceUID sceIoOpen_multipath(const char *path_list,const char *file, int flags, SceMode mode);

#endif
