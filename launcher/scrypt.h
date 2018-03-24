
struct script_cmd
{
	const char *name;
	char *(*func)(int argc,char **argv);
};

u32 str2val(char *str);
void *loadFileAlloc(const char *path,int *mid,int *fsize);

int script_execBuf(char *buf);
int script_execFile(const char *path);

int save_launcher_setting(const char *path);
