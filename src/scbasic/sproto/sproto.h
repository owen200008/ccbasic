#ifndef sproto_h
#define sproto_h

#include <stddef.h>

struct field {
    int m_bStar;
    int type;
    const char * name;
    struct sproto_type * st;
    int			m_nKeyType;
    uint64_t	m_nDefaultValue;
};

struct chunk {
    struct chunk * next;
};

struct pool {
    struct chunk * header;
    struct chunk * current;
    int current_used;
};
struct sproto {
    struct pool memory;
    int type_n;
    struct sproto_type * type;
};
struct sproto_type {
    const char * name;
    int n;
    //int base;
    struct field *f;
};

//支持的类型
#define SPROTO_CC_CHAR		10
#define SPROTO_CC_SHORT		11
#define SPROTO_CC_INT		12
#define SPROTO_CC_LONGLONG	13
#define SPROTO_CC_DOUBLE	14
#define SPROTO_CC_UCHAR		15
#define SPROTO_CC_USHORT	16
#define SPROTO_CC_UINT		17
#define SPROTO_CC_STRING	18
#define SPROTO_CC_STRUCT	19

#define SPROTO_CC_EXA_BEGIN				100
#define SPROTO_CC_EXA_CNETBASICVALUE	100
#define SPROTO_CC_EXA_END				200

#define SPROTO_CB_ERROR -1
#define SPROTO_CB_NIL -2
#define SPROTO_CB_NOARRAY -3

#define SPROTO_CC_CHAR_SIZE		1
#define SPROTO_CC_SHORT_SIZE	2
#define SPROTO_CC_INT_SIZE		4
#define SPROTO_CC_LONGLONG_SIZE	8
#define SPROTO_CC_DOUBLE_SIZE	8
#define SPROTO_CC_STRING_SIZE	2//+长度
#define SPROTO_CC_STRUCT_SIZE	0//+内容

struct sproto * sproto_create(const void * proto, size_t sz);
void sproto_release(struct sproto *);

struct sproto_type *  sproto_type(const struct sproto *, const char * type_name);

int sproto_pack(const void * src, int srcsz, void * buffer, int bufsz);
int sproto_unpack(const void * src, int srcsz, void * buffer, int bufsz);

struct sproto_arg {
	void *ud;
	const char *tagname;
	int m_bStar;
	int type;
	struct sproto_type *subtype;
	void *value;
	int length;
	int index;	// array base 1
	int			m_nMapKeyType;
	uint64_t*	m_pMapKeyValue;
    const char* m_pMapKeyString;
	uint64_t	m_nDefaultValue;
};

typedef int (*sproto_callback)(const struct sproto_arg *args);

int sproto_decode(const struct sproto_type *, const void * data, int size, sproto_callback cb, void *ud);
int sproto_encode(const struct sproto_type *, void * buffer, int size, sproto_callback cb, void *ud);

// for debug use
void sproto_dump(struct sproto *);
const char * sproto_name(struct sproto_type *);

#endif
