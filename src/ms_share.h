
#define MODE_DOPEN 0xffffffff


#define MSS_ERR_OTHER_DEVICE 0x8fffffff

int ms_share_open(PspIoDrvFileArg *arg,const char *file,int flags,SceMode mode);
int ms_share_close(PspIoDrvFileArg *arg);
int ms_share_read(PspIoDrvFileArg *arg,void *buf,int size);
int ms_share_write(PspIoDrvFileArg *arg,const void *buf,int size);
SceOff ms_share_seek(PspIoDrvFileArg *arg, u32 unk, SceOff ofs, int whence);
int ms_share_dread(PspIoDrvFileArg *arg, SceIoDirent *dir);

int ms_share_flush(PspIoDrvFileArg *arg);

/*
	setup(re-open) & get fd
*/
int ms_share_get_fd(PspIoDrvFileArg* arg);
int ms_share_check_fd(PspIoDrvFileArg* arg);

/*
	call after change file pointer (seek/read/write)
*/
//int ms_share_set_filepos(PspIoDrvFileArg* arg);
/*
	close all handle
*/
void ms_share_close_all(void);

/*
	install ms fd share driver
*/
int ms_share_install(void);

void ms_share_shutdown(void);
