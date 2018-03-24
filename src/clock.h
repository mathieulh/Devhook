#ifndef __CLOCK_H__
#define __CLOCK_H__

void clock_checker(int tick_usec);
void clock_init(void);

// API
int dhCLKSet(int cpuclk,int busclk);

#endif
