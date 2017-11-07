#include "../../inc/basic.h"

//#include "rc5.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_START

#define RC5_ENCRYPT	1
#define RC5_DECRYPT	0

/* 32 bit.  For Alpha, things may get weird */
#define RC5_32_INT unsigned long

#define RC5_32_BLOCK		8
#define RC5_32_KEY_LENGTH	16 /* This is a default, max is 255 */

/* This are the only values supported.  Tweak the code if you want more
* The most supported modes will be
* RC5-32/12/16
* RC5-32/16/8
*/
#define RC5_8_ROUNDS	8
#define RC5_12_ROUNDS	12
#define RC5_16_ROUNDS	16

typedef struct rc5_key_st
{
	/* Number of rounds */
	int rounds;
	RC5_32_INT data[2*(RC5_16_ROUNDS+1)];
} RC5_32_KEY;

void RC5_32_set_key(RC5_32_KEY *key, int len, const unsigned char *data, int rounds);
void RC5_32_ecb_encrypt(const unsigned char *in,unsigned char *out,RC5_32_KEY *key, int enc);

void RC5_32_encrypt(unsigned long *data,RC5_32_KEY *key);
void RC5_32_decrypt(unsigned long *data,RC5_32_KEY *key);

void RC5_32_cbc_encrypt(const unsigned char *in, unsigned char *out, long length, RC5_32_KEY *ks, unsigned char *iv, int enc);

void RC5_32_cfb64_encrypt(const unsigned char *in, unsigned char *out, long length, RC5_32_KEY *schedule, unsigned char *ivec, int *num, int enc);
void RC5_32_ofb64_encrypt(const unsigned char *in, unsigned char *out, long length, RC5_32_KEY *schedule, unsigned char *ivec, int *num);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "rc5_locl.h"

void RC5_32_set_key(RC5_32_KEY *key, int len, const unsigned char *data,int rounds)
{
	RC5_32_INT L[64],l,ll,A,B,*S,k;
	int i,j,m,c,t,ii,jj;

	if (	(rounds != RC5_16_ROUNDS) &&
		(rounds != RC5_12_ROUNDS) &&
		(rounds != RC5_8_ROUNDS))
		rounds=RC5_16_ROUNDS;

	key->rounds=rounds;
	S= &(key->data[0]);
	j=0;
	for (i=0; i<=(len-8); i+=8)
	{
		c2l(data,l);
		L[j++]=l;
		c2l(data,l);
		L[j++]=l;
	}
	ii=len-i;
	if (ii)
	{
		k=len&0x07;
		c2ln(data,l,ll,k);
		L[j+0]=l;
		L[j+1]=ll;
	}

	c=(len+3)/4;
	t=(rounds+1)*2;
	S[0]=RC5_32_P;
	for (i=1; i<t; i++)
		S[i]=(S[i-1]+RC5_32_Q)&RC5_32_MASK;

	j=(t>c)?t:c;
	j*=3;
	ii=jj=0;
	A=B=0;
	for (i=0; i<j; i++)
	{
		k=(S[ii]+A+B)&RC5_32_MASK;
		A=S[ii]=ROTATE_l32(k,3);
		m=(int)(A+B);
		k=(L[jj]+A+B)&RC5_32_MASK;
		B=L[jj]=ROTATE_l32(k,m);
		if (++ii >= t) ii=0;
		if (++jj >= c) jj=0;
	}
}

void RC5_32_ecb_encrypt(const unsigned char *in, unsigned char *out, RC5_32_KEY *ks, int encrypt)
{
	unsigned long l,d[2];

	c2l(in,l); d[0]=l;
	c2l(in,l); d[1]=l;
	if (encrypt)
		RC5_32_encrypt(d,ks);
	else
		RC5_32_decrypt(d,ks);
	l=d[0]; l2c(l,out);
	l=d[1]; l2c(l,out);
	l=d[0]=d[1]=0;
}

