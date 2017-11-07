#include "ccbasiclib_lua.h"
namespace kaguya {
	class CCQLite3DBTable_Warp {
	public:
		CCQLite3DBTable_Warp() {
			m_bSuccess = false;
			m_bSelfTable = true;
		}
		virtual ~CCQLite3DBTable_Warp() {
			if (!m_bSelfTable) {
				m_table.ClearNoRelease();
			}
		}
	public:
		CCQLite3DBTable		m_table;
		bool				m_bSuccess;
		bool				m_bSelfTable;
	};
	template<> struct lua_type_traits<CCQLite3DBTable_Warp> {
		typedef CCQLite3DBTable_Warp get_type;
		typedef const CCQLite3DBTable_Warp& push_type;
		static int push(lua_State* l, push_type sTmp) {
			if (!sTmp.m_bSuccess) {
				lua_pushnil(l);
				return 1;
			}
			CCQLite3DBTable* s = (CCQLite3DBTable*)&(sTmp.m_table);
			int nRows = s->NumOfRows();
			int nField = s->NumOfFields();
			lua_newtable(l);
			int array_index = lua_gettop(l);
			for (int i = 0; i < nRows; i++) {
				lua_newtable(l);
				int result_index = lua_gettop(l);
				for (int j = 0; j < nField; j++) {
					s->SetRow(i);
					lua_pushstring(l, s->NameOfField(j));
					lua_pushstring(l, s->ValueOfField(j));
					lua_settable(l, result_index);
				}
				lua_settop(l, result_index);
				lua_rawseti(l, array_index, i + 1);
			}
			//需要手动clear
			s->finalizeClose();
			return 1;
		}
	};
}

string BasiclibLua_BasicGetModulePath() {
	return basiclib::BasicGetModulePath().c_str();
}
void BasiclibLua_BasicLogEvent(const char* pLog) {
	basiclib::BasicLogEvent(basiclib::DebugLevel_Info, pLog);
}
void BasiclibLua_BasicLogEventError(const char* pLog) {
	basiclib::BasicLogEventError(pLog);
}
string BasiclibLua_Basic_MD5(const string& strEncode) {
	basiclib::CBasicMD5 md5;
	md5.update((unsigned char *)strEncode.c_str(), strEncode.length());
	md5.finalize();
	return md5.hex_digest();
}
uint32_t BasiclibLua_Basic_crc32(const string& strEncode) {
	return basiclib::Basic_crc32((unsigned char*)strEncode.c_str(), strEncode.length());
}

