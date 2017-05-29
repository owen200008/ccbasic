#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "msvcint.h"

#include "sproto.h"
#include <basic.h>

#define SPROTO_TARRAY 0x80
#define CHUNK_SIZE 1000
#define SIZEOF_LENGTH 4
#define SIZEOF_HEADER 2
#define SIZEOF_FIELD 2

#define CC_SIZE_LENGTH	2

inline bool IsStarMiss(int bStar, int nSZ){
	return nSZ == 0 && bStar;
}




static void
pool_init(struct pool *p) {
	p->header = NULL;
	p->current = NULL;
	p->current_used = 0;
}

static void
pool_release(struct pool *p) {
	struct chunk * tmp = p->header;
	while (tmp) {
		struct chunk * n = tmp->next;
		basiclib::BasicDeallocate(tmp);
		tmp = n;
	}
}

static void *
pool_newchunk(struct pool *p, size_t sz) {
	struct chunk * t = (struct chunk *)basiclib::BasicAllocate(sz + sizeof(struct chunk));
	if (t == NULL)
		return NULL;
	t->next = p->header;
	p->header = t;
	return t+1;
}

static void *
pool_alloc(struct pool *p, size_t sz) {
	// align by 8
	sz = (sz + 7) & ~7;
	if (sz >= CHUNK_SIZE) {
		return pool_newchunk(p, sz);
	}
	if (p->current == NULL) {
		if (pool_newchunk(p, CHUNK_SIZE) == NULL)
			return NULL;
		p->current = p->header;
	}
	if (sz + p->current_used <= CHUNK_SIZE) {
		void * ret = (char *)(p->current+1) + p->current_used;
		p->current_used += sz;
		return ret;
	}

	if (sz >= p->current_used) {
		return pool_newchunk(p, sz);
	} else {
		void * ret = pool_newchunk(p, CHUNK_SIZE);
		p->current = p->header;
		p->current_used = sz;
		return ret;
	}
}

static inline int
toword(const uint8_t * p) {
	return p[0] | p[1]<<8;
}

static inline uint32_t
todword(const uint8_t *p) {
	return p[0] | p[1]<<8 | p[2]<<16 | p[3]<<24;
}

static inline uint64_t
chartokey(const uint8_t * p) {
	return p[0];
}
static inline uint64_t
shorttokey(const uint8_t * p) {
	return p[0] | p[1] << 8;
}
static inline uint64_t
inttokey(const uint8_t * p) {
	return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
}
static inline uint64_t
longlongtokey(const uint8_t * p) {
	uint64_t low = todword(p);
	uint64_t hi = todword(p + sizeof(uint32_t));
	return low | hi << 32;
}
static inline uint64_t
doubletokey(const uint8_t * p) {
	return longlongtokey(p);
}
typedef uint64_t(*decode_keymap)(const uint8_t * p);

static int
count_array(const uint8_t * stream) {
	uint32_t length = todword(stream);
	int n = 0;
	stream += SIZEOF_LENGTH;
	while (length > 0) {
		uint32_t nsz;
		if (length < SIZEOF_LENGTH)
			return -1;
		nsz = todword(stream);
		nsz += SIZEOF_LENGTH;
		if (nsz > length)
			return -1;
		++n;
		stream += nsz;
		length -= nsz;
	}

	return n;
}

static int
struct_field(const uint8_t * stream, size_t sz) {
	const uint8_t * field;
	int fn, header, i;
	if (sz < SIZEOF_LENGTH)
		return -1;
	fn = toword(stream);
	header = SIZEOF_HEADER + SIZEOF_FIELD * fn;
	if (sz < header)
		return -1;
	field = stream + SIZEOF_HEADER;
	sz -= header;
	stream += header;
	for (i=0;i<fn;i++) {
		int value= toword(field + i * SIZEOF_FIELD);
		uint32_t dsz;
		if (value != 0 || i == 6)
			continue;
		if (sz < SIZEOF_LENGTH)
			return -1;
		dsz = todword(stream);
		if (sz < SIZEOF_LENGTH + dsz)
			return -1;
		stream += SIZEOF_LENGTH + dsz;
		sz -= SIZEOF_LENGTH + dsz;
	}

	return fn;
}

static const char *
import_string(struct sproto *s, const uint8_t * stream) {
	uint32_t sz = todword(stream);
	char * buffer = (char*)pool_alloc(&s->memory, sz+1);
	memcpy(buffer, stream+SIZEOF_LENGTH, sz);
	buffer[sz] = '\0';
	return buffer;
}

static const uint8_t *
import_field(struct sproto *s, struct field *f, const uint8_t * stream) {
	uint32_t sz;
	const uint8_t * result;
	int fn;
	int i;
	int array = 0;
	//f->tag = -1;
	f->type = -1;
	f->name = NULL;
	f->st = NULL;
	f->m_nKeyType = -1;
	f->m_nDefaultValue = 0;

	sz = todword(stream);
	stream += SIZEOF_LENGTH;
	result = stream + sz;
	fn = struct_field(stream, sz);
	if (fn < 0)
		return NULL;
	stream += SIZEOF_HEADER;
	for (i=0;i<fn;i++) {
		int value = toword(stream + SIZEOF_FIELD * i);
		if (value & 1) {
			continue;
		}
		if (i == 0) { // name
			if (value != 0)
				return NULL;
			f->name = import_string(s, stream + fn * SIZEOF_FIELD);
			continue;
		}
		value = value/2 - 1;
		switch(i) {
		case 1: // buildin
		{
			if (!(value >= SPROTO_CC_CHAR && value <= SPROTO_CC_STRING) && !(value >= SPROTO_CC_EXA_BEGIN && value <= SPROTO_CC_EXA_END))
				return NULL;	// invalid buildin type
			f->type = value;
		}
			break;
		case 2: // type index
			if (value >= s->type_n)
				return NULL;	// invalid type index
			if (f->type >= 0)
				return NULL;
			f->type = SPROTO_CC_STRUCT;
			f->st = &s->type[value];
			break;
		case 3: // star
			f->m_bStar = value;
			break;
		case 4: // array
			if (value)
				array = SPROTO_TARRAY;
			break;
		case 5:	// key
		{
			if ((array & SPROTO_TARRAY) && value != 0)
			{
                if (!(value >= SPROTO_CC_CHAR && value <= SPROTO_CC_STRING))
					return NULL;	// invalid buildin type
				f->m_nKeyType = value;
			}
		}
			break;
		case 6:	//default value
		{
			f->m_nDefaultValue = value;
		}
			break;
		default:
			return NULL;
		}
	}
	if (f->type < 0 || f->name == NULL)
		return NULL;
	f->type |= array;

	return result;
}

/*
.type {
	.field {
		name 0 : string
		buildin 1 : integer
		type 2 : integer
		tag 3 : integer
		array 4 : boolean
	}
	name 0 : string
	fields 1 : *field
}
*/
static const uint8_t *
import_type(struct sproto *s, struct sproto_type *t, const uint8_t * stream) {
	const uint8_t * result;
	uint32_t sz = todword(stream);
	int i;
	int fn;
	int n;
	int last;
	stream += SIZEOF_LENGTH;
	result = stream + sz;
	fn = struct_field(stream, sz);
	if (fn <= 0 || fn > 2)
		return NULL;
	for (i=0;i<fn*SIZEOF_FIELD;i+=SIZEOF_FIELD) {
		// name and fields must encode to 0
		int v = toword(stream + SIZEOF_HEADER + i);
		if (v != 0)
			return NULL;
	}
	memset(t, 0, sizeof(*t));
	stream += SIZEOF_HEADER + fn * SIZEOF_FIELD;
	t->name = import_string(s, stream);
	if (fn == 1) {
		return result;
	}
	stream += todword(stream)+SIZEOF_LENGTH;	// second data
	n = count_array(stream);
	if (n<0)
		return NULL;
	stream += SIZEOF_LENGTH;
	last = -1;
	t->n = n;
	t->f = (struct field*)pool_alloc(&s->memory, sizeof(struct field) * n);
	for (i=0;i<n;i++) 
	{
		struct field *f = &t->f[i];
		stream = import_field(s, f, stream);
		if (stream == NULL)
			return NULL;
	}
	return result;
}

static struct sproto *
create_from_bundle(struct sproto *s, const uint8_t * stream, size_t sz) {
	const uint8_t * content;
	const uint8_t * typedata = NULL;
	int fn = struct_field(stream, sz);
	int i;
	if (fn != 1)
		return NULL;

	stream += SIZEOF_HEADER;
	content = stream + fn*SIZEOF_FIELD;

	int value = toword(stream);
	int n;
	if (value != 0)
		return NULL;
	n = count_array(content);
	if (n<0)
		return NULL;
	typedata = content + SIZEOF_LENGTH;
	s->type_n = n;
	s->type = (struct sproto_type *)pool_alloc(&s->memory, n * sizeof(*s->type));
	content += todword(content) + SIZEOF_LENGTH;

	for (i=0;i<s->type_n;i++) {
		typedata = import_type(s, &s->type[i], typedata);
		if (typedata == NULL) {
			return NULL;
		}
	}
	return s;
}

