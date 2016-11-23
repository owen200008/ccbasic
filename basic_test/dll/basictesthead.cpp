#include "stdafx.h"
#include "basictesthead.h"

class CTestHeadMgr : public basiclib::CBasicObject
{
public:
	CTestHeadMgr()
	{
		
	}
	virtual ~CTestHeadMgr()
	{
	}
	void Register(CTestHead* pHead)
	{
		basiclib::BasicLogEvent("CTestHeadMgr::Register()");
		m_pHead = pHead;
	}
	void Release()
	{
		basiclib::BasicLogEvent("CTestHeadMgr::Release()");
		m_pHead = nullptr;
	}
	CTestHead* GetHead(){ return m_pHead; }
protected:
	CTestHead* m_pHead;
};

CTestHeadMgr m_mgr;

CTestHead::CTestHead()
{
	m_mgr.Register(this);
}

CTestHead::~CTestHead()
{
	m_mgr.Release();
}

void CTestHead::CallFunc()
{
	basiclib::BasicLogEvent("CTestHead::CallFunc");
}

CTestHead* GetTestHead()
{
	return m_mgr.GetHead();
}