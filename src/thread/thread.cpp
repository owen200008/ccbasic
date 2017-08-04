#include "../inc/basic.h"
__NS_BASIC_START

typedef basiclib::basic_map<int, void*>	MapBasicGlobalTLSData;
class CBasicGlobalTLSData : public basiclib::CBasicObject
{
public:
	CBasicGlobalTLSData(){
		m_mTLS.CreateTLS();
		m_mTLS.SetValue(new MapBasicGlobalTLSData());
		m_pCreateFunc = nullptr;
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
public:
	CBasicThreadTLS			m_mTLS;
	pCreateKeyTLSFunc		m_pCreateFunc;
};
typedef basiclib::CBasicSingleton<CBasicGlobalTLSData> CSingletonBasicGlobalTLSData;
void* BasicGetBasiclibGlobalTLS(int nKey){
	CBasicGlobalTLSData& globalData = CSingletonBasicGlobalTLSData::Instance();
	MapBasicGlobalTLSData* pRet = (MapBasicGlobalTLSData*)globalData.GetTLSData();
	MapBasicGlobalTLSData::iterator iter = pRet->find(nKey);
	if (iter != pRet->end())
		return iter->second;
	void* pKeyData = nullptr;
	if (nKey == BasicGetBasiclibGlobalTLS_Key_CorutinePoolData) {
		pKeyData = new CCorutinePlusThreadData();
	}
	else if (globalData.m_pCreateFunc) {
		void* pKeyData = globalData.m_pCreateFunc(nKey);
	}
	(*pRet)[nKey] = pKeyData;
	return pKeyData;
}

void BasicGetBasiclibGlobalTLS_BindCreateFunc(pCreateKeyTLSFunc func) {
	ASSERT(CSingletonBasicGlobalTLSData::Instance().m_pCreateFunc == nullptr);
	CSingletonBasicGlobalTLSData::Instance().m_pCreateFunc = func;
}

__NS_BASIC_END