struct sproto *
sproto_create(const void * proto, size_t sz) {
	struct pool mem;
	struct sproto * s;
	pool_init(&mem);
	s = (struct sproto *)pool_alloc(&mem, sizeof(*s));
	if (s == NULL)
		return NULL;
	memset(s, 0, sizeof(*s));
	s->memory = mem;
	if (create_from_bundle(s, (const uint8_t *)proto, sz) == NULL) {
		pool_release(&s->memory);
		return NULL;
	}
	return s;
}

void
sproto_release(struct sproto * s) {
	if (s == NULL)
		return;
	pool_release(&s->memory);
}

const char* GetBuildInType(int nType)
{
	switch (nType)
	{
	case SPROTO_CC_UCHAR:
		return "Net_UChar";
	case SPROTO_CC_CHAR:
		return "Net_Char";
	case SPROTO_CC_SHORT:
		return "Net_Short";
	case SPROTO_CC_USHORT:
		return "Net_UShort";
	case SPROTO_CC_INT:
		return "Net_Int";
	case SPROTO_CC_UINT:
		return "Net_UInt";
	case SPROTO_CC_LONGLONG:
		return "Net_LongLong";
	case SPROTO_CC_DOUBLE:
		return "Net_Double";
	case SPROTO_CC_STRING:
		return "Net_CBasicString";
	case SPROTO_CC_EXA_CNETBASICVALUE:
		return "CNetBasicValue";
	}
	return NULL;
}
void
sproto_dump(struct sproto *s) 
{
	int i,j;
	printf("=== %d types ===\n", s->type_n);
	for (i=0;i<s->type_n;i++) 
	{
		struct sproto_type *t = &s->type[i];
		printf("%s\n", t->name);
		for (j=0;j<t->n;j++) 
		{
			char array[2] = { 0, 0 };
			const char * type_name = NULL;
			struct field *f = &t->f[j];
			if (f->type & SPROTO_TARRAY) 
			{
				array[0] = '*';
			}
			else 
			{
				array[0] = 0;
			}
			{
				int t = f->type & ~SPROTO_TARRAY;
				if (t == SPROTO_CC_STRUCT) 
				{
					type_name = f->st->name;
				} 
				else 
				{
					type_name = GetBuildInType(t);
				}
			}
			if (f->m_nKeyType > 0) 
			{
				const char * key_type_name = GetBuildInType(f->m_nKeyType);
				printf("\t%s () Map %s->%s\n", f->name, key_type_name, type_name);
			} 
			else 
			{
				printf("\t%s () %s%s\n", f->name, array, type_name);
			}
		}
	}
}

struct sproto_type* sproto_type(const struct sproto *sp, const char * type_name) {
	int i;
	for (i=0;i<sp->type_n;i++) {
		if (strcmp(type_name, sp->type[i].name) == 0) {
			return &sp->type[i];
		}
	}
	return NULL;
}

const char *
sproto_name(struct sproto_type * st) {
	return st->name;
}

// encode & decode
// sproto_callback(void *ud, int tag, int type, struct sproto_type *, void *value, int length)
//	  return size, -1 means error

static inline int
encodemapkey_ccchar(uint64_t v, uint8_t * data) {
	data[0] = v;
	return SPROTO_CC_CHAR_SIZE;
}

static inline int
encode_ccchar(unsigned char v, uint8_t * data, int size) {
	if (size < sizeof(v))
		return -1;
	data[0] = v;
	return SPROTO_CC_CHAR_SIZE;
}

static inline int
encodemapkey_ccshrot(uint64_t v, uint8_t * data) {
	data[0] = v & 0xff;
	data[1] = (v >> 8) & 0xff;
	return SPROTO_CC_SHORT_SIZE;
}

static inline int
encode_ccshort(unsigned short v, uint8_t * data, int size) {
	if (size < sizeof(v))
		return -1;
	data[0] = v & 0xff;
	data[1] = (v >> 8) & 0xff;
	return SPROTO_CC_SHORT_SIZE;
}

static inline int
encodemapkey_ccint(uint64_t v, uint8_t * data) {
	data[0] = v & 0xff;
	data[1] = (v >> 8) & 0xff;
	data[2] = (v >> 16) & 0xff;
	data[3] = (v >> 24) & 0xff;
	return SPROTO_CC_INT_SIZE;
}

static inline int
encode_ccinteger(uint32_t v, uint8_t * data, int size) {
	if (size < sizeof(v))
		return -1;
	data[0] = v & 0xff;
	data[1] = (v >> 8) & 0xff;
	data[2] = (v >> 16) & 0xff;
	data[3] = (v >> 24) & 0xff;
	return SPROTO_CC_INT_SIZE;
}

static inline int
encodemapkey_cclonglong(uint64_t v, uint8_t * data) {
	data[0] = v & 0xff;
	data[1] = (v >> 8) & 0xff;
	data[2] = (v >> 16) & 0xff;
	data[3] = (v >> 24) & 0xff;
	data[4] = (v >> 32) & 0xff;
	data[5] = (v >> 40) & 0xff;
	data[6] = (v >> 48) & 0xff;
	data[7] = (v >> 56) & 0xff;
	return SPROTO_CC_LONGLONG_SIZE;
}
static inline int
encode_cclonglong(uint64_t v, uint8_t * data, int size) {
	if (size < sizeof(v))
		return -1;
	data[0] = v & 0xff;
	data[1] = (v >> 8) & 0xff;
	data[2] = (v >> 16) & 0xff;
	data[3] = (v >> 24) & 0xff;
	data[4] = (v >> 32) & 0xff;
	data[5] = (v >> 40) & 0xff;
	data[6] = (v >> 48) & 0xff;
	data[7] = (v >> 56) & 0xff;
	return SPROTO_CC_LONGLONG_SIZE;
}

static inline int
encodemapkey_ccdouble(uint64_t v, uint8_t * data) {
	return encodemapkey_cclonglong(v, data);
}

static inline int
encode_ccdouble(double v, uint8_t * data, int size) {
	return encode_cclonglong(*(uint64_t*)&v, data, size);
}

typedef int(*encode_mapkey)(uint64_t v, uint8_t * data);

static int
encode_ccbasicvalue(sproto_callback cb, struct sproto_arg *args, uint8_t *data, int size)
{
	int sz;
	args->value = data;
	args->length = size;
	sz = cb(args);
	if (sz < 0)
	{
		if (sz == SPROTO_CB_NIL)
			return 0;
		return -1;	// sz == SPROTO_CB_ERROR
	}
	assert(sz <= size);	// verify buffer overflow
	return sz;
}

