#ifndef __cocos2dx_xmqxz_lsproto_h__
#define __cocos2dx_xmqxz_lsproto_h__

#ifdef __cplusplus
extern "C" {
#endif
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#ifdef __cplusplus
}
#endif
#include <basic.h>
#include "../scbasic_head.h"

_SCBASIC_EXTERNC int _SCBASIC_DLL_API luaopen_sproto_core(lua_State *L);
_SCBASIC_EXTERNC int _SCBASIC_DLL_API SprotoDecodeFunc(lua_State *L, basiclib::CBasicBitstream* pSMBuf, struct sproto_type* st);

#endif // __cocos2dx_xmqxz_lsproto_h__



