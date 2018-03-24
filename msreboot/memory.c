/*
*/

#include "common.h"

void memcpy(u8 *dst,u8 *src,int size)
{
	while(size--) *dst++ = *src++;
}

