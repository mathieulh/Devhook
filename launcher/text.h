#ifndef __PRINTF_H__
#define __PRINTF_H__

//#define printf  pspDebugScreenPrintf
//#define locate  pspDebugScreenSetXY

#define printf  text_printf
#define locate  text_setXY

#define TEXT_CENTER -1
#define TEXT_CURR   -2

#define CENTER_X(S) (get_center_sx(S))
#define CONSOLE_WIDTH  text_get_width()
#define CONSOLE_HEIGHT text_get_height()

void text_clear(void);

int text_printf(const char *format,...);
int text_printfXY(int x,int y,const char *format,...);

void text_setXY(int x,int y);

void text_set_fc(int color);
void text_set_bc(int color);

int text_get_width(void);
int text_get_height(void);

int text_init(const char *sfont_path,const char *dfont_path);
int text_term(void);


#endif