static int
encode_ccmap(sproto_callback cb, struct sproto_arg *args, uint8_t *data, int size) {
	uint8_t * buffer;
	int nCount = 0;
	int sz = 0;
	if (size < CC_SIZE_LENGTH)
		return -1;
	size -= CC_SIZE_LENGTH;
	buffer = data + CC_SIZE_LENGTH;
    uint64_t uKeyValue = 0;
    args->m_pMapKeyValue = &uKeyValue;
    if (args->m_nMapKeyType == SPROTO_CC_STRING){
        switch (args->type)
        {
        case SPROTO_CC_CHAR:
        case SPROTO_CC_UCHAR:
        {
            int nNeedLength = SPROTO_CC_CHAR_SIZE + CC_SIZE_LENGTH;
            args->index = 1;
            for (;;)
            {
                if (size < nNeedLength)
                    return -1;
                int v = 0;
                args->value = &v;
                args->length = sizeof(v);
                sz = cb(args);
                if (sz < 0){
                    if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
                        break;
                    return -1;	// sz == SPROTO_CB_ERROR
                }
                assert(sz == SPROTO_CC_CHAR_SIZE);
                if (size < uKeyValue + nNeedLength)
                    return -1;
                buffer[0] = uKeyValue & 0xff;
                buffer[1] = (uKeyValue >> 8) & 0xff;
                if (uKeyValue > 0)
                    memcpy(buffer + 2, args->m_pMapKeyString, uKeyValue);
                int nLength = uKeyValue + CC_SIZE_LENGTH;
                buffer[nLength] = v & 0xFF;
                size -= nNeedLength + uKeyValue;
                buffer += nNeedLength + uKeyValue;
                ++args->index;
                nCount++;
            }
            break;
        }
        case SPROTO_CC_SHORT:
        case SPROTO_CC_USHORT:
        {
            int nNeedLength = SPROTO_CC_SHORT_SIZE + CC_SIZE_LENGTH;
            args->index = 1;
            for (;;)
            {
                if (size < nNeedLength)
                    return -1;
                int v = 0;
                args->value = &v;
                args->length = sizeof(v);
                sz = cb(args);
                if (sz < 0){
                    if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
                        break;
                    return -1;	// sz == SPROTO_CB_ERROR
                }
                assert(sz == SPROTO_CC_SHORT_SIZE);
                if (size < uKeyValue + nNeedLength)
                    return -1;
                buffer[0] = uKeyValue & 0xff;
                buffer[1] = (uKeyValue >> 8) & 0xff;
                if (uKeyValue > 0)
                    memcpy(buffer + 2, args->m_pMapKeyString, uKeyValue);
                int nLength = uKeyValue + CC_SIZE_LENGTH;
                buffer[nLength] = v & 0xFF;
                buffer[nLength + 1] = (v >> 8) & 0xFF;
                size -= nNeedLength + uKeyValue;
                buffer += nNeedLength + uKeyValue;
                ++args->index;
                nCount++;
            }
            break;
        }
        case SPROTO_CC_INT:
        case SPROTO_CC_UINT:
        {
            int nNeedLength = SPROTO_CC_INT_SIZE + CC_SIZE_LENGTH;
            args->index = 1;
            for (;;)
            {
                if (size < nNeedLength)
                    return -1;
                int v = 0;
                args->value = &v;
                args->length = sizeof(v);
                sz = cb(args);
                if (sz < 0){
                    if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
                        break;
                    return -1;	// sz == SPROTO_CB_ERROR
                }
                assert(sz == SPROTO_CC_INT_SIZE);
                if (size < uKeyValue + nNeedLength)
                    return -1;
                buffer[0] = uKeyValue & 0xff;
                buffer[1] = (uKeyValue >> 8) & 0xff;
                if (uKeyValue > 0)
                    memcpy(buffer + 2, args->m_pMapKeyString, uKeyValue);
                int nLength = uKeyValue + CC_SIZE_LENGTH;
                buffer[nLength] = v & 0xFF;
                buffer[nLength + 1] = (v >> 8) & 0xFF;
                buffer[nLength + 2] = (v >> 16) & 0xFF;
                buffer[nLength + 3] = (v >> 24) & 0xFF;
                size -= nNeedLength + uKeyValue;
                buffer += nNeedLength + uKeyValue;
                ++args->index;
                nCount++;
            }
            break;
        }
        case SPROTO_CC_LONGLONG:
        {
            int nNeedLength = SPROTO_CC_LONGLONG_SIZE + CC_SIZE_LENGTH;
            args->index = 1;
            for (;;)
            {
                if (size < nNeedLength)
                    return -1;
                uint64_t v = 0;
                args->value = &v;
                args->length = sizeof(v);
                sz = cb(args);
                if (sz < 0){
                    if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
                        break;
                    return -1;	// sz == SPROTO_CB_ERROR
                }
                assert(sz == SPROTO_CC_LONGLONG_SIZE);
                if (size < uKeyValue + nNeedLength)
                    return -1;
                buffer[0] = uKeyValue & 0xff;
                buffer[1] = (uKeyValue >> 8) & 0xff;
                if (uKeyValue > 0)
                    memcpy(buffer + 2, args->m_pMapKeyString, uKeyValue);
                int nLength = uKeyValue + CC_SIZE_LENGTH;
                buffer[nLength] = v & 0xFF;
                buffer[nLength + 1] = (v >> 8) & 0xFF;
                buffer[nLength + 2] = (v >> 16) & 0xFF;
                buffer[nLength + 3] = (v >> 24) & 0xFF;
                buffer[nLength + 4] = (v >> 32) & 0xFF;
                buffer[nLength + 5] = (v >> 40) & 0xFF;
                buffer[nLength + 6] = (v >> 48) & 0xFF;
                buffer[nLength + 7] = (v >> 56) & 0xFF;
                size -= nNeedLength + uKeyValue;
                buffer += nNeedLength + uKeyValue;
                ++args->index;
                nCount++;
            }
            break;
        }
        case SPROTO_CC_DOUBLE:
        {
            int nNeedLength = SPROTO_CC_LONGLONG_SIZE + CC_SIZE_LENGTH;
            args->index = 1;
            for (;;)
            {
                if (size < nNeedLength)
                    return -1;
                double v = 0;
                args->value = &v;
                args->length = sizeof(v);
                sz = cb(args);
                if (sz < 0){
                    if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
                        break;
                    return -1;	// sz == SPROTO_CB_ERROR
                }
                assert(sz == SPROTO_CC_DOUBLE_SIZE);
                if (size < uKeyValue + nNeedLength)
                    return -1;
                buffer[0] = uKeyValue & 0xff;
                buffer[1] = (uKeyValue >> 8) & 0xff;
                if (uKeyValue > 0)
                    memcpy(buffer + 2, args->m_pMapKeyString, uKeyValue);
                int nLength = uKeyValue + CC_SIZE_LENGTH;
                buffer[nLength] = (*(uint64_t*)&v) & 0xFF;
                buffer[nLength + 1] = (*(uint64_t*)&v >> 8) & 0xFF;
                buffer[nLength + 2] = (*(uint64_t*)&v >> 16) & 0xFF;
                buffer[nLength + 3] = (*(uint64_t*)&v >> 24) & 0xFF;
                buffer[nLength + 4] = (*(uint64_t*)&v >> 32) & 0xFF;
                buffer[nLength + 5] = (*(uint64_t*)&v >> 40) & 0xFF;
                buffer[nLength + 6] = (*(uint64_t*)&v >> 48) & 0xFF;
                buffer[nLength + 7] = (*(uint64_t*)&v >> 56) & 0xFF;
                size -= nNeedLength + uKeyValue;
                buffer += nNeedLength + uKeyValue;
                ++args->index;
                nCount++;
            }
            break;
        }
        case SPROTO_CC_STRING:
        {
            int nNeedLength = CC_SIZE_LENGTH + CC_SIZE_LENGTH;
            args->index = 1;
            for (;;) {
                if (size < nNeedLength)
                    return -1;
                args->value = buffer;
                args->length = size;
                sz = cb(args);
                if (sz < 0)
                {
                    if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
                        break;
                    return -1;	// sz == SPROTO_CB_ERROR
                }
                int nTotalSize = uKeyValue + sz + nNeedLength;
                if (size < nTotalSize)
                    return -1;
                int nCopy = uKeyValue + nNeedLength;
                if (sz > 0)
                    memmove(buffer + nCopy, buffer, sz);
                buffer[uKeyValue + CC_SIZE_LENGTH] = sz & 0xff;
                buffer[uKeyValue + CC_SIZE_LENGTH + 1] = (sz >> 8) & 0xff;
                buffer[0] = uKeyValue & 0xff;
                buffer[1] = (uKeyValue >> 8) & 0xff;
                if (uKeyValue > 0)
                    memcpy(buffer + 2, args->m_pMapKeyString, uKeyValue);

                buffer += nTotalSize;
                size -= nTotalSize;
                ++args->index;
                nCount++;
            }
            break;
        }
        case SPROTO_CC_STRUCT:
        case SPROTO_CC_EXA_CNETBASICVALUE:
        {
            int nNeedLength = CC_SIZE_LENGTH;
            args->index = 1;
            for (;;)
            {
                if (size < nNeedLength)
                    return -1;
                args->value = buffer;
                args->length = size;
                sz = cb(args);
                if (sz < 0)
                {
                    if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
                        break;
                    return -1;	// sz == SPROTO_CB_ERROR
                }
                int nTotalSize = uKeyValue + sz + nNeedLength;
                if (size < nTotalSize)
                    return -1;
                int nCopy = uKeyValue + nNeedLength;
                if (sz > 0)
                    memmove(buffer + nCopy, buffer, sz);
                buffer[0] = uKeyValue & 0xff;
                buffer[1] = (uKeyValue >> 8) & 0xff;
                if (uKeyValue > 0)
                    memcpy(buffer + 2, args->m_pMapKeyString, uKeyValue);

                buffer += nTotalSize;
                size -= nTotalSize;
                ++args->index;
                nCount++;
            }
            break;
        }
        break;
        default:
            assert(0);
            break;
        }
    }
    else{
        encode_mapkey keyencode = NULL;
        int nKeyLength = 8;
        if (args->m_nMapKeyType == SPROTO_CC_CHAR || args->m_nMapKeyType == SPROTO_CC_UCHAR){
            keyencode = encodemapkey_ccchar;
            nKeyLength = SPROTO_CC_CHAR_SIZE;
        }
        else if (args->m_nMapKeyType == SPROTO_CC_SHORT || args->m_nMapKeyType == SPROTO_CC_USHORT){
            keyencode = encodemapkey_ccshrot;
            nKeyLength = SPROTO_CC_SHORT_SIZE;
        }
        else if (args->m_nMapKeyType == SPROTO_CC_INT || args->m_nMapKeyType == SPROTO_CC_UINT){
            keyencode = encodemapkey_ccint;
            nKeyLength = SPROTO_CC_INT_SIZE;
        }
        else if (args->m_nMapKeyType == SPROTO_CC_LONGLONG){
            keyencode = encodemapkey_cclonglong;
            nKeyLength = SPROTO_CC_LONGLONG_SIZE;
        }
        else if (args->m_nMapKeyType == SPROTO_CC_DOUBLE){
            keyencode = encodemapkey_ccdouble;
            nKeyLength = SPROTO_CC_DOUBLE_SIZE;
        }
        else{
            assert(0);
            return 0;
        }

        switch (args->type)
        {
        case SPROTO_CC_CHAR:
        case SPROTO_CC_UCHAR:
        {
            int nNeedLength = SPROTO_CC_CHAR_SIZE + nKeyLength;
            args->index = 1;
            for (;;)
            {
                if (size < nNeedLength)
                    return -1;
                int v = 0;
                args->value = &v;
                args->length = sizeof(v);
                sz = cb(args);
                if (sz < 0)
                {
                    if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
                        break;
                    return -1;	// sz == SPROTO_CB_ERROR
                }
                assert(sz == SPROTO_CC_CHAR_SIZE);
                int nLength = keyencode(uKeyValue, buffer);
                buffer[nLength] = v & 0xFF;
                size -= nNeedLength;
                buffer += nNeedLength;
                ++args->index;
                nCount++;
            }
            break;
        }
        case SPROTO_CC_SHORT:
        case SPROTO_CC_USHORT:
        {
            int nNeedLength = SPROTO_CC_SHORT_SIZE + nKeyLength;
            args->index = 1;
            for (;;)
            {
                if (size < nNeedLength)
                    return -1;
                int v = 0;
                args->value = &v;
                args->length = sizeof(v);
                sz = cb(args);
                if (sz < 0)
                {
                    if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
                        break;
                    return -1;	// sz == SPROTO_CB_ERROR
                }
                int nLength = keyencode(uKeyValue, buffer);
                assert(sz == SPROTO_CC_SHORT_SIZE);
                buffer[nLength] = v & 0xFF;
                buffer[nLength + 1] = (v >> 8) & 0xFF;
                size -= nNeedLength;
                buffer += nNeedLength;
                ++args->index;
                nCount++;
            }
            break;
        }
        case SPROTO_CC_INT:
        case SPROTO_CC_UINT:
        {
            int nNeedLength = SPROTO_CC_INT_SIZE + nKeyLength;
            args->index = 1;
            for (;;)
            {
                if (size < nNeedLength)
                    return -1;
                int v = 0;
                args->value = &v;
                args->length = sizeof(v);
                sz = cb(args);
                if (sz < 0)
                {
                    if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
                        break;
                    return -1;	// sz == SPROTO_CB_ERROR
                }
                int nLength = keyencode(uKeyValue, buffer);
                assert(sz == SPROTO_CC_INT_SIZE);
                buffer[nLength] = v & 0xFF;
                buffer[nLength + 1] = (v >> 8) & 0xFF;
                buffer[nLength + 2] = (v >> 16) & 0xFF;
                buffer[nLength + 3] = (v >> 24) & 0xFF;
                size -= nNeedLength;
                buffer += nNeedLength;
                ++args->index;
                nCount++;
            }
            break;
        }
        case SPROTO_CC_LONGLONG:
        {
            int nNeedLength = SPROTO_CC_LONGLONG_SIZE + nKeyLength;
            args->index = 1;
            for (;;)
            {
                if (size < nNeedLength)
                    return -1;
                uint64_t v = 0;
                args->value = &v;
                args->length = sizeof(v);
                sz = cb(args);
                if (sz < 0)
                {
                    if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
                        break;
                    return -1;	// sz == SPROTO_CB_ERROR
                }
                int nLength = keyencode(uKeyValue, buffer);
                assert(nLength == sz - SPROTO_CC_LONGLONG_SIZE);
                buffer[nLength] = v & 0xFF;
                buffer[nLength + 1] = (v >> 8) & 0xFF;
                buffer[nLength + 2] = (v >> 16) & 0xFF;
                buffer[nLength + 3] = (v >> 24) & 0xFF;
                buffer[nLength + 4] = (v >> 32) & 0xFF;
                buffer[nLength + 5] = (v >> 40) & 0xFF;
                buffer[nLength + 6] = (v >> 48) & 0xFF;
                buffer[nLength + 7] = (v >> 56) & 0xFF;
                size -= nNeedLength;
                buffer += nNeedLength;
                ++args->index;
                nCount++;
            }
            break;
        }
        case SPROTO_CC_DOUBLE:
        {
            int nNeedLength = SPROTO_CC_LONGLONG_SIZE + nKeyLength;
            args->index = 1;
            for (;;)
            {
                if (size < nNeedLength)
                    return -1;
                double v = 0;
                args->value = &v;
                args->length = sizeof(v);
                sz = cb(args);
                if (sz < 0)
                {
                    if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
                        break;
                    return -1;	// sz == SPROTO_CB_ERROR
                }
                int nLength = keyencode(uKeyValue, buffer);
                assert(sz == SPROTO_CC_DOUBLE_SIZE);
                buffer[nLength] = (*(uint64_t*)&v) & 0xFF;
                buffer[nLength + 1] = (*(uint64_t*)&v >> 8) & 0xFF;
                buffer[nLength + 2] = (*(uint64_t*)&v >> 16) & 0xFF;
                buffer[nLength + 3] = (*(uint64_t*)&v >> 24) & 0xFF;
                buffer[nLength + 4] = (*(uint64_t*)&v >> 32) & 0xFF;
                buffer[nLength + 5] = (*(uint64_t*)&v >> 40) & 0xFF;
                buffer[nLength + 6] = (*(uint64_t*)&v >> 48) & 0xFF;
                buffer[nLength + 7] = (*(uint64_t*)&v >> 56) & 0xFF;
                size -= nNeedLength;
                buffer += nNeedLength;
                ++args->index;
                nCount++;
            }
            break;
        }
        case SPROTO_CC_STRING:
        {
            int nNeedLength = CC_SIZE_LENGTH + nKeyLength;
            args->index = 1;
            for (;;) {
                if (size < nNeedLength)
                    return -1;
                size -= nNeedLength;
                args->value = buffer + nNeedLength;
                args->length = size;
                sz = cb(args);
                if (sz < 0)
                {
                    if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
                        break;
                    return -1;	// sz == SPROTO_CB_ERROR
                }
                if (size < sz)
                    return -1;
                int nLength = keyencode(uKeyValue, buffer);
                buffer[nLength] = sz & 0xff;
                buffer[nLength + 1] = (sz >> 8) & 0xff;

                buffer += nNeedLength + sz;
                size -= sz;
                ++args->index;
                nCount++;
            }
            break;
        }
        case SPROTO_CC_STRUCT:
        case SPROTO_CC_EXA_CNETBASICVALUE:
        {
            int nNeedLength = nKeyLength;
            args->index = 1;
            for (;;)
            {
                if (size < nNeedLength)
                    return -1;
                size -= nNeedLength;
                args->value = buffer + nNeedLength;
                args->length = size;
                sz = cb(args);
                if (sz < 0)
                {
                    if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
                        break;
                    return -1;	// sz == SPROTO_CB_ERROR
                }
                if (size < sz)
                    return -1;
                int nLength = keyencode(uKeyValue, buffer);

                buffer += nNeedLength + sz;
                size -= sz;
                ++args->index;
                nCount++;
            }
            break;
        }
        break;
        default:
            assert(0);
            break;
        }
    }
    //存的是count
    data[0] = nCount & 0xff;
    data[1] = (nCount >> 8) & 0xff;
	return buffer - data;
}