void RC5_32_cbc_encrypt(const unsigned char *in, unsigned char *out, long length, RC5_32_KEY *ks, unsigned char *iv, int encrypt)
{
	register unsigned long tin0,tin1;
	register unsigned long tout0,tout1,xor0,xor1;
	register long l=length;
	unsigned long tin[2];

	if (encrypt)
	{
		c2l(iv,tout0);
		c2l(iv,tout1);
		iv-=8;
		for (l-=8; l>=0; l-=8)
		{
			c2l(in,tin0);
			c2l(in,tin1);
			tin0^=tout0;
			tin1^=tout1;
			tin[0]=tin0;
			tin[1]=tin1;
			RC5_32_encrypt(tin,ks);
			tout0=tin[0]; l2c(tout0,out);
			tout1=tin[1]; l2c(tout1,out);
		}
		if (l != -8)
		{
			c2ln(in,tin0,tin1,l+8);
			tin0^=tout0;
			tin1^=tout1;
			tin[0]=tin0;
			tin[1]=tin1;
			RC5_32_encrypt(tin,ks);
			tout0=tin[0]; l2c(tout0,out);
			tout1=tin[1]; l2c(tout1,out);
		}
		l2c(tout0,iv);
		l2c(tout1,iv);
	}
	else
	{
		c2l(iv,xor0);
		c2l(iv,xor1);
		iv-=8;
		for (l-=8; l>=0; l-=8)
		{
			c2l(in,tin0); tin[0]=tin0;
			c2l(in,tin1); tin[1]=tin1;
			RC5_32_decrypt(tin,ks);
			tout0=tin[0]^xor0;
			tout1=tin[1]^xor1;
			l2c(tout0,out);
			l2c(tout1,out);
			xor0=tin0;
			xor1=tin1;
		}
		if (l != -8)
		{
			c2l(in,tin0); tin[0]=tin0;
			c2l(in,tin1); tin[1]=tin1;
			RC5_32_decrypt(tin,ks);
			tout0=tin[0]^xor0;
			tout1=tin[1]^xor1;
			l2cn(tout0,tout1,out,l+8);
			xor0=tin0;
			xor1=tin1;
		}
		l2c(xor0,iv);
		l2c(xor1,iv);
	}
	tin0=tin1=tout0=tout1=xor0=xor1=0;
	tin[0]=tin[1]=0;
}

void RC5_32_encrypt(unsigned long *d, RC5_32_KEY *key)
{
	RC5_32_INT a,b,*s;

	s=key->data;

	a=d[0]+s[0];
	b=d[1]+s[1];
	E_RC5_32(a,b,s, 2);
	E_RC5_32(a,b,s, 4);
	E_RC5_32(a,b,s, 6);
	E_RC5_32(a,b,s, 8);
	E_RC5_32(a,b,s,10);
	E_RC5_32(a,b,s,12);
	E_RC5_32(a,b,s,14);
	E_RC5_32(a,b,s,16);
	if (key->rounds == 12)
	{
		E_RC5_32(a,b,s,18);
		E_RC5_32(a,b,s,20);
		E_RC5_32(a,b,s,22);
		E_RC5_32(a,b,s,24);
	}
	else if (key->rounds == 16)
	{
		/* Do a full expansion to avoid a jump */
		E_RC5_32(a,b,s,18);
		E_RC5_32(a,b,s,20);
		E_RC5_32(a,b,s,22);
		E_RC5_32(a,b,s,24);
		E_RC5_32(a,b,s,26);
		E_RC5_32(a,b,s,28);
		E_RC5_32(a,b,s,30);
		E_RC5_32(a,b,s,32);
	}
	d[0]=a;
	d[1]=b;
}

void RC5_32_decrypt(unsigned long *d, RC5_32_KEY *key)
{
	RC5_32_INT a,b,*s;

	s=key->data;

	a=d[0];
	b=d[1];
	if (key->rounds == 16) 
	{
		D_RC5_32(a,b,s,32);
		D_RC5_32(a,b,s,30);
		D_RC5_32(a,b,s,28);
		D_RC5_32(a,b,s,26);
		/* Do a full expansion to avoid a jump */
		D_RC5_32(a,b,s,24);
		D_RC5_32(a,b,s,22);
		D_RC5_32(a,b,s,20);
		D_RC5_32(a,b,s,18);
	}
	else if (key->rounds == 12)
	{
		D_RC5_32(a,b,s,24);
		D_RC5_32(a,b,s,22);
		D_RC5_32(a,b,s,20);
		D_RC5_32(a,b,s,18);
	}
	D_RC5_32(a,b,s,16);
	D_RC5_32(a,b,s,14);
	D_RC5_32(a,b,s,12);
	D_RC5_32(a,b,s,10);
	D_RC5_32(a,b,s, 8);
	D_RC5_32(a,b,s, 6);
	D_RC5_32(a,b,s, 4);
	D_RC5_32(a,b,s, 2);
	d[0]=a-s[0];
	d[1]=b-s[1];
}

