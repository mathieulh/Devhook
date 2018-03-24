/*
  user interface
*/
#include "common.h"

/////////////////////////////////////////////////////////////////////////////
// UI
/////////////////////////////////////////////////////////////////////////////
int get_key(void)
{
  SceCtrlData pad;
  unsigned int button;

  //sceCtrlReadBufferPositive(&pad, 1);
  sceCtrlReadBufferPositive((SceCtrlData *)(&pad), 1);
  button = pad.Buttons & ALL_CTRL; 

  return pad.Buttons;
}

int wait_key(void)
{
  int key;
  while(1)
  {
    if(!( get_key() & SENSE_KEY)) break;
    sceKernelDelayThread(20000);
  }
  while(1)
  {
    if( (key=get_key()) & SENSE_KEY) break;
    sceKernelDelayThread(20000);
  }

  return key;
}

int hold_button(int buttons_mask,int buttons,int timeout_msec)
{
	while(timeout_msec >0)
	{
		if( (get_key() & buttons_mask) != buttons)
			return -1; // failed
		sceKernelDelayThread(20*1000);
		timeout_msec -= 20;
	}
	// Success
	return 0;
}

int wait_hold_button(int buttons,int timeout_msec)
{
	if( (wait_key()&SENSE_KEY) != buttons) return -1;
	return hold_button(SENSE_KEY,buttons,timeout_msec);
}

/////////////////////////////////////////////////////////////////////////////
// select ITEM
/////////////////////////////////////////////////////////////////////////////
int selitem(int sx,int sy,const char *title,const char **list,int def)
{
	int max_sel;
	int sel;
	int i , key;
	int delta;
	int sx_; // left align
	char lst[CONSOLE_WIDTH+1];

//Kprintf("selitem(%d,%d,%s*,%d)\n",sx,sy,title,def);

	if(title)
	{
		// with title
		text_printfXY(sx,sy,title);
		sy++;
	}

	// auto centerling
	if(sx==TEXT_CENTER)
	{
		sx = 999;
		for(max_sel=0;list[max_sel];max_sel++)
		{
			sx_ = get_center_sx(list[max_sel]);
			if(sx > sx_) sx = sx_;
		}
	}
	if(max_sel==0) return -1;

  sel   = def;
  delta = 0;
  while(1)
  {
    for(i=0;i<max_sel;i++)
    {
      if(list[i][0] != 0x00)
      {
        // centering

        if(def<0)
        {
          text_set_fc(0);
          text_set_bc(0);
        }
        else
        {
          text_set_fc( (i==sel) ? COLOR_MENU_SEL_FG : COLOR_MENU_LIST_FG);
          text_set_bc( (i==sel) ? COLOR_MENU_SEL_BG : COLOR_MENU_LIST_BG);
        }
         strncpy(lst, list[i], CONSOLE_WIDTH);
         lst[CONSOLE_WIDTH] = '\0';
         text_printfXY(sx,sy+i,lst);
      }
    }

    // finish?
    if(def<0) break;

    while( (get_key() & (PSP_CTRL_UP|PSP_CTRL_DOWN|PSP_CTRL_CIRCLE|PSP_CTRL_TRIANGLE))!=0)
      sceKernelDelayThread(20000);

    def = sel;
    delta = 0;
    while( (def==sel) && (delta==0) )
    {
      key = get_key();
      if(key & PSP_CTRL_UP)       delta = -1;
      if(key & PSP_CTRL_DOWN)     delta = +1;
      if(key & PSP_CTRL_CIRCLE)   def = -1;
      if(key & PSP_CTRL_TRIANGLE) def = -2;
      sceKernelDelayThread(20000);
    }
    // change next select
    do
    {
      sel += delta;
      if(sel>=max_sel) sel = 0;
      else if(sel<0)   sel = max_sel-1;
    }while(list[sel][0] == 0x00);
  }

  locate(sx,sy);
  text_set_fc(0xffffff);
  text_set_bc(0);
  if(def==-2) return -1;  // cancel
  return sel;
}

SceIoDirent g_dir;

char *add_file_list(const char *dir_path,char **file_list,char **full_list,char *buf,char *ext)
{
  int fd;
  char *ptr = 0;

  // search end of list
  while(*file_list)
  {
    file_list++;
    full_list++;
  }

	fd = sceIoDopen(dir_path);
//printf("dopen fd %08X\n",fd);

	if(fd >= 0)
	{
		while(sceIoDread(fd, &g_dir) > 0)
		{
 			if((g_dir.d_stat.st_attr & FIO_SO_IFDIR) == 0)
			{
				// cmp extend 
				ptr = strchr(g_dir.d_name,'.');
				if(
					ext == NULL || (
					 ptr!=NULL &&
					(ptr[1]|0x20)==(ext[0]|0x20)&& 
					(ptr[2]|0x20)==(ext[1]|0x20)&& 
					(ptr[3]|0x20)==(ext[2]|0x20) )
				){
					// add list

					// full path
					sprintf(buf,"%s/%s",dir_path,g_dir.d_name);
					*full_list++ = buf;
					buf += strlen(buf)+1;
					// name
					if(file_list)
					{
						strcpy(buf,g_dir.d_name);
						*file_list++ = buf;
						buf += strlen(buf)+1;
					}
				}
			}
		}
		sceIoDclose(fd);
	}
	// EOF
	*file_list = 0;
	*full_list = 0;

	return buf;
}
