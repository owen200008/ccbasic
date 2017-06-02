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

#ifdef sproto_EXPORTS
#define __EXPORTSSPROTO
#endif
#ifdef __BASICWINDOWS
#ifdef __EXPORTSSPROTO
#define _SCBASIC_SPROTO_DLL_API 	__declspec(dllexport)
#endif
#ifdef __IMPORTSSPROTO
#define _SCBASIC_SPROTO_DLL_API 	__declspec(dllimport)
#endif
#endif

#ifndef _SCBASIC_SPROTO_DLL_API
#define _SCBASIC_SPROTO_DLL_API
#endif

extern "C" int _SCBASIC_SPROTO_DLL_API luaopen_sproto_core(lua_State *L);

#endif // __cocos2dx_xmqxz_lsproto_h__



