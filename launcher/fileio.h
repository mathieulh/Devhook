#ifndef __FILEIO__
#define __FILEIO__

/*************************************
open file in multi path
*************************************/
const char *make_path_one(char *buf,const char *path_list,const char *fname);
SceUID sceIoOpen_multipath(const char *path_list,const char *file, int flags, SceMode mode);

#endif
