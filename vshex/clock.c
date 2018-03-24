/*
	PSP VSH extender for devhook 0.50+
*/
#include "common.h"

/////////////////////////////////////////////////////////////////////////////
// change CPU clock
/////////////////////////////////////////////////////////////////////////////
#define MAX_CLK_LIST (sizeof(clk_list)/4)
static const int clk_list[] = 
{
	(  0<<16) +   0,
	(266<<16) + 133,
	(333<<16) + 166,
	(133<<16) +  66,
	(222<<16) + 111
};

void change_clock(int dir)
{
	u32 clock[2];
	int sel;

//Kprintf("change clock:");
	regGetClock(&(clock[0]),&(clock[1]));
//Kprintf("%d/%d:",clock[0],clock[1]);

	for(sel=0;sel<MAX_CLK_LIST;sel++)
		if( clk_list[sel]>>16 == clock[0]) break;

	// select new
	sel = limit(sel+dir,0,MAX_CLK_LIST-1);

	clock[0] = clk_list[sel]>>16;
	clock[1] = clk_list[sel]&0xffff;

//Kprintf("%d/%d:",clock[0],clock[1]);

	// change clock
	dhCLKSet(clock[0],clock[1]);
	// for next reboot
	regSetClock(clock[0],clock[1]);
}
