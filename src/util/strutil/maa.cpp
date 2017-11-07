#include "../../inc/basic.h"
#include "maa.h"

#define BIT31 0x80000000L 
#define FFFF  0xFFFFL 
#define _A	0x2040801L
#define _B	0x804021L
#define _C	0xBFEF7FDFL
#define _D	0x7DFEFBFFL

__NS_BASIC_START

static void mul32(unsigned long a, unsigned long b, unsigned long& u, unsigned long& l)
{
	unsigned long p1, p2, p3, sum;
	p1 = (a & FFFF) * (b & FFFF);
	p2 = (a & FFFF) * (b >> 16);
	p3 = (a >> 16) * (b & FFFF);
	sum = (p1 >> 16) + (p2 & FFFF) + (p3 & FFFF);
	l = (p1 & FFFF) + (sum << 16);
	u = (sum >> 16) + (p2 >> 16) + (p3 >> 16) + ((a >> 16) * (b >> 16));
}

static unsigned long mul1(unsigned long a, unsigned long b)
{
	unsigned long s, car, l, u;
	mul32(a, b, u, l);
	s = u + l;
	car = ((u ^ l) & BIT31) ? !(s & BIT31) : (u & BIT31) != 0;
	return (s + car);
}

static unsigned long mul2(unsigned long a, unsigned long b)
{
	unsigned long d, f, s, car, u, l;
	mul32(a, b, u, l);
	d = u + u; 
	car = (u & BIT31) != 0;
	f = d + car + car;
	s = f + l;
	car = ((f ^ l) & BIT31) ? !(s & BIT31) : (f & BIT31) != 0;
	return (s + car + car);
}

static unsigned long mul2a(unsigned long a, unsigned long b)
{
	unsigned long d, s, car, u, l;
	mul32(a, b, u, l);
	d = u + u;
	s = d + l;
	car = ((d ^ l) & BIT31) ? !(s & BIT31) : (d & BIT31) != 0;
	return (s + car + car);
}

static void byt(unsigned long x, unsigned long y, unsigned long& u, unsigned long& l, unsigned long& pat)
{
	unsigned long p;
	int b[8];
	int i = 0;
	for(i = 3; i >= 0; i--)
	{
		b[i] = x & 255; 
		b[i+4] = y & 255;
		x = x >> 8; 
		y = y >> 8;
	}
	p = 0; 
	for(i = 0; i < 8; i++) 
	{
		p = p + p;
		if(b[i] == 0)
		{
			p += 1;
			b[i] = p;
		} 
		else if(b[i] == 255)
		{
			p += 1;
			b[i] = 255 - p;
		}
	}
	pat = p;
	x = 0;
	y = 0;
	for(i = 0; i < 4; i++)
	{
		x = (x << 8) + b[i];
		y = (y << 8) + b[i+4];
	}
	u = x;
	l = y;
}

static void prelude(unsigned long x, unsigned long y, unsigned long* u, unsigned long* l)
{
	unsigned long j1[10], j2[10], k1[10], k2[10], h[10];
	unsigned long p, q;
	
	byt(x, y, j1[0], k1[0], p);
	
	q = (p + 1) * (p + 1);
	j1[2] = mul1(j1[0], j1[0]);
	j2[2] = mul2(j1[0], j1[0]);
	j1[4] = mul1(j1[2], j1[2]);
	j2[4] = mul2(j2[2], j2[2]);
	j1[6] = mul1(j1[2], j1[4]);
	j2[6] = mul2(j2[2], j2[4]);
	j1[8] = mul1(j1[2], j1[6]);
	j2[8] = mul2(j2[2], j2[6]);
	h[4]  = j1[4] ^ j2[4]; 
	h[6]  = j1[6] ^ j2[6];
	h[8]  = j1[8] ^ j2[8];
	
	k1[2] = mul1(k1[0], k1[0]);
	k2[2] = mul2(k1[0], k1[0]);
	k1[4] = mul1(k1[2], k1[2]);
	k2[4] = mul2(k2[2], k2[2]);
	k1[5] = mul1(k1[0], k1[4]);
	k2[5] = mul2(k1[0], k2[4]);
	k1[7] = mul1(k1[2], k1[5]);
	k2[7] = mul2(k2[2], k2[5]);
	k1[9] = mul1(k1[2], k1[7]);
	k2[9] = mul2(k2[2], k2[7]);
	
	h[0]  = k1[5] ^ k2[5];
	h[5]  = mul2(h[0], q);
	h[7]  = k1[7] ^ k2[7];
	h[9]  = k1[9] ^ k2[9];
	
	byt(h[4], h[5], u[0], l[0], p);		//x0 y0
	byt(h[6], h[7], u[1], l[1], p);		//v0 w
	byt(h[8], h[9], u[2], l[2], p);		//s=u; t=l;
	
}

static void mainloop(unsigned long m, unsigned long* u, unsigned long* l)
{
	unsigned long e, f, g, s;
	int car;
	u[1] = (u[1] & BIT31) ? (u[1] << 1) | 1L : u[1] << 1;     /* V=CYC(V); */
	e = u[1] ^ l[1];
	u[0] = u[0] ^ m;
	l[0] = l[0] ^ m; 
	f = e + l[0];
	g = e + u[0]; 
	f = f | _A;
	g = g | _B;
	f = f & _C;
	g = g & _D;

	unsigned long uu, ll;
	mul32(u[0], f, uu, ll);
	s = uu + ll;
	car = ((uu ^ ll) & BIT31) ? !(s & BIT31) : (uu & BIT31) != 0;
	u[0] = (car) ? s + 1L : s;

										/* y=mul2a(y,g); */
	mul32(l[0], g, uu, ll);
	g = uu + uu;
	s = g + ll;
	car = ((g ^ ll) & BIT31) ? !(s & BIT31) : (g & BIT31) != 0;
	l[0] = (car) ? s + 2L : s;
}

unsigned long Basic_MAA(unsigned long x, unsigned long y, unsigned long* m, long n)
{
	unsigned long u[3], l[3];
	prelude(x, y, u, l);
	for(long i = 0; i < n; i++)
	{
		mainloop(m[i], u, l);
	}
	mainloop(u[2], u, l);
	mainloop(l[2], u, l);
	return u[0] ^ l[0];
}

#define _X  0x12345678L
#define _Y  0x87654321L
unsigned long Basic_GetMessageAuth(void* pBuffer, long lLength)
{
	if(pBuffer == NULL || lLength <= 0)
	{
		return 0;
	}
	long lCount = lLength / sizeof(long)  + 1;
	unsigned long* pLong = new unsigned long[lCount];
	memset(pLong, 0, sizeof(long) * lCount);
	memcpy(pLong, pBuffer, lLength);
	unsigned long lRet = Basic_MAA(_X, _Y, pLong, lCount);
	delete[] pLong;
	return lRet;
}

__NS_BASIC_END