void ExportBasiclibClassToLua(lua_State* L) {
	kaguya::State luaState(L);
	int(basiclib::CBasicString::*pReplaceFunc)(const char*, const char*) = &basiclib::CBasicString::Replace;
	BOOL(basiclib::CBasicString::*pLoadStringFunc)(const char*, const char*) = &basiclib::CBasicString::LoadString;
	void(basiclib::CBasicString::*pTrimLeftFunc)(const char*) = &basiclib::CBasicString::TrimLeft;
	void(basiclib::CBasicString::*pTrimRightFunc)(const char*) = &basiclib::CBasicString::TrimRight;
	int(basiclib::CBasicString::*pFindFunc)(const char*, int) const = &basiclib::CBasicString::Find;
	int(basiclib::CBasicString::*pReverseFindFunc)(char) const = &basiclib::CBasicString::ReverseFind;
	luaState["CBasicString"].setClass(kaguya::UserdataMetatable<basiclib::CBasicString>()
		.setConstructors<basiclib::CBasicString()>()
		.addFunction("GetLength", &basiclib::CBasicString::GetLength)
		.addFunction("IsEmpty", &basiclib::CBasicString::IsEmpty)
		.addFunction("Empty", &basiclib::CBasicString::Empty)
		.addFunction("GetAt", &basiclib::CBasicString::GetAt)
		.addFunction("SetAt", &basiclib::CBasicString::SetAt)
		.addFunction("Compare", &basiclib::CBasicString::Compare)
		.addFunction("CompareNoCase", &basiclib::CBasicString::CompareNoCase)
		.addFunction("MakeUpper", &basiclib::CBasicString::MakeUpper)
		.addFunction("MakeLower", &basiclib::CBasicString::MakeLower)
		.addFunction("MakeReverse", &basiclib::CBasicString::MakeReverse)
		.addFunction("FindOneOf", &basiclib::CBasicString::FindOneOf)
		.addFunction("GetBuffer", &basiclib::CBasicString::GetBuffer)
		.addFunction("ReleaseBuffer", &basiclib::CBasicString::ReleaseBuffer)
		.addFunction("LoadString", pLoadStringFunc)
		.addFunction("Replace", pReplaceFunc)
		.addFunction("TrimLeft", pTrimLeftFunc)
		.addFunction("TrimRight", pTrimRightFunc)
		.addFunction("Find", pFindFunc)
		.addFunction("ReverseFind", pReverseFindFunc)
		.addStaticFunction("SetString", [](basiclib::CBasicString* pStr, const std::string& str) { {
				pStr->assign(str.c_str(), str.length());
			}})
		.addStaticFunction("GetTotalString", [](basiclib::CBasicString* pStr) { {
				std::string strRet;
				strRet.assign(pStr->c_str(), pStr->length());
				return strRet;
			}})
		.addStaticFunction("Left", [](basiclib::CBasicString* pStr, int nCount) { {
				basiclib::CBasicString strRet = pStr->Left(nCount);
				return std::string(strRet.c_str(), strRet.length());
			}})
		.addStaticFunction("Right", [](basiclib::CBasicString* pStr, int nCount) { {
				basiclib::CBasicString strRet = pStr->Right(nCount);
				return std::string(strRet.c_str(), strRet.length());
			}})
		.addStaticFunction("Mid", [](basiclib::CBasicString* pStr, int nFirst, int nCount) { {
				basiclib::CBasicString strRet = pStr->Mid(nFirst, nCount);
				return std::string(strRet.c_str(), strRet.length());
			}})
		);
	luaState["CBasicBitstream"].setClass(kaguya::UserdataMetatable<basiclib::CBasicBitstream>()
		.setConstructors<basiclib::CBasicBitstream()>()
		.addStaticFunction("ResetReadError", [](basiclib::CBasicBitstream* pSM){{
            pSM->ResetReadError();
        }})
		.addStaticFunction("IsReadError", [](basiclib::CBasicBitstream* pSM){{
            return pSM->IsReadError();
        }})
		.addStaticFunction("IsEmpty", [](basiclib::CBasicBitstream* pSM){{
            return pSM->IsEmpty();
        }})
		.addStaticFunction("Free", [](basiclib::CBasicBitstream* pSM){{
            pSM->Free();
        }})
		.addStaticFunction("GetDataLength", [](basiclib::CBasicBitstream* pSM){{
            return pSM->GetDataLength();
        }})
		.addStaticFunction("SetDataLength", [](basiclib::CBasicBitstream* pSM, long nLength){{
            pSM->SetDataLength(nLength);
        }})
		.addStaticFunction("AppendString", [](basiclib::CBasicBitstream* pSM, const char* lpszText){{
            pSM->AppendString(lpszText);
        }})
		.addStaticFunction("GetAllocBufferLength", [](basiclib::CBasicBitstream* pSM){{
            return pSM->GetAllocBufferLength();
        }})
		.addStaticFunction("InitFormFile", [](basiclib::CBasicBitstream* pSM, const char* lpszFile){{
            return pSM->InitFormFile(lpszFile);
        }})
		.addStaticFunction("GetDataBuffer", [](basiclib::CBasicBitstream* pSM) { {
				std::string strRet;
				if (pSM->GetDataBuffer()) {
					strRet.assign(pSM->GetDataBuffer(), pSM->GetDataLength());
				}
				return strRet;
			}})
		.addStaticFunction("AppendData", [](basiclib::CBasicBitstream* pSM, const std::string& str) { {
				pSM->AppendData(str.c_str(), str.length());
			}})
		.addStaticFunction("AppendDataEx", [](basiclib::CBasicBitstream* pSM, const std::string& str) { {
				pSM->AppendDataEx(str.c_str(), str.length());
			}})
		.addStaticFunction("AppendSMBuffer", [](basiclib::CBasicBitstream* pSM, const basiclib::CBasicBitstream* pSM2)->void { {
				pSM->AppendData(pSM2->GetDataBuffer(), pSM2->GetDataLength());
			}})
		.addStaticFunction("Compare", [](basiclib::CBasicBitstream* pSM, const basiclib::CBasicBitstream* pSM2)->bool { {
				if (pSM->GetDataLength() != pSM2->GetDataLength())
					return false;
				return memcmp(pSM->GetDataBuffer(), pSM2->GetDataBuffer(), pSM->GetDataLength()) == 0;
			}})
		);
	luaState["CNetBasicValue"].setClass(kaguya::UserdataMetatable<basiclib::CNetBasicValue>()
	.setConstructors<basiclib::CNetBasicValue()>()
	.addFunction("SetLong", &basiclib::CNetBasicValue::SetLong)
	.addFunction("SetDouble", &basiclib::CNetBasicValue::SetDouble)
	.addFunction("SetLongLong", &basiclib::CNetBasicValue::SetLongLong)
	.addFunction("GetLong", &basiclib::CNetBasicValue::GetLong)
	.addFunction("GetDouble", &basiclib::CNetBasicValue::GetDouble)
	.addFunction("GetLongLong", &basiclib::CNetBasicValue::GetLongLong)
	.addFunction("GetStringRef", &basiclib::CNetBasicValue::GetStringRef)
	.addFunction("GetDataType", &basiclib::CNetBasicValue::GetDataType)
	.addFunction("GetDataLength", &basiclib::CNetBasicValue::GetDataLength)
	.addFunction("IsNull", &basiclib::CNetBasicValue::IsNull)
	.addFunction("SetNull", &basiclib::CNetBasicValue::SetNull)
	.addFunction("IsString", &basiclib::CNetBasicValue::IsString)
	.addFunction("CompareBasicValue", &basiclib::CNetBasicValue::CompareBasicValue)
	.addFunction("CompareInt", &basiclib::CNetBasicValue::CompareInt)
	.addFunction("CompareDouble", &basiclib::CNetBasicValue::CompareDouble)
	.addFunction("CompareLongLong", &basiclib::CNetBasicValue::CompareLongLong)
	.addFunction("GetSeriazeLength", &basiclib::CNetBasicValue::GetSeriazeLength)
	.addStaticFunction("ComparePointString", [](basiclib::CNetBasicValue* pData, const std::string& str) { {
	return pData->ComparePointString(str.c_str(), str.length());
	}})
	.addStaticFunction("SeriazeSMBuf", [](basiclib::CNetBasicValue* pData, basiclib::CBasicSmartBuffer* pBuf) { {
	pData->SeriazeSMBuf(*pBuf);
	}})
	.addStaticFunction("UnSeriazeSMBuf", [](basiclib::CNetBasicValue* pData, basiclib::CBasicSmartBuffer* pBuf) { {
	pData->UnSeriazeSMBuf(*pBuf);
	}})
	.addStaticFunction("SetString", [](basiclib::CNetBasicValue* pData, const std::string& str) { {
	pData->SetString(str.c_str(), str.length());
	}})
	);
	luaState["CCQLite3DB"].setClass(kaguya::UserdataMetatable<CCQLite3DB>()
	.setConstructors<CCQLite3DB()>()
	.addFunction("Open", &CCQLite3DB::Open)
	.addFunction("Close", &CCQLite3DB::Close)
	.addFunction("SetOpenPWD", &CCQLite3DB::SetOpenPWD)
	.addFunction("SetPWD", &CCQLite3DB::SetPWD)
	.addFunction("GetDataToTable", &CCQLite3DB::GetDataToTable)
	.addFunction("ExecSQL", &CCQLite3DB::ExecSQL)
	.addFunction("GetLastError", &CCQLite3DB::GetLastError)
	.addStaticFunction("QueryTotalData", [](CCQLite3DB* pDB, const char* pSQL)->kaguya::CCQLite3DBTable_Warp { {
			kaguya::CCQLite3DBTable_Warp ret;
			if (!pDB->GetDataToTable(pSQL, &ret.m_table)) {
				return ret;
			}
			ret.m_bSuccess = true;
			ret.m_bSelfTable = false;
			return ret;
		}})
	);
	bool(CCQLite3DBTable::*pFieldIsNullFunc)(const char*) = &CCQLite3DBTable::FieldIsNull;
	const char*(CCQLite3DBTable::*pValueOfFieldFunc)(const char*) = &CCQLite3DBTable::ValueOfField;
	luaState["CCQLite3DBTable"].setClass(kaguya::UserdataMetatable<CCQLite3DBTable>()
	.setConstructors<CCQLite3DBTable()>()
	.addFunction("NumOfFields", &CCQLite3DBTable::NumOfFields)
	.addFunction("NumOfRows", &CCQLite3DBTable::NumOfRows)
	.addFunction("NameOfField", &CCQLite3DBTable::NameOfField)
	.addFunction("ValueOfField", pValueOfFieldFunc)
	.addFunction("FieldIsNull", pFieldIsNullFunc)
	.addFunction("GetUIntField", &CCQLite3DBTable::GetUIntField)
	.addFunction("GetIntField", &CCQLite3DBTable::GetIntField)
	.addFunction("GetDoubleField", &CCQLite3DBTable::GetDoubleField)
	.addFunction("GetStringField", &CCQLite3DBTable::GetStringField)
	);

	luaState["CPBZK"].setClass(kaguya::UserdataMetatable<CPBZK>()
		.setConstructors<CPBZK()>()
		.addFunction("ReadPBZKFileBuffer", &CPBZK::ReadPBZKFileBuffer)
		.addFunction("IsContainPBZK", &CPBZK::IsContainPBZK)
		 .addStaticFunction("ReadPBZK", [](CPBZK* p, const string& strData){
			p->ReadPBZKFileBuffer(strData.c_str(), strData.length());
		})
		.addStaticFunction("ReplacePBZK", [](CPBZK* pPBZK, const string& str, const char* pReplate, bool bDeep, bool bCheckSpecialZF) { {
				pPBZK->ReplacePBZK((char*)str.c_str(), str.length(), pReplate[0], bDeep, bCheckSpecialZF);
				return str;
			}})
	);

	//全局函数
	
	luaState["BasicGetModulePath"] = &BasiclibLua_BasicGetModulePath;
	luaState["BasicLogEvent"] = &BasiclibLua_BasicLogEvent;
	luaState["BasicLogEventError"] = &BasiclibLua_BasicLogEventError;
	luaState["Basic_MD5"] = &BasiclibLua_Basic_MD5;
	luaState["Basic_crc32"] = &BasiclibLua_Basic_crc32;
}