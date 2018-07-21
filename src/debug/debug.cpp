#include "../inc/basic.h"
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

__NS_BASIC_START

/////////////////////////////////////////////////////////////////////////////////////////
//
bool BasicAssertFailedLine(const char* lpszFileName, int nLine)
{
	BasicTrace("ASSERT @%s:%d\n", lpszFileName, nLine);
	return true;
}

//
void BasicDebugBreak()
{
	assert(false);
}

void BasicTraceV(const char* lpszFormat, va_list argList)
{
	char szTraceBuff[1024];
	memset(szTraceBuff, 0, sizeof(szTraceBuff));
	vsnprintf(szTraceBuff, sizeof(szTraceBuff) / sizeof(char)-1, lpszFormat, argList);

	//OutputDebugString(szTraceBuff);
	BasicTraceDebugView(szTraceBuff);
}

void BasicTrace(const char* lpszFormat, ...)
{
	va_list argList;
	va_start(argList, lpszFormat);
	BasicTraceV(lpszFormat, argList);
	va_end(argList);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
CBasicCalcUseTime::CBasicCalcUseTime(){
}
void CBasicCalcUseTime::Init(callbackCBasicCalcUseTime callback){
    m_callback = callback;
}

CBasicCalcUseTime::~CBasicCalcUseTime(){
    if(m_bStart){
        EncCalc();
    }
}

void CBasicCalcUseTime::StartCalc(){
    m_dwBegin = basiclib::BasicGetTickTime();
    m_bStart = true;
}

void CBasicCalcUseTime::EncCalc(){
    EncCalc(m_callback);
}

void CBasicCalcUseTime::EncCalc(callbackCBasicCalcUseTime callback){
    if(m_bStart){
        m_dwUseTime = basiclib::BasicGetTickTime() - m_dwBegin;
        m_dwTotalUseTime += m_dwUseTime;
        m_bStart = false;

        if(m_callback){
            m_callback(m_dwUseTime, m_dwTotalUseTime);
        }
    }
}

//手动调用回调
void CBasicCalcUseTime::CallbackLastData(){
    if(m_callback){
        m_callback(m_dwUseTime, m_dwTotalUseTime);
    }
}

void CBasicCalcUseTime::ResetData(){
    m_dwUseTime = 0;
    m_dwTotalUseTime = 0;
}


__NS_BASIC_END

