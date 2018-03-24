/****************************************************************************
****************************************************************************/

#include "common.h"


/****************************************************************************
****************************************************************************/
static int fbm_install = 0;


#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 216

// PSPSDK debugprint
#define DEBUG_PRINTF_FONT_WIDTH  7
#define DEBUG_PRINTF_FONT_HEIGHT 6
static int fcolor = 0xffffff;
static int bcolor = 0x000000;

void text_clear(void)
{
	pspDebugScreenClear();
	pspDebugScreenSetXY(0,1);
	if(fbm_install)
		fbm_printCR(0,0,"",fcolor,bcolor,FBM_FONT_FILL | FBM_BACK_FILL, 100);
}

/****************************************************************************
****************************************************************************/
static char msg_buf[1024];

int text_printf(const char *format,...)
{
	int ret;
	va_list arg;

	va_start(arg,format);
	ret = vsprintf(msg_buf,format,arg);
	if(fbm_install)
		fbm_printCR(-1,-1,msg_buf,fcolor,bcolor,FBM_FONT_FILL | FBM_BACK_FILL, 100);
	else
		pspDebugScreenPrintf("%s",msg_buf);
	va_end(arg);
	return ret;
}

int text_printfXY(int x,int y,const char *format,...)
{
	int ret;
	va_list arg;

	va_start(arg,format);
	ret = vsprintf(msg_buf,format,arg);

	if(x==TEXT_CENTER) x=get_center_sx(msg_buf); // centering
	if(fbm_install)
	{
		fbm_printCR(x,y,msg_buf,fcolor,bcolor,FBM_FONT_FILL | FBM_BACK_FILL, 100);
	}
	else
	{
		pspDebugScreenSetXY(x,y+1);
		pspDebugScreenPrintf("%s",msg_buf);
	}
	va_end(arg);
	return ret;
}

/****************************************************************************
****************************************************************************/
void text_setXY(int x,int y)
{
	if(fbm_install)
	{
		fbm_printCR(x,y,"",fcolor,bcolor,FBM_FONT_FILL | FBM_BACK_FILL, 100);
	}
	else
	{
		if(x>=0 && y>=0)
			pspDebugScreenSetXY(x,y+1);
	}
	
}

/****************************************************************************
****************************************************************************/
void text_set_fc(int color)
{
	fcolor = color;
	pspDebugScreenSetTextColor(color);
}

void text_set_bc(int color)
{
	bcolor = color;
	pspDebugScreenSetBackColor(color);
}

/****************************************************************************
****************************************************************************/
int get_center_sx(char *msg)
{
	int msg_width;
	int font_width;

	if(fbm_install)
	{
		msg_width  = fbm_getwidth(msg);
		font_width = SCREEN_WIDTH / fbmMaxCol;
	}
	else
	{
		font_width = DEBUG_PRINTF_FONT_WIDTH;
		msg_width  = strlen(msg)*font_width;
	}

	return (SCREEN_WIDTH-msg_width)/font_width/2;
}

/****************************************************************************
****************************************************************************/
int text_get_width(void)
{
	if(fbm_install) return fbmMaxCol;
	return SCREEN_WIDTH / DEBUG_PRINTF_FONT_WIDTH;
}

int text_get_height(void)
{
	if(fbm_install) return fbmMaxRow;
	return (SCREEN_HEIGHT-16) / DEBUG_PRINTF_FONT_HEIGHT;
}

/****************************************************************************
****************************************************************************/
int text_init(const char *sfont_path,const char *dfont_path)
{
	if (fbm_init(sfont_path,dfont_path, 1) >=0)
	{
		fbm_install = 1;
		return 0;
	}
	fbm_freeall();
	fbm_install = 0;
	return -1;
}

int text_term(void)
{
	fbm_freeall();
	fbm_install = 0;
	return 0;
}

