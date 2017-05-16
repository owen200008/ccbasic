#ifndef SCBASIC_HEAD_H
#define SCBASIC_HEAD_H

#ifdef __BASICWINDOWS
#ifdef __EXPORTSCBASIC
#define _SCBASIC_DLL_API 	__declspec(dllexport)
#endif
#ifdef __IMPORTSCBASIC
#define _SCBASIC_DLL_API 	__declspec(dllimport)
#endif
#endif

#ifndef _SCBASIC_DLL_API
#define _SCBASIC_DLL_API
#endif

#define _SCBASIC_EXTERNC		extern "C"

#define GlobalGetClassName(s) #s

#endif