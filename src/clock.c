/*
	DEVHOOK clock controll
 */
#include "common.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static u32 clk_reg[2] = {0,0};

/////////////////////////////////////////////////////////////////////////////
// clock checker
/////////////////////////////////////////////////////////////////////////////
/*
vsh

0000DC9D

0000DC1D
0000DC3F (or   22)
0000DC3D (and ~02)
0000DC1D (and ~02)

game

0000DC1D
0000DC1E ( or  02)
0000DC3DD1C

*/
void clock_checker(int tick_usec)
{
	//1sec
	int cpu_freq , bus_freq;

	// clock rate
	cpu_freq = clk_reg[0];
	bus_freq = clk_reg[1];

	if(cpu_freq != 0) // 0ÇÃéûÇÕêßå‰ÇµÇ»Ç¢
	{
		int cur_cpu , cur_bus , clk_diff;

		cur_cpu = scePowerGetCpuClockFrequency();
		cur_bus = scePowerGetBusClockFrequency();
		clk_diff = (cpu_freq  - cur_cpu) + (bus_freq-cur_bus);

		if( (clk_diff < -2) ||  (clk_diff > 2))
		{
//if( ((*(u32 *)0xbc100050) & 0x20) == 0) return;

//sceSysregMeBusClockDisable();
			scePowerSetClockFrequency(cpu_freq,cpu_freq,bus_freq);
//sceSysregMeBusClockEnable();

//			scePowerSetBusClockFrequency(bus_freq);
//			scePowerSetCpuClockFrequency(cpu_freq);

			//dh_config.cpu_Mhz = scePowerGetCpuClockFrequency();
			//dh_config.bus_Mhz = scePowerGetBusClockFrequency();
			Kprintf("CLOCK %d/%d to %d/%d\n",cur_cpu,cur_bus,scePowerGetCpuClockFrequency(),scePowerGetBusClockFrequency());
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// set clock
/////////////////////////////////////////////////////////////////////////////
int dhCLKSet(int cpuclk,int busclk)
{
	clk_reg[0] = cpuclk;
	clk_reg[1] = busclk;
	dhSetRegistry("CLOCK",clk_reg,8);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// clock initialize
/////////////////////////////////////////////////////////////////////////////
void clock_init(void)
{
	// get clock value
	dhGetRegistry("CLOCK",clk_reg,8);
}
