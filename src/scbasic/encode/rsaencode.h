#ifndef SCBASIC_RSA_H
#define SCBASIC_RSA_H

#include <basic.h>
#include "../../../3rd/cryptopp/randpool.h"
#include "../../../3rd/cryptopp/rsa.h"

#define DEFAULT_SCBASICRSA_SEED	"seed_scbasic"
class CSCBasicRSA : public basiclib::CBasicObject
{
public:
	CSCBasicRSA(const char* pSeed = DEFAULT_SCBASICRSA_SEED);
	virtual ~CSCBasicRSA();

	void GenerateRSAKey(unsigned int keyLength, const char *privFilename, const char *pubFilename);
	void GenerateRSAKey(unsigned int keyLength, string& strPub, string& strPri);

	//! 设置public&private
	void SetPublicFileName(const char* pPubFileName);
	void SetPublicKey(const char* pData);
	void SetPrivateFileName(const char* pPrivateFileName);
	void SetPrivateKey(const char* pData);

	//! 计算加密所需的数据长度
	size_t CalcEncryptNeedLength(int nLength);

	size_t Encrypt(const char* pEncode, int nLength, byte* pOutput, int nOutputLength);
	size_t Decrypt(const char* pDecode, int nLength, byte* pOutput, int nOutputLength);

	//! 最大不能超过max signlength, 0代表失败
	size_t Sign(const char* pEncode, int nLength, byte* pOutput, int nOutputLength);
	bool Verify(const char* pDecode, int nLength, const char* pVerify, int nVerifyLength);

	CryptoPP::RandomPool& GetRandomPool() { return m_rng; }
protected:
	CryptoPP::RandomPool					m_rng;
	CryptoPP::RSAES_OAEP_SHA_Encryptor		m_pubEncode;
	CryptoPP::RSASSA_PKCS1v15_SHA_Verifier	m_pubDecode;
	CryptoPP::RSAES_OAEP_SHA_Decryptor		m_priDecode;
	CryptoPP::RSASSA_PKCS1v15_SHA_Signer	m_priEncode;
};

struct RSAInitData {
	Net_UShort					m_usVersion;
	basiclib::CBasicSmartBuffer m_smBufPub;
	basiclib::CBasicSmartBuffer m_smBufPri;
};
typedef basiclib::basic_vector<RSAInitData>	VTRSAInitData;
class CXKRSAManager : public basiclib::CBasicObject
{
public:
	CXKRSAManager();
	virtual ~CXKRSAManager();

	bool InitFromPath(const char* pPath);
	bool InitFromPath(const char* pPath, const std::function<void(const char* pFileName, basiclib::CBasicSmartBuffer& smBuf)>& func);
	bool InitFromData(Net_UShort usDefaultVersion, VTRSAInitData& vtData);
	CSCBasicRSA* GetRSAByVersion(Net_UShort sVersion);
	Net_UShort GetRSADefaultVersion() { return m_usDefaultVersion; }
	CSCBasicRSA* GetDefaultRSA() { return m_pDefaultRSA; }
protected:
	typedef basiclib::basic_map<Net_UShort, CSCBasicRSA*>   MapVersionToRSA;
	typedef MapVersionToRSA::iterator                       MapVersionToRSAIterator;
	MapVersionToRSA                                         m_mapRSA;
	Net_UShort                                              m_usDefaultVersion;
	CSCBasicRSA*                                            m_pDefaultRSA;
};

#endif
