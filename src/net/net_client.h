/***********************************************************************************************
// 文件名:     net_client.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2016-9-12 11:50:18
// 内容描述:   定义TCP通信的基本类
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_NET_CLIENT_H
#define BASIC_NET_CLIENT_H

/////////////////////////////////////////////////////////////////////////////////////////////
#include "net.h"

__NS_BASIC_START
/////////////////////////////////////////////////////////////////////////////////////////////
//声明
class CBasicSessionNetClient;		//主动连接
//////////////////////////////////////////////////////////////////////////////
#pragma warning (push)
#pragma warning (disable: 4251)
#pragma warning (disable: 4275)

//! 请调用CreateNetClient
_BASIC_DLL_API CBasicSessionNetClient* CreateNetWithClient(size_t nClassSize, uint16_t usRecTimeout, const std::function<CBasicSessionNetClient*(void*)>& func);

#define DefineCreateNetClient(T) \
public:\
	static T* CreateNetClient(uint16_t usRecTimeout = 0){return (T*)basiclib::CreateNetWithClient(sizeof(T), usRecTimeout, [&](void* pData)->CBasicSessionNetClient*{\
			return new (pData)T;\
		});}
#define DefineCreateNetClientDefault(T) \
DefineCreateNetClient(T)\
protected:\
	T(){}\
	virtual ~T(){}

class _BASIC_DLL_API CBasicSessionNetClient : public CBasicSessionNetNotify
{
	DefineCreateNetClientDefault(CBasicSessionNetClient);
public:
	//! 连接 formats [IPv6Address]:port || IPv4Address:port
	virtual int32_t Connect(const char* lpszAddress);

	//! 重连
	int32_t DoConnect();

	//! 获取连接地址
	basiclib::CBasicString& GetConnectAddr();
};
#pragma warning (pop)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__NS_BASIC_END
//////////////////////////////////////////////////////////////////////////////////////////////////

#endif 
