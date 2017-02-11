#include "../inc/basic_def.h"
#include  "xtra.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//对文件内容进行解密
#define MX (z>>5^y<<2) + (y>>3^z<<4) ^ (sum^y) + (k[p&3^e]^z);
int32_t Baisc_XTRA(int32_t* v, int32_t n, int32_t* k)
{
	uint32_t sum = 0, e, DELTA = 0x9e3779b9;
	int32_t p, q;
	if (n > 1)
	{          /* Coding Part */
		q = 6 + 52 / n;
		uint32_t z = v[n - 1];
		uint32_t y = v[0];
		while (q-- > 0) {
			sum += DELTA;
			e = (sum >> 2) & 3;
			for (p = 0; p<n - 1; p++) y = v[p + 1], z = v[p] += MX;
			y = v[0];
			z = v[n - 1] += MX;
		}
		return 0;
	}
	else if (n < -1)
	{  /* Decoding Part */
		n = -n;
		q = 6 + 52 / n;
		sum = q*DELTA;
		uint32_t z = v[n - 1];
		uint32_t y = v[0];
		while (sum != 0) {
			e = (sum >> 2) & 3;
			for (p = n - 1; p>0; p--) z = v[p - 1], y = v[p] -= MX;
			z = v[n - 1];
			y = v[0] -= MX;
			sum -= DELTA;
		}
		return 0;
	}
	return 1;
}