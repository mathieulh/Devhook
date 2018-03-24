//#include "common.h"

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspsdk.h>
#include <pspctrl.h>
#include <stdio.h>
#include <string.h>

#include "psp_uart.h"

void psp_uart_putc(int txd)
{
  struct psp_uart_hw *uart = (struct psp_uart_hw *)0xbe500000;

  asm(" .word 0x7c042420" ::);
 
  while( uart->sts & psp_uart_sts_TI);
  uart->txd = txd;
}

void uart_dbg_putc(int arg1,int code)
{
  struct psp_uart_hw *uart = (struct psp_uart_hw *)0xbe500000;

  //if(uart_dbg_init_flag==0)
  if(code==0x200)
  {
    int brg = 96000000 / 115200;
    //enable
    *(unsigned int *)0xbc100078 |= (0x00010000 << 4);

    uart->r50 |= 0x4000;
    uart->r58 |= 0x2000;
    uart->r78 |= 0x0008;
    uart->brgh = brg>>6;
    uart->brgl = brg&0x3f;
    uart->r2c  = 0x70;
    uart->r30  = 0x0301;
    uart->r34  = 0x00;
    uart->r38  = 0x10;
    uart->r3c  = 0x028d;
    uart->r44 |= 0x07ff;
//  uart->r1c  = 0x1ff;

#if 0
    while( uart->sts & psp_uart_sts_TI);
    uart->txd = '-';
#endif
    //uart_dbg_init_flag=1;
  } else if(code==0x201)
  {
    while( uart->sts & psp_uart_sts_TI);
  }

  if(code&0xffffff00) return;
//  if(code&0xffffff00) code = '.';

  if(code==0x0a)
  {
    while( uart->sts & psp_uart_sts_TI);
    uart->txd = 0x0d;
  }
  while( uart->sts & psp_uart_sts_TI);
  uart->txd = code;
}

void psp_uart_puts(unsigned char *data,int len)
{
  struct psp_uart_hw *uart = (struct psp_uart_hw *)0xbe500000;
  while(len--);
  {
    //asm(" .word 0x7c042420" ::);
    while( uart->sts & psp_uart_sts_TI);
    uart->txd = data++;
  }
}
