/*
  devhook config controll

  KprintfÇ™msrebootÇ≈ìÆÇ©Ç»Ç¢Ç±Ç∆Ç…íçà”
  
 */
#include "common.h"

/****************************************************************************
	default setting
****************************************************************************/
const char *inital_value[] = {
///////////////////////////////////////
// devhook setting
///////////////////////////////////////
	"SFO_VER","0",
	"KPRINT_UART","0",
	"CLOCK","000/000",
	"PRELOAD_ADDR","0x00000000",
	"FLASH0","",
	"FLASH1","",
	"UMD_PATH","",
	"REBOOT_PATH","",
	"BTCNF_PATH","/kd/pspcnf_tbl_dh.txt",
///////////////////////////////////////
// vshex setting
///////////////////////////////////////
	"VSHEX_AUTOMENU","1",
	"VSHEX_UMD_DELAY","1",
///////////////////////////////////////
// launcher setting
///////////////////////////////////////
	"AUTOEXEC","autoexec.txt",
//
	NULL
};

/****************************************************************************
	default messages
****************************************************************************/
const char *lang_value[] = {
	"STR_TITLE"			,"PSP DEVHOOK Ver.0.52 by BOOSTER",
//
	"STR_UMD_DISC"		,"UMD Disc",
	"STR_VM_NOCHANGE"	,"no change   ",
	"STR_VM_2XX200"		,"2.xx to 2.00",

	"STR_CK_NORMAL"		,"  no change   ",
/*************************************
  information
*************************************/
	"STR_BAT_NOCONNECT"	,"no-connect",
	"STR_BAT_UNKLNOWN"	,"unknown",

	"STR_STS_BOOTMODE"	,"BOOTMODE",
	"STR_STS_UMDISO"	,"UMD-FILE",
	"STR_STS_UMDMODE"	,"UMD-SFO ",
	"STR_STS_BTCONF"	,"CONF TBL",
	"STR_STS_REBOOT"	,"REBOOT  ",
	"STR_STS_FLASH"		,"FLASH0,1",
	"STR_STS_CLOCK"		,"CLOCK   ",
	"STR_STS_REMOTE"	,"REMOTE  ",
	"STR_STS_PREADDR"	,"PL-ADDR ",

	"STR_STS_DHVER"		,"DH Ver. ",
	"STR_STS_FWVER"		,"FW ver. ",
	"STR_STS_BATTERY"	,"BATTERY ",

	"STR_RC_KPRINT"		,"KPRINTF(115kbps)",
	"STR_RC_NOKPRINT"	,"REMOTE CTRL",

/*************************************
  message
*************************************/
	"STR_STS_KEYS"		,"CIRCLE:SELECT  TRIANGLE:CANCEL     HOME:MENU ON/OFF (XMB)",
	"STR_AUTORUN_WAIT"	,"auto start after %2d sec. PUSH CIRCLE to open menu",
	"STR_AUTORUN"		,"                AUTO START                    ",
/****************************************************************************
	dh_inst.c
****************************************************************************/
  "STR_FLASH0_INST_MSG"	,"'%s' is installed in flash0.",
  "STR_FLASH0_NONE_MSG"	,"devhook isn't installed in flash0:",
  "STR_FLASH1_INST_MSG"	,"'%s' is installed in flash1.",
  "STR_FLASH1_NONE_MSG"	,"devhook isn't installed in flash1:",

  "STR_FLASH_DUP_WARNING","\
flash0: and flash1: can install an only one firmware version.\n\
If there is another version already, remove before install.\n",

  "STR_FLASH0_WARNING","\
This operation write flash0: device.\n\
flash0 isn't written in normal PSP operation.\n\
When failing in the writing to flash0:, there is danger which PSP never boot.\n\
The supported firmware is only version 1.50 without any MOD.\n\
Connect both of AC power and the battery.\n\
And Don't touch PSP absolutely until this operation finish.\n\
\n\
Press CIRCLE button and hold 5seconds to start\n\
Press other button to cancel\n",

  "STR_FLASH1_WARNING","\
This operation write flash1: device.\n\
flash1 was written in normal PSP operation.\n\
But PSP never boot when that the registry file can't initialize.\n\
Connect both of AC power and the battery.\n\
And Don't touch PSP absolutely until this operation finish.\n\
\n\
Press CIRCLE button and hold 1seconds to start\n\
Press other button to cancel\n",

  "STR_FLASH_NO_WARNING","\
This operation doesn't write to built-in FlashROM.\n\
\n\
Press CIRCLE button and hold 1second to start\n\
Press other button to cancel\n",

  "STR_FLASH1_BACKUP"	,"Backup config files\n'%s' to '%s'",
  "STR_FLASH1_WORK"		,"Install config files\n'%s' to '%s'",
  "STR_FLASH1_CLEANUP"	,"Cleanup,remove all devhook files from\n'%s%s'",

  "STR_FLASH0_FONT"		,"Install font\n'%s' to '%s'",
  "STR_FLASH0_KDRES"	,"Install kd/rsource (for Korean(949) char set)\n'%s' to '%s'",
  "STR_FLASH0_CLEANUP"	,"Cleanup,remove all devhook files from\n'%s%s'",
//
  "STR_FLASH_CANCEL"	,"canceled",
  "STR_FLASH_DONE"		,"Pres any button to return",
//
  "STR_PSAR_INSTALL"	,"Setup Firmware files from PSAR Dumper output",
  "STR_START_BTN"		,"Press CIRCLE to START , other buttons to CANCEL",
//
	NULL
};

