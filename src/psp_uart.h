
// uart headphone = 0xbe500000
// uart4 = 0xbe4c0000

/* bit 5 = TX EMPTY , bit4 = rxready , bit7 =0‚ÌŽžASleep•s‰Â ,bit3=1‚ÌŽžsleep•s‰Â */
#define psp_uart_sts_BSY  0x80
#define psp_uart_sts_TI   0x20
#define psp_uart_sts_RE   0x10
#define psp_uart_sts_BIT3 0x08

struct psp_uart_hw
{
  volatile unsigned int txd;  // +00 : txd
  volatile unsigned int r04;  // +04 
  volatile unsigned int rxd;  // +08 : rxd
  volatile unsigned int r0c;  // +0c ? 0x0000
  volatile unsigned int r10;  // +10 ? 0x0000
  volatile unsigned int r14;  // +14 ? 0x0000
  volatile unsigned int sts;  // +18 : status */
  volatile unsigned int r1c;  // +1c : unk , 0x1ff */
  volatile unsigned int r20;  // +20 ? 0x0000
  volatile unsigned int brgh; // +24 :(96000000 / bps) >> 6
  volatile unsigned int brgl; // +28 :(96000000 / bps) & 0x3f
  volatile unsigned int r2c;  // +2c : unknown , 0x70 = enable , 0x60 = stop?
  volatile unsigned int r30;  // +30 : unk , 0x0301
  volatile unsigned int r34;  // +34 : 0x00=0x00 , 0x08=0x09,0x16=0x12,0x24=0x12,0x27=0x24 */
  volatile unsigned int r38;  // +38 : unk , uart4=00 , uart=0x10
  volatile unsigned int r3c;  // +3c : unk , uart4=00 , uart=0x28d
  volatile unsigned int r40;  // +40 ? 0
  volatile unsigned int r44;  // +44 :  unk , 0x07ff / 0x0000
  volatile unsigned int r48;  // +48 ? 0
  volatile unsigned int r4c;  // +4c ? 0
  volatile unsigned int r50;  // +50 : unk , set bit14 init,resume = 0x4000
  volatile unsigned int r54;  // +54 ? 0
  volatile unsigned int r58;  // +58 : unk , set bit9  init,resume = 0x0020
  volatile unsigned int r5c;  // +5c ?
  volatile unsigned int r60;  // +60
  volatile unsigned int r64;  // +64
  volatile unsigned int r68;  // +68
  volatile unsigned int r6c;  // +6c
  volatile unsigned int r70;  // +70
  volatile unsigned int r74;  // +74
  volatile unsigned int r78;  // +78 : unk , set bit3 init resume , set bit19 sleep
};

int psp_uart_init(int bps);
void psp_uart_putc(int txd);
int psp_uart_getc(void);
void psp_uart_puts(unsigned char *data,int len);

void uart_dbg_putc(int arg1,int code);