static int
encode_array(sproto_callback cb, struct sproto_arg *args, uint8_t *data, int size) {
	if (args->m_nMapKeyType > 0)
	{
		return encode_ccmap(cb, args, data, size);
	}
	uint8_t * buffer;
	int nCount = 0;
	int sz = 0;
	if (size < CC_SIZE_LENGTH)
		return -1;
	size -= CC_SIZE_LENGTH;
	buffer = data + CC_SIZE_LENGTH;
	switch (args->type)
	{
	case SPROTO_CC_CHAR:
	case SPROTO_CC_UCHAR:
	{
		args->index = 1;
		for (;;)
		{
			int v = 0;
			args->value = &v;
			args->length = sizeof(v);
			sz = cb(args);
			if (sz < 0)
			{
				if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
					break;
				return -1;	// sz == SPROTO_CB_ERROR
			}
			if (size < SPROTO_CC_CHAR_SIZE)
				return -1;
			buffer[0] = v & 0xFF;
			size -= SPROTO_CC_CHAR_SIZE;
			buffer += SPROTO_CC_CHAR_SIZE;
			++args->index;
			nCount++;
		}
		break;
	}
	case SPROTO_CC_SHORT:
	case SPROTO_CC_USHORT:
	{
		args->index = 1;
		for (;;)
		{
			int v = 0;
			args->value = &v;
			args->length = sizeof(v);
			sz = cb(args);
			if (sz < 0)
			{
				if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
					break;
				return -1;	// sz == SPROTO_CB_ERROR
			}
			if (size < SPROTO_CC_SHORT_SIZE)
				return -1;
			buffer[0] = v & 0xFF;
			buffer[1] = (v >> 8) & 0xFF;
			size -= SPROTO_CC_SHORT_SIZE;
			buffer += SPROTO_CC_SHORT_SIZE;
			++args->index;
			nCount++;
		}
		break;
	}
	case SPROTO_CC_INT:
	case SPROTO_CC_UINT:
	{
		args->index = 1;
		for (;;)
		{
			int v = 0;
			args->value = &v;
			args->length = sizeof(v);
			sz = cb(args);
			if (sz < 0)
			{
				if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
					break;
				return -1;	// sz == SPROTO_CB_ERROR
			}
			if (size < SPROTO_CC_INT_SIZE)
				return -1;
			buffer[0] = v & 0xFF;
			buffer[1] = (v >> 8) & 0xFF;
			buffer[2] = (v >> 16) & 0xFF;
			buffer[3] = (v >> 24) & 0xFF;
			size -= SPROTO_CC_INT_SIZE;
			buffer += SPROTO_CC_INT_SIZE;
			++args->index;
			nCount++;
		}
		break;
	}
	case SPROTO_CC_LONGLONG:
	{
		args->index = 1;
		for (;;)
		{
			uint64_t v = 0;
			args->value = &v;
			args->length = sizeof(v);
			sz = cb(args);
			if (sz < 0)
			{
				if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
					break;
				return -1;	// sz == SPROTO_CB_ERROR
			}
			if (size < SPROTO_CC_LONGLONG_SIZE)
				return -1;
			buffer[0] = v & 0xFF;
			buffer[1] = (v >> 8) & 0xFF;
			buffer[2] = (v >> 16) & 0xFF;
			buffer[3] = (v >> 24) & 0xFF;
			buffer[4] = (v >> 32) & 0xFF;
			buffer[5] = (v >> 40) & 0xFF;
			buffer[6] = (v >> 48) & 0xFF;
			buffer[7] = (v >> 56) & 0xFF;
			size -= SPROTO_CC_LONGLONG_SIZE;
			buffer += SPROTO_CC_LONGLONG_SIZE;
			++args->index;
			nCount++;
		}
		break;
	}
	case SPROTO_CC_DOUBLE:
	{
		args->index = 1;
		for (;;)
		{
			double v = 0;
			args->value = &v;
			args->length = sizeof(v);
			sz = cb(args);
			if (sz < 0)
			{
				if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
					break;
				return -1;	// sz == SPROTO_CB_ERROR
			}
			if (size < SPROTO_CC_LONGLONG_SIZE)
				return -1;
			buffer[0] = (*(uint64_t*)&v) & 0xFF;
			buffer[1] = (*(uint64_t*)&v >> 8) & 0xFF;
			buffer[2] = (*(uint64_t*)&v >> 16) & 0xFF;
			buffer[3] = (*(uint64_t*)&v >> 24) & 0xFF;
			buffer[4] = (*(uint64_t*)&v >> 32) & 0xFF;
			buffer[5] = (*(uint64_t*)&v >> 40) & 0xFF;
			buffer[6] = (*(uint64_t*)&v >> 48) & 0xFF;
			buffer[7] = (*(uint64_t*)&v >> 56) & 0xFF;
			size -= SPROTO_CC_LONGLONG_SIZE;
			buffer += SPROTO_CC_LONGLONG_SIZE;
			++args->index;
			nCount++;
		}
		break;
	}
	case SPROTO_CC_STRING:
	{
		args->index = 1;
		for (;;) {
			if (size < CC_SIZE_LENGTH)
				return -1;
			size -= CC_SIZE_LENGTH;
			args->value = buffer + CC_SIZE_LENGTH;
			args->length = size;
			sz = cb(args);
			if (sz < 0)
			{
				if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
					break;
				return -1;	// sz == SPROTO_CB_ERROR
			}
			buffer[0] = sz & 0xff;
			buffer[1] = (sz >> 8) & 0xff;

			buffer += CC_SIZE_LENGTH + sz;
			size -= sz;
			++args->index;
			nCount++;
		}
		break;
	}
	case SPROTO_CC_STRUCT:
	case SPROTO_CC_EXA_CNETBASICVALUE:
	{
		args->index = 1;
		for (;;) 
		{
			args->value = buffer;
			args->length = size;
			sz = cb(args);
			if (sz < 0)
			{
				if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)		// nil object , end of array
					break;
				return -1;	// sz == SPROTO_CB_ERROR
			}
			buffer += sz;
			size -= sz;
			++args->index;
			nCount++;
		}
	}
		break;
	default:
		assert(0);
		break;
	}
	//存的是count
	data[0] = nCount & 0xff;
	data[1] = (nCount >> 8) & 0xff;

	return buffer - data;
}

