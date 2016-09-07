#include "../inc/basic.h"

__NS_BASIC_START
/////////////////////////////////////////////////////////////////////////////////////////////
CBasicObject::CBasicObject()
{
}

CBasicObject::~CBasicObject()
{
}

////////////////////////////////////////////////////////////////////////
void* CBasicObject::operator new(size_t nSize)
{
	return BasicAllocate(nSize);
}

void CBasicObject::operator delete(void* p)
{
	BasicDeallocate(p);
}

__NS_BASIC_END