/****************************************************************************
****************************************************************************/
#define MAX_CONFIG_AREA 32768

static u8 config_buf[MAX_CONFIG_AREA];
static u32 config_size = 0;

/****************************************************************************
****************************************************************************/
static inline void lock(void) {}
static inline void unlock(void) {}

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
		dlen = p[1] + (p[2]<<8);
		next = p+4+nlen+dlen;
		if( (nlen==slen) && memcmp(name,p+4,nlen)==0 ) return p;
		p = next;
	}
	return NULL;
}

/****************************************************************************
****************************************************************************/
int GetRegistry(const char *name,void *data,int size)
{
	u8 *p  = config_buf;
//	u8 *ep = &config_buf[config_size];

	int nlen,dlen;

	lock();

	p = search_name(name);
	if(p)
	{
		nlen = p[0];
		dlen = p[1] + (p[2]<<8);
		if(data && size>0)
		{
			memcpy(data,p+4+nlen,size);
		}
		unlock();
		return 0;
	}
	unlock();
	return -1;
}

/****************************************************************************
****************************************************************************/
int GetRegistryDWORD(const char *name,u32 *data)
{
	char tmp_buf[16];
	int result = GetRegistry(name,tmp_buf,sizeof(tmp_buf));
	if(result < 0) return result;

	*data = str2val(tmp_buf);
	return 4;
}

/****************************************************************************
****************************************************************************/
int SetRegistry(const char *name,const void *data,int size)
{
	u8 *p  = config_buf;
	u8 *ep = &config_buf[config_size];
	u8 *next;
	int nlen,dlen;

	lock();

	p = search_name(name);
	if(p)
	{
		nlen = p[0];
		dlen = p[1] + (p[2]<<8);

		// delete data
		next = p+4+nlen+dlen;
		memmove(p,next,ep-next);
		config_size -= next-p;
	}

	if(data && size>0)
	{
		nlen = strlen(name);
		config_buf[config_size]    = nlen;
		config_buf[config_size+1]  =  size;
		config_buf[config_size+2]  =  size>>8;
		memcpy(&config_buf[config_size+4],name,nlen);

		config_size += 4+nlen;
		memcpy(&config_buf[config_size],data,size);
		config_size += size;
	}
	unlock();

	return 0;
}

int SetRegistryStr(const char *name,const void *data)
{
//Kprintf("SetRegistryStr(%s,%s)\n",name,data);
	return SetRegistry(name,data,strlen(data)+1);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int SetRegistryDWORD(const char *name,u32 data)
{
	char buf[16];
	sprintf(buf,"0x%08X",data);
	return SetRegistryStr(name,buf);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int SetRegistryInt(const char *name,int data)
{
	char buf[16];
	sprintf(buf,"%d",data);
	return SetRegistryStr(name,buf);
}

/*******************************************************************************
get/set string name value
*********************************************************************************/
u32 get_value(char *name)
{
	char buf[128];
	u32 val = 0;

	// get default position
	sprintf(buf,"VAL_%s",name);
	GetRegistryDWORD(buf,&val);
	return val;
}

void set_value(char *name,int val)
{
	char buf[128];
	// set default position
	sprintf(buf,"VAL_%s",name);
	SetRegistryInt(buf,val);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void InitRegistryLang(void)
{
	int i;
	for(i=0;lang_value[i];i+=2)
	{
		SetRegistryStr(lang_value[i],lang_value[i+1]);
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void InitRegistry(void)
{
	int i;

	for(i=0;inital_value[i];i+=2)
	{
		SetRegistryStr(inital_value[i],inital_value[i+1]);
	}

	InitRegistryLang();
}

