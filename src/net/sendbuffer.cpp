#include "../inc/basic.h"
#include "sendbuffer.h"
#include "net.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_START

//////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
CMsgSendBuffer::CMsgSendBuffer(){
	SetDataLength(1024);
	SetDataLength(0);
}
CMsgSendBuffer::~CMsgSendBuffer(){

}

int32_t CMsgSendBuffer::SendBuffer(int32_t lSend){
	if(m_cbBuffer < lSend){
		ASSERT(FALSE);
		m_cbBuffer = 0;
		return 0;
	}
	m_cbBuffer -= lSend;
	if(m_cbBuffer > 0){
		memmove(m_pszBuffer, m_pszBuffer + lSend, m_cbBuffer);
	}
	return m_cbBuffer;
}

__NS_BASIC_END
