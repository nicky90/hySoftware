/*
 * gfchecksum.c
 */

#include "gfchecksum.h"
#include "md5.h"

#include <stdio.h>


int gf_check_data(char *chksm, char *data, int len)
{
	Md5Buffer((unsigned char *)chksm, (unsigned char const *)data, len);

	return 0;
}


int gf_checkcmp_data(char *chksm, char *data, int len)
{
	int ret=0;
	unsigned char realsm[GF_CHECK_LEN];

	Md5Buffer((unsigned char *)realsm, (unsigned char const *)data, len);
	ret=Md5Cmp((unsigned char *)realsm, (unsigned char *)chksm);

	return ret;
}

void gf_md5_16to32(uint8_t *str16, char *str32)
{
	int i;
	for(i=0; i<16; i++)
	{
		//printf("%d\t",str16[i]);
		sprintf(str32+2*i, "%02X", str16[i]);
	}
}

void gf_md5_32to16(char *str32, uint8_t *str16)
{
	int i, a, b;
	for(i=0; i<16; i++)
	{
		if((str32[2*i] >= 'A') && (str32[2*i] <= 'F'))
			a = str32[2*i] - 'A' + 10;
		else if((str32[2*i]>='a') && (str32[2*i]<='f'))
			a = str32[2*i] - 'a' + 10;
		else a = str32[2*i] - '0';

		if((str32[2*i+1] >= 'A') && (str32[2*i+1] <= 'F'))
			b = str32[2*i+1] - 'A' + 10;
		else if((str32[2*i+1]>='a') && (str32[2*i+1]<='f'))
			b = str32[2*i+1] - 'a' + 10;
		else b = str32[2*i+1] - '0';

		str16[i] = a * 16 + b;
	}

}