int
sproto_encode(const struct sproto_type *st, void * buffer, int size, sproto_callback cb, void *ud) {
	struct sproto_arg args;
	uint8_t * header = (uint8_t *)buffer;
	uint8_t * data = header;
	int index = 0;
	args.ud = ud;
	for (int i=0;i<st->n;i++) 
	{
		struct field *f = &st->f[i];
		int type = f->type;
		int sz = -1;
		args.tagname = f->name;
		args.m_bStar = f->m_bStar;
		uint8_t* pStarValue = data;
		if (args.m_bStar)
			data += 1;
		args.subtype = f->st;
		args.m_nMapKeyType = f->m_nKeyType;
		args.m_nDefaultValue = f->m_nDefaultValue;
		if (type & SPROTO_TARRAY) 
		{
			args.type = type & ~SPROTO_TARRAY;
			sz = encode_array(cb, &args, data, size);
		} 
		else 
		{
			args.type = type;
			args.index = 0;
			switch(type) 
			{
			case SPROTO_CC_CHAR:
			case SPROTO_CC_UCHAR:
			{
				unsigned char ccCharvalue;
				args.value = &ccCharvalue;
				args.length = sizeof(ccCharvalue);
				sz = cb(&args);
				if (sz < 0) {
					if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)
						continue;
					return -1;	// sz == SPROTO_CB_ERROR
				}
				else if (sz == 1){
					sz = encode_ccchar(ccCharvalue, data, size);
				}
				else{
					return -1;
				}
				break;
			}
			case SPROTO_CC_SHORT:
			case SPROTO_CC_USHORT:
			{
				unsigned short ccCharvalue;
				args.value = &ccCharvalue;
				args.length = sizeof(ccCharvalue);
				sz = cb(&args);
				if (sz < 0) {
					if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)
						continue;
					return -1;	// sz == SPROTO_CB_ERROR
				}
				else if (sz == 2){
					sz = encode_ccshort(ccCharvalue, data, size);
				}
				else{
					return -1;
				}
				break;
			}
			case SPROTO_CC_INT:
			case SPROTO_CC_UINT:
			{
				uint32_t ccCharvalue;
				args.value = &ccCharvalue;
				args.length = sizeof(ccCharvalue);
				sz = cb(&args);
				if (sz < 0) {
					if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)
						continue;
					return -1;	// sz == SPROTO_CB_ERROR
				}
				else if (sz == 4){
					sz = encode_ccinteger(ccCharvalue, data, size);
				}
				else{
					return -1;
				}
				break;
			}
			case SPROTO_CC_LONGLONG:{
				uint64_t ccCharvalue;
				args.value = &ccCharvalue;
				args.length = sizeof(ccCharvalue);
				sz = cb(&args);
				if (sz < 0) {
					if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)
						continue;
					return -1;	// sz == SPROTO_CB_ERROR
				}
				else if (sz == 8){
					sz = encode_cclonglong(ccCharvalue, data, size);
				}
				else{
					return -1;
				}
				break;
			}
			case SPROTO_CC_DOUBLE:
			{
				double ccCharvalue;
				args.value = &ccCharvalue;
				args.length = sizeof(ccCharvalue);
				sz = cb(&args);
				if (sz < 0) {
					if (sz == SPROTO_CB_NIL || sz == SPROTO_CB_NOARRAY)
						continue;
					return -1;	// sz == SPROTO_CB_ERROR
				}
				else if (sz == 8){
					sz = encode_ccdouble(ccCharvalue, data, size);
				}
				else{
					return -1;
				}
				break;
			}
			case SPROTO_CC_STRING:
			{
				if (size < CC_SIZE_LENGTH)
					return -1;
				args.value = data + CC_SIZE_LENGTH;
				args.length = size - CC_SIZE_LENGTH;
				sz = cb(&args);
				if (sz < 0){
					if (sz == SPROTO_CB_NIL)
						continue;
					return -1;	// sz == SPROTO_CB_ERROR
				}
				assert(sz <= size - CC_SIZE_LENGTH);	// verify buffer overflow
				data[0] = sz & 0xff;
				data[1] = (sz >> 8) & 0xff;
				sz = sz + CC_SIZE_LENGTH;
				break;
			}
			case SPROTO_CC_STRUCT:
			case SPROTO_CC_EXA_CNETBASICVALUE:
				args.value = data;
				args.length = size;
				sz = cb(&args);
				if (sz < 0){
					if (sz == SPROTO_CB_NIL)
						continue;
					return -1;	// sz == SPROTO_CB_ERROR
				}
				assert(sz <= size);	// verify buffer overflow
				break;
			}
		}
		if (sz < 0)
			return sz;
		if (f->m_bStar){
			pStarValue[0] = (sz > 0) ? 1 : 0;
		}
		if (sz > 0){
			data += sz;
			size -= sz;
		}
	}
	return data - (header);
}

