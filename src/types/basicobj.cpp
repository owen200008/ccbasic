#include "../inc/basic.h"

__NS_BASIC_START
/////////////////////////////////////////////////////////////////////////////////////////////
CBasicObject::CBasicObject(){
}

CBasicObject::~CBasicObject(){
}

////////////////////////////////////////////////////////////////////////
void* CBasicObject::operator new(size_t nSize){
	return BasicAllocate(nSize);
}
void* CBasicObject::operator new(size_t nSize, void* pPoint) {
	return pPoint;
}

void CBasicObject::operator delete(void* p){
	BasicDeallocate(p);
}
void CBasicObject::operator delete(void* p, void* pPont) {
	//do nothing
	ASSERT(0);
}

__NS_BASIC_END
