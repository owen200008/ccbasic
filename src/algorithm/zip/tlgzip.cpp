#include "../../inc/basic.h"

__NS_BASIC_START

static voidpf g_alloc(voidpf opaque, uInt items, uInt size)
{
	uInt toalloc = items * size;

	return BasicAllocate(toalloc);
}

static void g_free(voidpf opaque, voidpf address)
{
	BasicDeallocate(address);
	address = opaque;
}

int basic_gcompress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen, int level)
{
	z_stream stream;
	int err;

	stream.next_in = (Bytef*)source;
	stream.avail_in = (uInt)sourceLen;

	time_t now;
	time(&now);
#ifdef __MSVC
	_snprintf((char*)dest, *destLen, "%c%c%c%c%d%c%c", 0x1f, 0x8b,
		Z_DEFLATED, 0 /*flags*/, (uint32_t)now /*time*/, 0 /*xflags*/, 0x0b);
#else
	sprintf((char*)dest, "%c%c%c%c%d%c%c", 0x1f, 0x8b,
		Z_DEFLATED, 0 /*flags*/, (uint32_t)now /*time*/, 0 /*xflags*/, 0x0b);
#endif
	stream.next_out = dest + 10;
	stream.avail_out = (uInt)*destLen;
	if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

	stream.zalloc = (alloc_func)g_alloc;
	stream.zfree = (free_func)g_free;
	stream.opaque = (voidpf)0;

	err = deflateInit2(&stream, level,
		Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
	if (err != Z_OK) return err;

	err = deflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END) {
		deflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}
	*destLen = stream.total_out + 10 + sizeof(uLong) * 2;	// compress length + gz head + crc + uncompress len	
	uLong *ptail = (uLong*)(dest + 10 + stream.total_out);
	*ptail ++  = crc32(0, source, sourceLen);
	*ptail = sourceLen;

	err = deflateEnd(&stream);
	return err;
}

#define FTEXT		0x80
#define FHCRC		0x40
#define FEXTRA		0x20
#define FNAME		0x10
#define FCOMMENT	0x08

struct FEXTRAHEAD
{
	byte SI1;
	byte SI2;
	unsigned short XLEN;
};

int basic_guncompress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen)
{
	if (NULL == dest || *destLen == 0)
	{
		uLong* ptail = (uLong*)(source + sourceLen);
		*destLen = *(--ptail);
		return Z_OK;
	}

	z_stream stream;
	int err;

	stream.next_in = (Bytef*)(source + 10);

	BYTE bFlag = source[3];
	if (bFlag & FEXTRA)
	{
		FEXTRAHEAD* pHead = (FEXTRAHEAD*)stream.next_in;
		stream.next_in += sizeof(FEXTRAHEAD) + pHead->XLEN;
	}
	else if ((bFlag & FCOMMENT) || (bFlag & FTEXT) || (bFlag & FNAME))
	{
		stream.next_in += strlen((char*)stream.next_in) + 1;
	}

	stream.avail_in = (uInt)sourceLen - 10 - sizeof(uLong) * 2;

	stream.next_out = dest;
	stream.avail_out = (uInt)*destLen;

	stream.zalloc = (alloc_func)g_alloc;
	stream.zfree = (free_func)g_free;
	stream.opaque = (voidpf)0;

	err = inflateInit2(&stream, -MAX_WBITS);
	if (err != Z_OK) return err;

	err = inflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END)
	{
		deflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}

	*destLen = stream.total_out;
	err = inflateEnd(&stream);
	return err;
}

__NS_BASIC_END

