#ifndef __CO_ROUTINE_H__
#define __CO_ROUTINE_H__

#include <stdlib.h>
typedef void (*coctx_pfn_t)(const char* s);
struct coctx_t
{
	void *regs[ 5 ];

	size_t ss_size;
	char *ss_sp;
};

void coctx_init(coctx_t *ctx);
void coctx_make(coctx_t *ctx, coctx_pfn_t pfn, const void* s);
void coctx_swapcontext(coctx_t *ctx1, coctx_t *ctx2);

#endif


