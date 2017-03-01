#include "../inc/basic.h"
__NS_BASIC_START

typedef basiclib::basic_map<int, void*>	MapBasicGlobalTLSData;
class CBasicGlobalTLSData : public basiclib::CBasicObject
{
public:
	CBasicGlobalTLSData(){
		m_mTLS.CreateTLS();
		m_mTLS.SetValue(new MapBasicGlobalTLSData());
	}
	virtual ~CBasicGlobalTLSData(){

	}
	MapBasicGlobalTLSData* GetTLSData(){
		MapBasicGlobalTLSData* pRet = (MapBasicGlobalTLSData*)m_mTLS.GetValue();
		if (pRet)
			return pRet;
		pRet = new MapBasicGlobalTLSData();
		m_mTLS.SetValue(pRet);
		return pRet;
	}
protected:
	CBasicThreadTLS m_mTLS;
};
typedef basiclib::CBasicSingleton<CBasicGlobalTLSData> CSingletonBasicGlobalTLSData;
void* BasicGetBasiclibGlobalTLS(int nKey){
	MapBasicGlobalTLSData* pRet = (MapBasicGlobalTLSData*)CSingletonBasicGlobalTLSData::Instance().GetTLSData();
	if (pRet){
		MapBasicGlobalTLSData::iterator iter = pRet->find(nKey);
		if (iter != pRet->end())
			return iter->second;
	}
	return nullptr;
}
void BasicSetBasiclibGlobalTLS(int nKey, void* pData){
	MapBasicGlobalTLSData* pRet = (MapBasicGlobalTLSData*)CSingletonBasicGlobalTLSData::Instance().GetTLSData();
	(*pRet)[nKey] = pData;
}

__NS_BASIC_END