static inline uint64_t
expand64(uint32_t v) {
	uint64_t value = v;
	if (value & 0x80000000) {
		value |= (uint64_t)~0  << 32 ;
	}
	return value;
}


static int
decode_ccmap(sproto_callback cb, struct sproto_arg *args, uint8_t * stream, int nSize)
{
	if (nSize < CC_SIZE_LENGTH)
		return -1;
	const uint8_t* pResStream = stream;
	int nCount = toword(stream);
	stream += CC_SIZE_LENGTH;
	nSize -= CC_SIZE_LENGTH;
	if (nCount == 0)
	{
		// It's empty array, call cb with index == -1 to create the empty array.
		args->index = -1;
		args->value = NULL;
		args->length = 0;
		cb(args);
		return stream - pResStream;
	}


    if (args->m_nMapKeyType == SPROTO_CC_STRING){
        switch (args->type)
        {
        case SPROTO_CC_CHAR:
        case SPROTO_CC_UCHAR:
        {
            uint64_t nKeyLength = 0;
            for (int i = 0; i<nCount; i++){
                if (nSize < CC_SIZE_LENGTH)
                    return -1;
                nKeyLength = toword(stream);
                int nTotalLength = CC_SIZE_LENGTH + nKeyLength + SPROTO_CC_CHAR_SIZE;
                if (nSize < nTotalLength)
                    return -1;
                args->m_pMapKeyString = (const char*)stream + CC_SIZE_LENGTH;
                
                uint64_t value = stream[nKeyLength + CC_SIZE_LENGTH];
                args->index = i + 1;
                args->value = &value;
                args->m_pMapKeyValue = &nKeyLength;
                cb(args);
                stream += nTotalLength;
                nSize -= nTotalLength;
            }
            break;
        }
        case SPROTO_CC_SHORT:
        case SPROTO_CC_USHORT:
        {
            uint64_t nKeyLength = 0;
            for (int i = 0; i<nCount; i++){
                if (nSize < CC_SIZE_LENGTH)
                    return -1;
                nKeyLength = toword(stream);
                int nTotalLength = CC_SIZE_LENGTH + nKeyLength + SPROTO_CC_SHORT_SIZE;
                if (nSize < nTotalLength)
                    return -1;
                args->m_pMapKeyString = (const char*)stream + CC_SIZE_LENGTH;

                uint64_t value = toword(stream + nKeyLength + CC_SIZE_LENGTH);
                args->index = i + 1;
                args->value = &value;
                args->m_pMapKeyValue = &nKeyLength;
                cb(args);
                stream += nTotalLength;
                nSize -= nTotalLength;
            }
            break;
        }
        case SPROTO_CC_INT:
        case SPROTO_CC_UINT:
        {
            uint64_t nKeyLength = 0;
            for (int i = 0; i<nCount; i++){
                if (nSize < CC_SIZE_LENGTH)
                    return -1;
                nKeyLength = toword(stream);
                int nTotalLength = CC_SIZE_LENGTH + nKeyLength + SPROTO_CC_INT_SIZE;
                if (nSize < nTotalLength)
                    return -1;
                args->m_pMapKeyString = (const char*)stream + CC_SIZE_LENGTH;

                uint64_t value = todword(stream + nKeyLength + CC_SIZE_LENGTH);
                args->index = i + 1;
                args->value = &value;
                args->m_pMapKeyValue = &nKeyLength;
                cb(args);
                stream += nTotalLength;
                nSize -= nTotalLength;
            }
            break;
        }
        case SPROTO_CC_LONGLONG:
        {
            uint64_t nKeyLength = 0;
            for (int i = 0; i<nCount; i++){
                if (nSize < CC_SIZE_LENGTH)
                    return -1;
                nKeyLength = toword(stream);
                int nTotalLength = CC_SIZE_LENGTH + nKeyLength + SPROTO_CC_LONGLONG_SIZE;
                if (nSize < nTotalLength)
                    return -1;
                args->m_pMapKeyString = (const char*)stream + CC_SIZE_LENGTH;

                uint64_t value = longlongtokey(stream + nKeyLength + CC_SIZE_LENGTH);
                args->index = i + 1;
                args->value = &value;
                args->m_pMapKeyValue = &nKeyLength;
                cb(args);
                stream += nTotalLength;
                nSize -= nTotalLength;
            }
            break;
        }
        case SPROTO_CC_DOUBLE:
        {
            uint64_t nKeyLength = 0;
            for (int i = 0; i<nCount; i++){
                if (nSize < CC_SIZE_LENGTH)
                    return -1;
                nKeyLength = toword(stream);
                int nTotalLength = CC_SIZE_LENGTH + nKeyLength + SPROTO_CC_DOUBLE_SIZE;
                if (nSize < nTotalLength)
                    return -1;
                args->m_pMapKeyString = (const char*)stream + CC_SIZE_LENGTH;

                uint64_t value = longlongtokey(stream + nKeyLength + CC_SIZE_LENGTH);
                double dValue = *(double*)&value;
                args->index = i + 1;
                args->value = &dValue;
                args->m_pMapKeyValue = &nKeyLength;
                cb(args);
                stream += nTotalLength;
                nSize -= nTotalLength;
            }
            break;
        }
        case SPROTO_CC_STRING:
        {
            uint64_t nKeyLength = 0;
            for (int i = 0; i<nCount; i++){
                if (nSize < CC_SIZE_LENGTH)
                    return -1;
                nKeyLength = toword(stream);
                int nTotalLength = CC_SIZE_LENGTH + nKeyLength + CC_SIZE_LENGTH;
                if (nSize < nTotalLength)
                    return -1;
                args->m_pMapKeyString = (const char*)stream + CC_SIZE_LENGTH;
                uint32_t hsz = toword(stream + CC_SIZE_LENGTH + nKeyLength);
                if (nSize < nTotalLength + hsz)
                    return -1;

                args->index = i + 1;
                args->value = stream + nTotalLength;
                args->length = hsz;
                args->m_pMapKeyValue = &nKeyLength;
                if (cb(args))
                    return -1;
                stream += nTotalLength + hsz;
                nSize -= nTotalLength + hsz;
            }
            break;
        }
        case SPROTO_CC_STRUCT:
        case SPROTO_CC_EXA_CNETBASICVALUE:
        {
            uint64_t nKeyLength = 0;
            for (int i = 0; i<nCount; i++){
                if (nSize < CC_SIZE_LENGTH)
                    return -1;
                nKeyLength = toword(stream);
                int nTotalLength = CC_SIZE_LENGTH + nKeyLength;
                if (nSize < nTotalLength)
                    return -1;
                args->m_pMapKeyString = (const char*)stream + CC_SIZE_LENGTH;
                
                args->index = i + 1;
                args->value = stream + nTotalLength;
                args->length = nSize - nTotalLength;
                args->m_pMapKeyValue = &nKeyLength;
                int nReadSize = cb(args);
                //特殊处理
                if (nReadSize < 0)
                    return -1;
                stream += nTotalLength + nReadSize;
                nSize -= nTotalLength + nReadSize;
            }
            break;
        }
        default:
            return -1;
        }
    }
    else{
        decode_keymap keydecode = NULL;
        int nKeyLength = 8;
        if (args->m_nMapKeyType == SPROTO_CC_CHAR || args->m_nMapKeyType == SPROTO_CC_UCHAR)
        {
            keydecode = chartokey;
            nKeyLength = SPROTO_CC_CHAR_SIZE;
        }
        else if (args->m_nMapKeyType == SPROTO_CC_SHORT || args->m_nMapKeyType == SPROTO_CC_USHORT)
        {
            keydecode = shorttokey;
            nKeyLength = SPROTO_CC_SHORT_SIZE;
        }
        else if (args->m_nMapKeyType == SPROTO_CC_INT || args->m_nMapKeyType == SPROTO_CC_UINT)
        {
            keydecode = inttokey;
            nKeyLength = SPROTO_CC_INT_SIZE;
        }
        else if (args->m_nMapKeyType == SPROTO_CC_LONGLONG)
        {
            keydecode = longlongtokey;
            nKeyLength = SPROTO_CC_LONGLONG_SIZE;
        }
        else if (args->m_nMapKeyType == SPROTO_CC_DOUBLE)
        {
            keydecode = doubletokey;
            nKeyLength = SPROTO_CC_DOUBLE_SIZE;
        }
        else
        {
            assert(0);
            return 0;
        }
        switch (args->type)
        {
        case SPROTO_CC_CHAR:
        case SPROTO_CC_UCHAR:
        {
            int nEveryItemSize = nKeyLength + SPROTO_CC_CHAR_SIZE;
            int nArraySize = nCount * nEveryItemSize;
            if (nSize < nArraySize)
                return -1;
            for (int i = 0; i<nCount; i++)
            {
                uint64_t mapkeyvalue = keydecode(stream + i * nEveryItemSize);
                uint64_t value = stream[i * nEveryItemSize + nKeyLength];
                args->index = i + 1;
                args->value = &value;
                args->m_pMapKeyValue = &mapkeyvalue;
                cb(args);
            }
            stream += nArraySize;
            break;
        }
        case SPROTO_CC_SHORT:
        case SPROTO_CC_USHORT:
        {
            int nEveryItemSize = nKeyLength + SPROTO_CC_SHORT_SIZE;
            int nArraySize = nCount * nEveryItemSize;
            if (nSize < nArraySize)
                return -1;
            for (int i = 0; i<nCount; i++)
            {
                uint64_t mapkeyvalue = keydecode(stream + i * nEveryItemSize);
                uint64_t value = toword(stream + i * nEveryItemSize + nKeyLength);
                args->index = i + 1;
                args->value = &value;
                args->m_pMapKeyValue = &mapkeyvalue;
                cb(args);
            }
            stream += nArraySize;
            break;
        }
        case SPROTO_CC_INT:
        case SPROTO_CC_UINT:
        {
            int nEveryItemSize = nKeyLength + SPROTO_CC_INT_SIZE;
            int nArraySize = nCount * nEveryItemSize;
            if (nSize < nArraySize)
                return -1;
            for (int i = 0; i<nCount; i++)
            {
                uint64_t mapkeyvalue = keydecode(stream + i * nEveryItemSize);
                uint64_t value = todword(stream + i * nEveryItemSize + nKeyLength);
                args->index = i + 1;
                args->value = &value;
                args->m_pMapKeyValue = &mapkeyvalue;
                cb(args);
            }
            stream += nArraySize;
            break;
        }
        case SPROTO_CC_LONGLONG:
        {
            int nEveryItemSize = nKeyLength + SPROTO_CC_LONGLONG_SIZE;
            int nArraySize = nCount * nEveryItemSize;
            if (nSize < nArraySize)
                return -1;
            for (int i = 0; i<nCount; i++)
            {
                uint64_t mapkeyvalue = keydecode(stream + i * nEveryItemSize);
                uint64_t value = longlongtokey(stream + i * nEveryItemSize + nKeyLength);
                args->index = i + 1;
                args->value = &value;
                args->m_pMapKeyValue = &mapkeyvalue;
                cb(args);
            }
            stream += nArraySize;
            break;
        }
        case SPROTO_CC_DOUBLE:
        {
            int nEveryItemSize = nKeyLength + SPROTO_CC_DOUBLE_SIZE;
            int nArraySize = nCount * nEveryItemSize;
            if (nSize < nArraySize)
                return -1;
            for (int i = 0; i<nCount; i++)
            {
                uint64_t mapkeyvalue = keydecode(stream + i * nEveryItemSize);
                uint64_t value = longlongtokey(stream + i * nEveryItemSize + nKeyLength);
                double dValue = *(double*)&value;
                args->index = i + 1;
                args->value = &dValue;
                args->m_pMapKeyValue = &mapkeyvalue;
                cb(args);
            }
            stream += nArraySize;
            break;
        }
        case SPROTO_CC_STRING:
        {
            int nEveryItemSize = nKeyLength + CC_SIZE_LENGTH;
            uint32_t hsz;
            for (int i = 0; i < nCount; i++)
            {
                if (nSize < nEveryItemSize)
                    return -1;
                uint64_t mapkeyvalue = keydecode(stream);

                hsz = toword(stream);
                stream += nEveryItemSize;
                nSize -= nEveryItemSize;
                if (nSize < hsz)
                    return -1;

                args->index = i + 1;
                args->value = stream;
                args->length = hsz;
                args->m_pMapKeyValue = &mapkeyvalue;
                if (cb(args))
                    return -1;
                stream += hsz;
                nSize -= hsz;
            }
            break;
        }
        case SPROTO_CC_STRUCT:
        case SPROTO_CC_EXA_CNETBASICVALUE:
        {
            int index = 1;
            for (int i = 0; i < nCount; i++)
            {
                if (nSize < nKeyLength)
                    return -1;
                uint64_t mapkeyvalue = keydecode(stream);
                stream += nKeyLength;
                nSize -= nKeyLength;

                args->index = index;
                args->value = stream;
                args->length = nSize;
                args->m_pMapKeyValue = &mapkeyvalue;
                int nReadSize = cb(args);
                //特殊处理
                if (nReadSize < 0)
                    return -1;
                stream += nReadSize;
                nSize -= nReadSize;
                ++index;
            }
            break;
        }
        default:
            return -1;
        }
    }
	return stream - pResStream;
}

