#include "../inc/basic.h"
#include <string.h>
#include <stdint.h>

#define ESP 0
#define EIP 1
// -----------
#define RSP 0
#define RIP 1
#define RBX 2
#define RDI 3
#define RSI 4

#ifdef __BASICWINDOWS
extern "C"
{
	extern void coctx_swap(coctx_t *, coctx_t*);
}
void coctx_make(coctx_t *ctx, coctx_pfn_t pfn, const void* s1)
{
	int *sp = (int*)((uintptr_t)ctx->ss_sp + ctx->ss_size);
	sp -= 1;
	sp = (int*)((unsigned long)sp & -16L);
	memset((char*)sp - 64, 0, 64);
	sp[0] = (int)s1;
	sp -= 1;
	sp[0] = 0;

	ctx->regs[ESP] = (char*)sp;
	ctx->regs[EIP] = (char*)pfn;
}
#else
extern "C"
{
	extern void coctx_swap( coctx_t *,coctx_t* ) asm("coctx_swap");
}
#endif

void coctx_init(coctx_t *ctx)
{
	memset(ctx,0,sizeof(*ctx));
}
#if defined(__i386__)
void coctx_make(coctx_t *ctx,coctx_pfn_t pfn, const void* s1)
{
	int *sp = (int*)((uintptr_t)ctx->ss_sp + ctx->ss_size);
	sp -= 1;
	sp = (char*)((unsigned long)sp & -16L);
	memset(sp - 64, 0, 64);
	sp[0] = (int)s1;
	sp -= 1;
	sp[0] = 0;

	ctx->regs[ ESP ] = (char*)sp;
	ctx->regs[ EIP ] = (char*)pfn;
}
#elif defined(__x86_64__)
void coctx_make(coctx_t *ctx, coctx_pfn_t pfn, const void *s1)
{
	char *stack = ctx->ss_sp;

	long long int *sp = (long long int*)((uintptr_t)stack + ctx->ss_size);
	sp -= 1;
	sp = (long long int*)((((uintptr_t)sp) & - 16L ) - 8);
	memset(sp - 128, 0 , 128);

	ctx->regs[ RBX ] = &sp[1];
	ctx->regs[ RSP ] = (char*)sp;
	ctx->regs[ RIP ] = (char*)pfn;
	
	sp[0] = 0;
	sp[1] = 0;
	
	ctx->regs[ RDI ] = (char*)s1;
}
#endif
void coctx_swapcontext(coctx_t *ctx1, coctx_t *ctx2)
{
	coctx_swap(ctx1, ctx2);
}