void RC5_32_ofb64_encrypt(const unsigned char *in, unsigned char *out, long length, RC5_32_KEY *schedule, unsigned char *ivec, int *num)
{
	register unsigned long v0,v1,t;
	register int n= *num;
	register long l=length;
	unsigned char d[8];
	register char *dp;
	unsigned long ti[2];
	unsigned char *iv;
	int save=0;

	iv=(unsigned char *)ivec;
	c2l(iv,v0);
	c2l(iv,v1);
	ti[0]=v0;
	ti[1]=v1;
	dp=(char *)d;
	l2c(v0,dp);
	l2c(v1,dp);
	while (l--)
	{
		if (n == 0)
		{
			RC5_32_encrypt((unsigned long *)ti,schedule);
			dp=(char *)d;
			t=ti[0]; l2c(t,dp);
			t=ti[1]; l2c(t,dp);
			save++;
		}
		*(out++)= *(in++)^d[n];
		n=(n+1)&0x07;
	}
	if (save)
	{
		v0=ti[0];
		v1=ti[1];
		iv=(unsigned char *)ivec;
		l2c(v0,iv);
		l2c(v1,iv);
	}
	t=v0=v1=ti[0]=ti[1]=0;
	*num=n;
}

void RC5_32_cfb64_encrypt(const unsigned char *in, unsigned char *out, long length, RC5_32_KEY *schedule, unsigned char *ivec, int *num, int encrypt)
{
	register unsigned long v0,v1,t;
	register int n= *num;
	register long l=length;
	unsigned long ti[2];
	unsigned char *iv,c,cc;

	iv=(unsigned char *)ivec;
	if (encrypt)
	{
		while (l--)
		{
			if (n == 0)
			{
				c2l(iv,v0); ti[0]=v0;
				c2l(iv,v1); ti[1]=v1;
				RC5_32_encrypt((unsigned long *)ti,schedule);
				iv=(unsigned char *)ivec;
				t=ti[0]; l2c(t,iv);
				t=ti[1]; l2c(t,iv);
				iv=(unsigned char *)ivec;
			}
			c= *(in++)^iv[n];
			*(out++)=c;
			iv[n]=c;
			n=(n+1)&0x07;
		}
	}
	else
	{
		while (l--)
		{
			if (n == 0)
			{
				c2l(iv,v0); ti[0]=v0;
				c2l(iv,v1); ti[1]=v1;
				RC5_32_encrypt((unsigned long *)ti,schedule);
				iv=(unsigned char *)ivec;
				t=ti[0]; l2c(t,iv);
				t=ti[1]; l2c(t,iv);
				iv=(unsigned char *)ivec;
			}
			cc= *(in++);
			c=iv[n];
			iv[n]=cc;
			*(out++)=c^cc;
			n=(n+1)&0x07;
		}
	}
	v0=v1=ti[0]=ti[1]=t=c=cc=0;
	*num=n;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//输出的函数
static CCriticalSection g_mtLock;
int Basic_RC5_ecb_encrypt(basiclib::IBasicSecurity* pSecurity, char *pOutData, char *pInData, long lDatalen, const char *pKey, int iKeylen, int iEncrypt, int nRounds)
{
	if( !( pOutData && pInData && pKey && (lDatalen=(lDatalen+7)&0xfffffff8) ))
	{
		return -1;
	}
	
	CSingleLock lock(&g_mtLock);
	lock.Lock();

	RC5_32_KEY key; 
	RC5_32_set_key(&key, iKeylen, (unsigned char*)pKey, nRounds);
	
	for(long i = 0, j = lDatalen >> 3; i < j; ++i, pOutData+=8, pInData+=8)
	{
		RC5_32_ecb_encrypt((unsigned char *)pInData, (unsigned char *)pOutData, &key, iEncrypt);
		if( pSecurity )
			pSecurity->Check();
	}
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_END