static int
decode_array(sproto_callback cb, struct sproto_arg *args, uint8_t * stream, int nSize) 
{
	if (args->m_nMapKeyType > 0)
	{
		return decode_ccmap(cb, args, stream, nSize);
	}
	if (nSize < CC_SIZE_LENGTH)
		return -1;
	const uint8_t* pResStream = stream;
	int nCount = toword(stream);
	stream += CC_SIZE_LENGTH;
	nSize -= CC_SIZE_LENGTH;
	if (nCount == 0) 
	{
		// It's empty array, call cb with index == -1 to create the empty array.
		args->index = -1;
		args->value = NULL;
		args->length = 0;
		cb(args);
		return stream - pResStream;
	}	

	int type = args->type;
	int i;
	
	switch (type) 
	{
	case SPROTO_CC_CHAR:
	case SPROTO_CC_UCHAR:
	{
		int nArraySize = nCount * SPROTO_CC_CHAR_SIZE;
		if (nSize < nArraySize)
			return -1;
		for (i = 0; i<nCount; i++) 
		{
			uint64_t value = stream[i];
			args->index = i + 1;
			args->value = &value;
			cb(args);
		}
		stream += nArraySize;
		nSize -= nArraySize;
		break;
	}
	case SPROTO_CC_SHORT:
	case SPROTO_CC_USHORT:
	{
		int nArraySize = nCount * SPROTO_CC_SHORT_SIZE;
		if (nSize < nArraySize)
			return -1;
		for (i = 0; i<nCount; i++) 
		{
			uint64_t value = toword(stream + i * sizeof(unsigned short));
			args->index = i + 1;
			args->value = &value;
			args->length = sizeof(value);
			cb(args);
		}
		stream += nArraySize;
		nSize -= nArraySize;
		break;
	}
	case SPROTO_CC_INT:
	case SPROTO_CC_UINT:
	{
		int nArraySize = nCount * SPROTO_CC_INT_SIZE;
		if (nSize < nArraySize)
			return -1;
		for (i = 0; i<nCount; i++) 
		{
			uint64_t value = todword(stream + i * sizeof(uint32_t));
			args->index = i + 1;
			args->value = &value;
			args->length = sizeof(value);
			cb(args);
		}
		stream += nArraySize;
		nSize -= nArraySize;
		break;
	}
	case SPROTO_CC_LONGLONG:
	{
		int nArraySize = nCount * SPROTO_CC_LONGLONG_SIZE;
		if (nSize < nArraySize)
			return -1;
		for (i = 0; i<nCount; i++) 
		{
			uint64_t low = todword(stream + i*sizeof(uint64_t));
			uint64_t hi = todword(stream + i*sizeof(uint64_t) + sizeof(uint32_t));
			uint64_t value = low | hi << 32;
			args->index = i + 1;
			args->value = &value;
			args->length = sizeof(value);
			cb(args);
		}
		stream += nArraySize;
		nSize -= nArraySize;
		break;
	}
	case SPROTO_CC_DOUBLE:
	{
		int nArraySize = nCount * SPROTO_CC_DOUBLE_SIZE;
		if (nSize < nArraySize)
			return -1;
		for (i = 0; i<nCount; i++) 
		{
			uint64_t low = todword(stream + i*sizeof(uint64_t));
			uint64_t hi = todword(stream + i*sizeof(uint64_t) + sizeof(uint32_t));
			uint64_t value = low | hi << 32;
			double dValue = *(double*)&value;
			args->index = i + 1;
			args->value = &dValue;
			args->length = sizeof(value);
			cb(args);
		}
		stream += nArraySize;
		nSize -= nArraySize;
		break;
	}
	case SPROTO_CC_STRING:
	{
		uint32_t hsz;
		int index = 1;
		for (int i = 0; i < nCount; i++)
		{
			if (nSize < CC_SIZE_LENGTH)
				return -1;
			hsz = toword(stream);
			stream += CC_SIZE_LENGTH;
			nSize -= CC_SIZE_LENGTH;

			args->index = index;
			args->value = stream;
			args->length = hsz;
			if (cb(args))
				return -1;
			stream += hsz;
			nSize -= hsz;
			++index;
		}
		break;
	}
	case SPROTO_CC_STRUCT:
	case SPROTO_CC_EXA_CNETBASICVALUE:
	{
		int index = 1;
		for (int i = 0; i < nCount; i++)
		{
			args->index = index;
			args->value = stream;
			args->length = nSize;
			int nReadSize = cb(args);
			//特殊处理
			if (nReadSize < 0)
				return -1;
			stream += nReadSize;
			nSize -= nReadSize;
			++index;
		}
		break;
	}
	default:
		return -1;
	}
	return stream - pResStream;
}

