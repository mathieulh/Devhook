/*
	PSP devhook launcher script player

	compatible as devhook 0.51.0000
*/

#include "common.h"

////////////////////////////////////////////////////////////////////////////////
// external COMMAND list
////////////////////////////////////////////////////////////////////////////////
extern const struct script_cmd command_list[];

/////////////////////////////////////////////////////////////////////////////
// launcher save registrys
/////////////////////////////////////////////////////////////////////////////
enum
{
	TYPE_STR,
	TYPE_DEC,
	TYPE_DWORD,
};

typedef struct registry_set
{
	const char *name;
	u8 type;
}REG_LIST;

static const REG_LIST registry_save_list[] = {
{"SFO_VER"					,TYPE_DEC},
{"KPRINT_UART"				,TYPE_DEC},
{"CLOCK"					,TYPE_DEC},
{"PRELOAD_ADDR"				,TYPE_DWORD},
{"FLASH0"					,TYPE_STR},
{"FLASH1"					,TYPE_STR},
{"UMD_PATH"					,TYPE_STR},
{"REBOOT_PATH"				,TYPE_STR},
{"BTCNF_PATH"				,TYPE_STR},
//
{"VSHEX_AUTOMENU"			,TYPE_DEC},
{"VSHEX_UMD_DELAY"			,TYPE_DEC},
//
{"AUTOEXEC"					,TYPE_STR},
{"AUTO_RUN_TIME"			,TYPE_DEC},
//
{NULL,0}
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static char *buf_getl(char *ptr,char *buf,int size)
{
	int p = 0;
	u8 ch;
	u8 quartation = 0;

	while(1)
	{
		buf[p] = 0x00; // eom
		ch = (u8)(*ptr);
		if(ch==0x00) return ptr; // end
		if(quartation)
		{
			if(ch==quartation) quartation = 0;
		}
		else
		{
			if(ch==0x0a) break;
			else if( (ch=='\'') || (ch=='\"') ) quartation = ch;
		}

		// output (CR+LF > LF)
		if((ch!=0x0d) && (p<size) ) buf[p++] = (char)ch;

		ptr++;
	}

	// top of nextline
	if(*ptr && ((u8)*ptr)<0x20) ptr++;
	return ptr;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int separate_param(char *buf,char **params,int size)
{
	int num_params = 0;
	int separater;
	int quart_mark;

	separater = '=';
	while(num_params < size)
	{
		// get value
		while(*buf==0x20) buf++;

		// store parameter
		params[num_params++] = buf;

		// check quartation mark , end mark
		switch(*buf)
		{
		case 0x00:
			break;
		case '\"':
		case '\'':
			quart_mark = *buf++;
			while(*buf)
			{
				if(*buf==quart_mark)
				{
					buf++;
					break;
				}
				buf++;
			}
		}
		// search next separater
		while(*buf)
		{
			if(*buf==separater || *buf==',') break;
			buf++;
		}
		if(*buf==0x00) break;

		// separate param
		*buf++ = 0x00;
		separater = ',';
	}

	return num_params;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static const char hex_list[16]="0123456789abcdef";
u32 str2val(char *str)
{
	u32 sum;
	u32 val;
	char ch;
	char *p;
	int k;
	int nimus = 0;

	while(*str==' ') str++;

	// nimus
	if(str[0]=='-')
	{
		nimus = 1;
		str++;
	}
	else if(str[0]=='+')
		str++;

	// dec / hex
	k = 10;
	if(str[0]=='0' && str[1]=='x')
	{
		str+=2;
		k = 16;
	}

	sum = 0;
	while(1)
	{
		ch = *str++;
		p = strchr(hex_list,ch);
		if(p==NULL) break;
		val = (p-hex_list);
		if(val >= k) break;

		sum = sum * k + val;
	}

	return nimus ? -sum : sum;
}

/////////////////////////////////////////////////////////////////////////////
// get dec/hex from string
/////////////////////////////////////////////////////////////////////////////
void str_set(char *dst,char *src)
{
	strcpy(dst,src);
}

/////////////////////////////////////////////////////////////////////////////
// get parameter from "REG_NAME=VAL1,VAL2" 
/////////////////////////////////////////////////////////////////////////////
void set_registry_line(int argc,char **argv)
{
	// set direct num
	if(argc>=2)
	{
		// set local registry
		SetRegistryStr(argv[0],argv[1]);
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int strlen4(const char *str)
{
	int size = strlen(str)+1;
	return ((size+3)>>2)<<2;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static char *external_command(int argc,char **argv)
{
	const struct script_cmd *cmd_list;
	char *name = argv[0];
	char *(*func)(int,char **);
#if 0
{
	int i;
	Kprintf("CMD[%s]",argv[0]);
	for(i=1;i<argc;i++) Kprintf(",[%s]",argv[i]);
	Kprintf("\n");
}
#endif
	for(cmd_list=command_list;cmd_list->name;cmd_list++)
	{
		if(strcmp(cmd_list->name,name)==0)
		{
			// KEEP kernel are address
			func = (void *)( ((u32)cmd_list->func) | 0x80000000);
			return func(argc,argv);
		}
	}

	text_printf("Unknown Script Command %s\n",name);
	Kprintf("Unknown Script Command %s\n",name);
	return "";
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#define MAX_PARAM 8
#define CMD_STACK_SIZE 4096

int script_execBuf(char *buf)
{
	char *ptr,*next_ptr;
	char *param[MAX_PARAM];
	int num_param;
	const char *result;
	int i;
	char *param_buf;
	int mid;
	char *stack;
	char *p;
	char chr;

	// allocate heap
	mid = sceKernelAllocPartitionMemory(2, "cmd_stack", 0, CMD_STACK_SIZE, NULL);
	if(mid<0) return -1;
	stack = (char *)sceKernelGetBlockHeadAddr(mid);

	ptr = buf;
	while(1)
	{
		// get line
		next_ptr = buf_getl(ptr,stack,CMD_STACK_SIZE);
		if(ptr==next_ptr) break;

		param_buf = stack+strlen4(stack);

		// separate each values from line string
		num_param = separate_param(stack,param,MAX_PARAM);

		// replace parameter from command function
		// and strip quartation
		for(i=num_param-1;i>=1;i--)
		{
			switch(chr=param[i][0])
			{
			case ':':
				param[i]++; // remove ':'
				result = external_command(num_param-i,param+i);
				if(result==NULL) goto command_err;

				// replace param with result
				strcpy(param_buf,result);
				param[i] = param_buf;
				param_buf += strlen(param_buf)+1;
				break;
			case '\"':
			case '\'':
				// strip quartation
				param[i]++;
				p = strrchr(param[i],chr);
				if(p) *p=0x00;
				break;
			}
		}

		// set registry
		switch(param[0][0])
		{
		case '#': // comment
			break;
		case ':': // external command
			param[0]++; // remove ':' in command name
			external_command(num_param,param);
			break;
		case '!': // section name == end of current section
			goto exit;
		default: // REGISTRY=VALUE
			set_registry_line(num_param,param);
		}
command_err:
		// next line
		ptr = next_ptr;
	}
exit:
	sceKernelFreePartitionMemory(mid);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void *loadFileAlloc(const char *path,int *mid,int *fsize)
{
	int fd;
	int result;
	char *file_buf;
	int file_size;

	fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
	if(fd<0) return NULL;

	file_size = sceIoLseek32(fd,0,PSP_SEEK_END);
	if(file_size<0)
	{
		sceIoClose(fd);
		return NULL;
	}

	*mid = sceKernelAllocPartitionMemory(2, "file_buf", 0, file_size+1, NULL);
	if(mid<0)
	{
		sceIoClose(fd);
		return NULL;
	}
	file_buf = (char *)sceKernelGetBlockHeadAddr(*mid);
	memset(file_buf,0,file_size+1);

	// load setting filr to buf
	sceIoLseek32(fd,0,PSP_SEEK_SET);
	result = sceIoRead(fd,file_buf,file_size);
	sceIoClose(fd);
	if( (result<0) || (result > file_size) )
	{
		sceKernelFreePartitionMemory(*mid);
		return NULL;
	}
	if(fsize) *fsize = file_size;

	return (void *)file_buf;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int script_execFile(const char *path)
{
	int mid;
	char *file_buf;
	int result;

	// load file
	file_buf = loadFileAlloc(path,&mid,NULL);
	if(file_buf==NULL) return -1;
	// exec file
	result = script_execBuf(file_buf);

	sceKernelFreePartitionMemory(mid);
	return result;
}

/////////////////////////////////////////////////////////////////////////////
// save launcher setting file with script format
/////////////////////////////////////////////////////////////////////////////
int save_launcher_setting(const char *path)
{
	int fd;
	int result;
	const REG_LIST *p;
	char param_buf[256];
	char save_buf[256];

	fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if(fd<0) return -1;

	for(p=registry_save_list;p->name;p++)
	{
		// get registry value
		memset(param_buf,0,sizeof(param_buf));
		GetRegistry(p->name,param_buf,sizeof(param_buf));

		switch(p->type)
		{
		case TYPE_DEC:
		case TYPE_DWORD:
			sprintf(save_buf,"%s=%s\n",p->name,(char *)(param_buf));
			break;
		case TYPE_STR:
			sprintf(save_buf,"%s=\"%s\"\n",p->name,(char *)(param_buf));
			break;
		}
		result = sceIoWrite(fd, save_buf, strlen(save_buf));
	}
	sceIoClose(fd);

	return 0;
}
