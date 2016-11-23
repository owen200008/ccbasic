#ifndef INC_BASKCTESTHEAD_H
#define INC_BASKCTESTHEAD_H

#include <basic.h>
class CTestHead : public basiclib::CBasicObject
{
public:
	CTestHead();
	virtual ~CTestHead();

	virtual void CallFunc();
};

__declspec(dllexport) CTestHead* GetTestHead();

#endif