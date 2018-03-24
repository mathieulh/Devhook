/*
	intel hex dumper
*/
/****************************************************************************
	IPL decrypt catcher
****************************************************************************/

extern void (*Kprintf)(const char *format,...);

static const char hex[]="0123456789ABCDEF";

static char sbuf[128];
static unsigned char buf[128];

static void put_hex(unsigned char *buf,int size,int addr,int type)
{
	int i,len,val;
	int sum = 0;
	len = 0;
	sbuf[len++] = ':';
	sum += size;
	sbuf[len++] = hex[size/16];
	sbuf[len++] = hex[size&0x0f];

	val = (addr >> 8)&0xff;
	sum += val;
	sbuf[len++] = hex[val/16];
	sbuf[len++] = hex[val&0x0f];

	val = addr&0xff;
	sum += val;
	sbuf[len++] = hex[val/16];
	sbuf[len++] = hex[val&0x0f];

	sum += type;
	sbuf[len++] = hex[type/16];
	sbuf[len++] = hex[type&0x0f];

	for(i=0;i<size;i++)
	{
		val = buf[i];
		sum += val;
		sbuf[len++] = hex[val/16];
		sbuf[len++] = hex[val&0x0f];
	}
	sbuf[len++] = hex[(sum>>4)&0x0f];
	sbuf[len++] = hex[sum&0x0f];
	sbuf[len++] = 0x0d;
	sbuf[len++] = 0x0a;
	sbuf[len++] = 0x00;
	Kprintf(sbuf);
}

void dump_ihex(void *top,int size)
{
	int i,j;
	unsigned char *p = (unsigned char *)top;

	Kprintf("DUMP HEX %08X-%08X\n",(int)p,(int)p + size -1);

//	u32 val;
	int len;
	for(i=0;i<size;i+=32)
	{
		asm(" .word 0x7c042420" ::);

		if( (i&0xffff)==0)
		{
			buf[0] = i>>4;
			buf[1] = (i>>12)&0xff;
			put_hex(buf,2,0,2);
		}

		len = 0;
		for(j=0;j<32;j+=4)
		{
			unsigned int val = *(unsigned int *)(p+i+j);
			buf[len++] = val&0xff;
			buf[len++] = (val>>8)&0xff;
			buf[len++] = (val>>16)&0xff;
			buf[len++] = (val>>24)&0xff;
		}
		put_hex(buf,len,i&0xffff,0);
	}
}
