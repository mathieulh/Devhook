/*
	vfprintf for Kprintf
*/
#include "common.h"
//#include <stdarg.h>

#define DIFF_INT_LONG 0 /* support 'l' parameter ? */

#define FLAG_PLUS 0x80
#define FLAG_ZERO 0X40
#define FLAG_UNSIGN 0X20

static int numtofile(FILE_ *fp,unsigned long number,int place,char flags,int deci,int chra)
{
  unsigned long num;
  unsigned long keta = 1;
  int stores = 0;

  /* '+','-' */
  if(!(flags&FLAG_UNSIGN))
  {
    if(((long)number)<0)
    {
      fp->put(fp,'-');
      stores++;
      number = -number;
    }else if(flags&FLAG_PLUS)
    {
      fp->put(fp,number==0 ? ' ' : '+');
      stores++;
    }
  }

  /* max keta */
  while( (place>1) || (number >= (keta*deci)) )
  {
    // maximum check
    if(keta > keta*deci)
      break;
    keta *= deci;
    place--;
  }

  /* head '0' or ' ' */
  while( keta>1 )
  {
    if( (number/keta)%deci > 0) break;
    fp->put(fp,flags&FLAG_ZERO ? '0':' ');
    stores++;
    keta /= deci;
  }

  while( keta )
  {
    num = (number/keta)%deci;
    if( num < 10 )
      fp->put(fp,'0'+num);
    else
      fp->put(fp,chra+num-10);
    stores++;
    keta /= deci;
  }
  return stores;
}

/* format decomposer */
int vfprintf_(FILE_ *fp,const char *format,va_list arg)
{
  char flags;
  int len_n,len_e;
  char *sptr;
#if DIFF_INT_LONG
  char size;
#endif
  int stores = 0;
  long val_long;
  unsigned char base;

  while(*format)
  {
    if(*format!='%')
      fp->put(fp,*format++);
    else
    { /* % entry */
      format++;

      flags = 0;
      /* head '+' */
      if(*format == '+')
      {
        flags |= FLAG_PLUS;
        format++;
      } else if(*format == '-')
        format++;

      /* head '0' */
      if(*format == '0')
      {
        flags |= FLAG_ZERO;
        format++;
      }

      /* length */
      len_n = len_e = 0;
      while(*format>='0' && *format<='9')
        len_n = len_n*10 + (*format++)-'0';
      /* '.' */
      if(*format=='.')
      {
        format++;
        while(*format>='0' && *format<='9')
          len_e = len_e*10 + (*format++)-'0';
      }
#if DIFF_INT_LONG
      size = sizeof(int); /* int */
#endif
      base = 10;
again:
      /* type check */
      switch(*format|0x20)
      {
      case 'l': /* longlong / long */
#if DIFF_INT_LONG
        size = (*format&0x20) ? sizeof(long) : sizeof(longlong);
#endif
        format++;
        goto again;
      case 'u':
        flags |= FLAG_UNSIGN;
        format++;
        goto again;

      case 'c':
        fp->put(fp,(char)va_arg(arg,int));
        break;

      case 'x':
        flags |= FLAG_UNSIGN;
        base = 16;
      case 'd':
#if DIFF_INT_LONG
        switch(size)
        {
        case 1: val_long = va_arg(arg,long);
        case 2: val_long = va_arg(arg,longlong);
        default:
          if(flags&FLAG_UNSIGN)
            val_long = (unsigned int)va_arg(arg,unsigned int);
          else
            val_long = va_arg(arg,int);
        }
#else
        if(flags&FLAG_UNSIGN)
          val_long = (unsigned int)va_arg(arg,int);
        else
          val_long = va_arg(arg,int);
#endif
        stores += numtofile(fp,val_long,len_n,flags,base,*format-'X'+'A');
        break;
      case 's':
        sptr = va_arg(arg,char *);
        while(*sptr)
        {
          fp->put(fp,*sptr++);
          stores++;
        }
        break;
      default:
        break;
      }
      if(*format) format++;
    }
  }
  return stores;
}

int fprintf_(FILE_ *fp,const char *format,...)
{
  int ret;
  va_list arg;

  va_start(arg,format);
  ret = vfprintf_(fp,format,arg);
  va_end(arg);
  return ret;
}