int
sproto_decode(const struct sproto_type *st, const void * data, int size, sproto_callback cb, void *ud) 
{
	struct sproto_arg args;
	int total = size;
	uint8_t * stream;
	uint8_t * datastream;
	
	stream = (uint8_t *)data;
	datastream = stream;// +fn * SIZEOF_FIELD;
	args.ud = ud;
	for (int i = 0; i<st->n; i++) 
	{
		struct field *f = &st->f[i];
		args.tagname = f->name;
		args.m_bStar = f->m_bStar;
		if (f->m_bStar){
			datastream += 1;
			size -= 1;
			if (!datastream[0]){
				continue;
			}
		}
		uint8_t* currentdata = datastream;

		args.type = f->type & ~SPROTO_TARRAY;
		args.subtype = f->st;
		args.index = 0;
		args.m_nMapKeyType = f->m_nKeyType;
		if (f->type & SPROTO_TARRAY)
		{
			int decodeSize = decode_array(cb, &args, currentdata, size);
			if (decodeSize < 0)
			{
				return -1;
			}
			datastream += decodeSize;
			size -= decodeSize;
		}
		else
		{
			switch (f->type)
			{
			case SPROTO_CC_CHAR:
			case SPROTO_CC_UCHAR:
			{
				if (size < SPROTO_CC_CHAR_SIZE)
					return -1;
				uint64_t value = currentdata[0];
				args.value = &value;
				cb(&args);

				datastream += SPROTO_CC_CHAR_SIZE;
				size -= SPROTO_CC_CHAR_SIZE;

				break;
			}
			case SPROTO_CC_SHORT:
			case SPROTO_CC_USHORT:
			{
				if (size < SPROTO_CC_SHORT_SIZE)
					return -1;
				uint64_t value = toword(currentdata);
				args.value = &value;
				cb(&args);

				datastream += SPROTO_CC_SHORT_SIZE;
				size -= SPROTO_CC_SHORT_SIZE;
				break;
			}
			case SPROTO_CC_INT:
			case SPROTO_CC_UINT:
			{
				if (size < SPROTO_CC_INT_SIZE)
					return -1;
				uint64_t value = todword(currentdata);
				args.value = &value;
				cb(&args);

				datastream += SPROTO_CC_INT_SIZE;
				size -= SPROTO_CC_INT_SIZE;
				break;
			}
			case SPROTO_CC_LONGLONG:
			{
				if (size < SPROTO_CC_LONGLONG_SIZE)
					return -1;
				uint64_t value = longlongtokey(currentdata);
				args.value = &value;
				cb(&args);

				datastream += SPROTO_CC_LONGLONG_SIZE;
				size -= SPROTO_CC_LONGLONG_SIZE;
				break;
			}
			case SPROTO_CC_DOUBLE:
			{
				if (size < SPROTO_CC_DOUBLE_SIZE)
					return -1;
				double value = *(double*)longlongtokey(currentdata);
				args.value = &value;
				cb(&args);

				datastream += SPROTO_CC_DOUBLE_SIZE;
				size -= SPROTO_CC_DOUBLE_SIZE;
				break;
			}
			case SPROTO_CC_STRING:
			{
				if (size < CC_SIZE_LENGTH)
					return -1;
				uint32_t sz = toword(currentdata);
				if (size < sz + CC_SIZE_LENGTH)
					return -1;
				args.value = currentdata + CC_SIZE_LENGTH;
				args.length = sz;
				if (cb(&args) < 0)
					return -1;
				datastream += CC_SIZE_LENGTH + sz;
				size -= CC_SIZE_LENGTH + sz;
				break;
			}
			case SPROTO_CC_STRUCT:
			case SPROTO_CC_EXA_CNETBASICVALUE:
			{
				args.value = currentdata;
				args.length = size;
				int sz = cb(&args);
				//特殊处理
				if (sz < 0)
					return -1;
				datastream += sz;
				size -= sz;
				break;
			}
			default:
				return -1;
			}
		}
	}
	return total - size;
}

// 0 pack

static int
pack_seg(const uint8_t *src, uint8_t * buffer, int sz, int n) {
	uint8_t header = 0;
	int notzero = 0;
	int i;
	uint8_t * obuffer = buffer;
	++buffer;
	--sz;
	if (sz < 0)
		obuffer = NULL;

	for (i=0;i<8;i++) {
		if (src[i] != 0) {
			notzero++;
			header |= 1<<i;
			if (sz > 0) {
				*buffer = src[i];
				++buffer;
				--sz;
			}
		}
	}
	if ((notzero == 7 || notzero == 6) && n > 0) {
		notzero = 8;
	}
	if (notzero == 8) {
		if (n > 0) {
			return 8;
		} else {
			return 10;
		}
	}
	if (obuffer) {
		*obuffer = header;
	}
	return notzero + 1;
}

static inline void
write_ff(const uint8_t * src, uint8_t * des, int n) {
	int i;
	int align8_n = (n+7)&(~7);

	des[0] = 0xff;
	des[1] = align8_n/8 - 1;
	memcpy(des+2, src, n);
	for(i=0; i< align8_n-n; i++){
		des[n+2+i] = 0;
	}
}

int
sproto_pack(const void * srcv, int srcsz, void * bufferv, int bufsz) {
	uint8_t tmp[8];
	int i;
	const uint8_t * ff_srcstart = NULL;
	uint8_t * ff_desstart = NULL;
	int ff_n = 0;
	int size = 0;
	const uint8_t * src = (const uint8_t *)srcv;
	uint8_t * buffer = (uint8_t *)bufferv;
	for (i=0;i<srcsz;i+=8) {
		int n;
		int padding = i+8 - srcsz;
		if (padding > 0) {
			int j;
			memcpy(tmp, src, 8-padding);
			for (j=0;j<padding;j++) {
				tmp[7-j] = 0;
			}
			src = tmp;
		}
		n = pack_seg(src, buffer, bufsz, ff_n);
		bufsz -= n;
		if (n == 10) {
			// first FF
			ff_srcstart = src;
			ff_desstart = buffer;
			ff_n = 1;
		} else if (n==8 && ff_n>0) {
			++ff_n;
			if (ff_n == 256) {
				if (bufsz >= 0) {
					write_ff(ff_srcstart, ff_desstart, 256*8);
				}
				ff_n = 0;
			}
		} else {
			if (ff_n > 0) {
				if (bufsz >= 0) {
					write_ff(ff_srcstart, ff_desstart, ff_n*8);
				}
				ff_n = 0;
			}
		}
		src += 8;
		buffer += n;
		size += n;
	}
	if(bufsz >= 0){
		if(ff_n == 1)
			write_ff(ff_srcstart, ff_desstart, 8);
		else if (ff_n > 1)
			write_ff(ff_srcstart, ff_desstart, srcsz - (intptr_t)(ff_srcstart - (const uint8_t*)srcv));
	}
	return size;
}

int
sproto_unpack(const void * srcv, int srcsz, void * bufferv, int bufsz) {
	const uint8_t * src = (const uint8_t *)srcv;
	uint8_t * buffer = (uint8_t *)bufferv;
	int size = 0;
	while (srcsz > 0) {
		uint8_t header = src[0];
		--srcsz;
		++src;
		if (header == 0xff) {
			int n;
			if (srcsz < 0) {
				return -1;
			}
			n = (src[0] + 1) * 8;
			if (srcsz < n + 1)
				return -1;
			srcsz -= n + 1;
			++src;
			if (bufsz >= n) {
				memcpy(buffer, src, n);
			}
			bufsz -= n;
			buffer += n;
			src += n;
			size += n;
		} else {
			int i;
			for (i=0;i<8;i++) {
				int nz = (header >> i) & 1;
				if (nz) {
					if (srcsz < 0)
						return -1;
					if (bufsz > 0) {
						*buffer = *src;
						--bufsz;
						++buffer;
					}
					++src;
					--srcsz;
				} else {
					if (bufsz > 0) {
						*buffer = 0;
						--bufsz;
						++buffer;
					}
				}
				++size;
			}
		}
	}
	return size;
}
