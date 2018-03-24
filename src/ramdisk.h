
// RAMDISK header
typedef struct ramdisk_header
{
	struct ramdisk_header *prev; // +00
	struct ramdisk_header *next; // +04
	int size;					 // +08
	int memid;					 // +0C
	char name[0x30];			 // +10 name
}RD_HDR;

int ramdisk_save_file(const char *fw_dir,const char *name);

void *ramdisk_search(const char *name,int *psize);

void ramdisk_collect(void *new_ptr);

int ramdisk_rename(const char *old_path,const char *new_path);
int build_ramdisk_file(const char *fw_dir,char *file);
int build_ramdisk(const char *fw_dir,const char *conf_tbl);

//
void ramdisk_set_top_addr(void *addr,int size);
void *ramdisk_get_top_addr(void);

void ramdisk_load_after_boot(void);
