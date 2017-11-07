/***********************************************************************************************
// 文件名:     net_server.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2016-9-12 11:50:18
// 内容描述:   定义TCP通信的基本类
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_NET_SERVER_H
#define BASIC_NET_SERVER_H

/////////////////////////////////////////////////////////////////////////////////////////////
#include "net.h"

__NS_BASIC_START
/////////////////////////////////////////////////////////////////////////////////////////////
//声明
//class CBasicObject;
class CBasicSessionNetServerSession;
class CBasicSessionNetServer;		//监听
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma warning (push)
#pragma warning (disable: 4251)
#pragma warning (disable: 4275)

//! 请使用CreateNetServerSession
_BASIC_DLL_API CBasicSessionNetServerSession* CreateNetWithServerSession(size_t nClassSize, uint32_t nSessionID, uint16_t usRecTimeout, const std::function<CBasicSessionNetServerSession*(void*)>& func);

#define DefineCreateNetServerSession(T) \
public:\
	static T* CreateNetServerSession(uint32_t nSessionID, uint16_t usRecTimeout){return (T*)basiclib::CreateNetWithServerSession(sizeof(T), nSessionID, usRecTimeout, [&](void* pData)->CBasicSessionNetServerSession*{\
			return new (pData)T;\
		});}
#define DefineCreateNetServerSessionDefault(T) \
DefineCreateNetServerSession(T)\
protected:\
	T(){}\
	virtual ~T(){}


class CBasicNet_SocketTransfer;
class CBasicNet_SocketListen;
class _BASIC_DLL_API CBasicSessionNetServerSession : public CBasicSessionNetNotify
{
	DefineCreateNetServerSessionDefault(CBasicSessionNetServerSession)
public:
	//! 获取sessionid，等于socketid
	uint32_t GetSessionID();

	//!获取对端的地址和端口
	const char* GetNetAddress();
	uint32_t GetNetAddressPort();
protected:
	friend class CBasicNet_SocketListen;
};
typedef basiclib::CBasicRefPtr<CBasicSessionNetServerSession> CRefBasicSessionNetServerSession;
/////////////////////////////////////////////////////////////////////////////////////////////
//! //! 请使用CreateNetServer
_BASIC_DLL_API CBasicSessionNetServer* CreateNetWithServer(size_t nClassSize, const std::function<CBasicSessionNetServer*(void*)>& func);

#define DefineCreateNetServer(T) \
public:\
	static T* CreateNetServer(){return (T*)basiclib::CreateNetWithServer(sizeof(T), [&](void* pData)->CBasicSessionNetServer*{\
			return new (pData)T;\
		});}
#define DefineCreateNetServerDefault(T) \
DefineCreateNetServer(T)\
protected:\
	T(){}\
	virtual ~T(){}


typedef basiclib::basic_map<uint32_t, CRefBasicSessionNetServerSession>				MapClientSession;
typedef basiclib::basic_vector<CRefBasicSessionNetServerSession>					VTClientSession;
class _BASIC_DLL_API CBasicSessionNetServer : public CBasicSessionNet
{
	DefineCreateNetServerDefault(CBasicSessionNetServer)
public:
	//! 判断是否监听
	bool IsListen();
	
	/*formats
	-[IPv6Address]:port
	-IPv4Address:port
	*/
	virtual int32_t Listen(const char* lpszAddress, bool bWaitSuccess = false);

	//! 设置接收超时断开
	void SetClientRecTimeout(uint16_t uTimesSecond) {
		m_usRecTimeout = uTimesSecond;
	}

	//! 发送给所有客户端
	void SendToAll(void * pData, int nLength, DWORD dwFlag, bool bTrasmit = true);

	//! 断开所有连接,断开前的回调
	void CloseAllSession(const std::function<void(CBasicSessionNetServerSession* pSession)>& func);
	void CloseAll();

	//! 获取状态信息
	virtual void GetNetStatus(CBasicString& strStatus);

	//获取用户在线数
	long GetOnlineSessionCount();

	//! 获取session
	CRefBasicSessionNetServerSession GetClientBySessionID(uint32_t nSessionID);

	//! 拷贝所有客户端
	void CopyClientSession(VTClientSession& vtClient);
public:
	//! ontimer线程
	virtual void OnTimer(unsigned int nTick);
protected:
	//构造服务器对象
	virtual CBasicSessionNetServerSession* CreateServerClientSession(uint32_t nSessionID);
	virtual CBasicSessionNetServerSession* ConstructSession(uint32_t nSessionID);

	//! 接收对象
	void AcceptClient(CBasicSessionNetServerSession* pClient);
protected:
	//! 断开连接
	int32_t OnClientDisconnectCallback(CBasicSessionNetNotify* pClient, uint32_t p2);//clientdisconnectcallback
protected:
	CCriticalSection						m_mtxCSession;
	MapClientSession						m_mapClientSession;

	basiclib::CBasicString					m_strListenAddr;

	uint16_t								m_usRecTimeout;				//超时时间，0代表不超时

	friend class CBasicNet_SocketListen;
};

#pragma warning (pop)
__NS_BASIC_END
//////////////////////////////////////////////////////////////////////////////////////////////////

#endif 
