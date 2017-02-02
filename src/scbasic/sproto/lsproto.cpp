#include <string.h>
#include <stdlib.h>
#include "msvcint.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "sproto.h"
#include <basic.h>

#define MAX_GLOBALSPROTO 16
#define ENCODE_BUFFERSIZE 2050

#define ENCODE_MAXSIZE 0x1000000
#define ENCODE_DEEPLEVEL 64

#ifndef luaL_newlib /* using LuaJIT */
/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup) {
#ifdef luaL_checkversion
	luaL_checkversion(L);
#endif
	luaL_checkstack(L, nup, "too many upvalues");
	for (; l->name != NULL; l++) {  /* fill the table with given functions */
		int i;
		for (i = 0; i < nup; i++)  /* copy upvalues to the top */
			lua_pushvalue(L, -nup);
		lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
		lua_setfield(L, -(nup + 2), l->name);
	}
	lua_pop(L, nup);  /* remove upvalues */
}

#define luaL_newlibtable(L,l) \
  lua_createtable(L, 0, sizeof(l)/sizeof((l)[0]) - 1)

#define luaL_newlib(L,l)  (luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))
#endif

#if LUA_VERSION_NUM < 503

#if LUA_VERSION_NUM < 502
static lua_Integer lua_tointegerx(lua_State *L, int idx, int *isnum) {
	if (lua_isnumber(L, idx)) {
		if (isnum) *isnum = 1;
		return lua_tointeger(L, idx);
	}
	else {
		if (isnum) *isnum = 0;
		return 0;
	}
}
#endif

// work around , use push & lua_gettable may be better
#define lua_geti lua_rawgeti
#define lua_seti lua_rawseti

#endif

static int
lnewproto(lua_State *L) {
	struct sproto * sp;
	size_t sz;
	void * buffer = (void *)luaL_checklstring(L,1,&sz);
	sp = sproto_create(buffer, sz);
	if (sp) {
		lua_pushlightuserdata(L, sp);
		return 1;
	}
	return 0;
}

static int
ldeleteproto(lua_State *L) {
	struct sproto * sp = (struct sproto *)lua_touserdata(L, 1);
	if (sp == NULL) {
		return luaL_argerror(L, 1, "Need a sproto object");
	}
	sproto_release(sp);
	return 0;
}

static int
lquerytype(lua_State *L) {
	const char * type_name;
	struct sproto *sp = (struct sproto *)lua_touserdata(L, 1);
	struct sproto_type *st;
	if (sp == NULL) {
		return luaL_argerror(L, 1, "Need a sproto object");
	}
	type_name = luaL_checkstring(L,2);
	st = sproto_type(sp, type_name);
	if (st) {
		lua_pushlightuserdata(L, st);
		return 1;
	}
	return 0;
}

struct encode_ud {
	lua_State *L;
	struct sproto_type *st;
	int tbl_index;
	const char * array_tag;
	int array_index;
	int deep;
	int iter_index;
};


static int
encode_xmdefault(const struct sproto_arg *args)
{
	struct encode_ud *self = (struct encode_ud *)args->ud;
	lua_State *L = self->L;
	if (self->deep >= ENCODE_DEEPLEVEL)
		return luaL_error(L, "The table is too deep");
	if (args->index > 0)
	{
		return SPROTO_CB_NIL;
	}
	else
	{
		switch (args->type)
		{
		case SPROTO_CC_CHAR:
		case SPROTO_CC_UCHAR:
		{
			*(unsigned char *)args->value = args->m_nDefaultValue;
			return SPROTO_CC_CHAR_SIZE;
		}
		case SPROTO_CC_SHORT:
		case SPROTO_CC_USHORT:
		{
			*(unsigned short *)args->value = args->m_nDefaultValue;
			return SPROTO_CC_SHORT_SIZE;
		}
		case SPROTO_CC_INT:
		case SPROTO_CC_UINT:
		{
			*(uint32_t *)args->value = args->m_nDefaultValue;
			return SPROTO_CC_INT_SIZE;
		}
		case SPROTO_CC_LONGLONG:
		{
			*(uint64_t *)args->value = args->m_nDefaultValue;
			return SPROTO_CC_LONGLONG_SIZE;
		}
		case SPROTO_CC_DOUBLE:
		{
			*(double *)args->value = args->m_nDefaultValue;
			return SPROTO_CC_DOUBLE_SIZE;
		}
		case SPROTO_CC_STRING:
		{
			return 0;
		}
		case SPROTO_CC_STRUCT:
		{
			struct encode_ud sub;
			int r;
			sub.L = L;
			sub.st = args->subtype;
			sub.array_tag = NULL;
			sub.array_index = 0;
			sub.deep = self->deep + 1;
			r = sproto_encode(args->subtype, args->value, args->length, encode_xmdefault, &sub);
			if (r < 0)
				return SPROTO_CB_ERROR;
			return r;
		}
		case SPROTO_CC_EXA_CNETBASICVALUE:
		{
			char szBuf[8] = { 0 };
			basiclib::CNetBasicValue basicValue;
			int nRet = basicValue.Seriaze(szBuf, 8);
			assert(nRet == 1);
			memcpy(args->value, szBuf, nRet);
			return nRet;
		}
			break;
		default:
		{
			luaL_error(L, "tagname not find %s ", args->tagname);
			assert(0);
			lua_pop(L, 1);
			return SPROTO_CB_NIL;
		}
		}
	}
}

static int
encode(const struct sproto_arg *args) 
{
	struct encode_ud *self = (encode_ud *)args->ud;
	lua_State *L = self->L;
	if (self->deep >= ENCODE_DEEPLEVEL)
		return luaL_error(L, "The table is too deep");
	if (args->index > 0) 
	{
		if (args->tagname != self->array_tag) 
		{
			// a new array
			self->array_tag = args->tagname;
			lua_getfield(L, self->tbl_index, args->tagname);
			if (lua_isnil(L, -1)) 
			{
				//判断是否是star
				if (args->m_bStar)
					return 0;

				if (self->array_index) 
				{
					lua_replace(L, self->array_index);
				}
				self->array_index = 0;
				return SPROTO_CB_NOARRAY;
			}
			if (!lua_istable(L, -1)) 
			{
				return luaL_error(L, ".*%s(%d) should be a table (Is a %s)",
					args->tagname, args->index, lua_typename(L, lua_type(L, -1)));
			}
			if (self->array_index) 
			{
				lua_replace(L, self->array_index);
			} 
			else 
			{
				self->array_index = lua_gettop(L);
			}
		}
		if (args->m_nMapKeyType > 0)
		{
			// use lua_next to iterate the table
			// todo: check the key is equal to mainindex value
			lua_pushvalue(L,self->iter_index);
			if (!lua_next(L, self->array_index)) 
			{
				// iterate end
				lua_pushnil(L);
				lua_replace(L, self->iter_index);
				return SPROTO_CB_NIL;
			}
			lua_insert(L, -2);
			lua_replace(L, self->iter_index);

			//压入key
			lua_pushvalue(L, self->iter_index);
			switch (args->m_nMapKeyType)
			{
			case SPROTO_CC_CHAR:
			case SPROTO_CC_UCHAR:
			{
				int v = lua_tointeger(L, -1);
				if (!lua_isnumber(L, -1))
				{
					return luaL_error(L, ".%s[%d] is not a char (Is a %s)",
						args->tagname, args->index, lua_typename(L, lua_type(L, -1)));
				}
				*args->m_pMapKeyValue = v;
			}
			case SPROTO_CC_SHORT:
			case SPROTO_CC_USHORT:
			{
				int v = lua_tointeger(L, -1);
				if (!lua_isnumber(L, -1))
				{
					return luaL_error(L, ".%s[%d] is not a char (Is a %s)",
						args->tagname, args->index, lua_typename(L, lua_type(L, -1)));
				}
				*args->m_pMapKeyValue = v;
			}
			break;
			case SPROTO_CC_INT:
			case SPROTO_CC_UINT:
			{
				int v = lua_tointeger(L, -1);
				if (!lua_isnumber(L, -1))
				{
					return luaL_error(L, ".%s[%d] is not a char (Is a %s)",
						args->tagname, args->index, lua_typename(L, lua_type(L, -1)));
				}
				*args->m_pMapKeyValue = v;
			}
			break;
			case SPROTO_CC_LONGLONG:
			{
				if (!lua_isnumber(L, -1))
				{
					return luaL_error(L, ".%s[%d] is not an integer (Is a %s)",
						args->tagname, args->index, lua_typename(L, lua_type(L, -1)));
				}
				int64_t v = lua_tonumber(L, -1);
				*args->m_pMapKeyValue = v;
			}
			break;
			case SPROTO_CC_DOUBLE:
			{
				if (!lua_isnumber(L, -1))
				{
					return luaL_error(L, ".%s[%d] is not an integer (Is a %s)",
						args->tagname, args->index, lua_typename(L, lua_type(L, -1)));
				}
				double dValue = lua_tonumber(L, -1);
				int64_t v = *(int64_t*)&dValue;
				*args->m_pMapKeyValue = v;
			}
			break;
			default:
				assert(0);
				return luaL_error(L, "key not support type %d", args->m_nMapKeyType);
			}
			lua_pop(L, 1);
		} 
		else
		{
			lua_geti(L, self->array_index, args->index);
		}
		if (lua_isnil(L, -1))
		{
			lua_pop(L, 1);
			return SPROTO_CB_NIL;
		}
	} 
	else 
	{
		lua_getfield(L, self->tbl_index, args->tagname);
		if (lua_isnil(L, -1))
		{
			lua_pop(L, 1);
			//判断是否是star
			if (args->m_bStar)
				return 0;
			return encode_xmdefault(args);
		}
	}

	switch (args->type) 
	{
	case SPROTO_CC_CHAR:
	case SPROTO_CC_UCHAR:
	{
		bool bNumber = lua_isnumber(L, -1);
		int v = 0;
		if (bNumber)
			v = lua_tointeger(L, -1);
		else
		{
			if (!lua_isboolean(L, -1))
			{
				return luaL_error(L, ".%s[%d] is not a char (Is a %s)",
					args->tagname, args->index, lua_typename(L, lua_type(L, -1)));
			}
			v = lua_toboolean(L, -1);
		}
		*(unsigned char *)args->value = v;
		lua_pop(L, 1);
		return SPROTO_CC_CHAR_SIZE;
	}
	case SPROTO_CC_SHORT:
	case SPROTO_CC_USHORT:
	{
		int v = lua_tointeger(L, -1);
		if (!lua_isnumber(L, -1)) {
			return luaL_error(L, ".%s[%d] is not a char (Is a %s)",
				args->tagname, args->index, lua_typename(L, lua_type(L, -1)));
		}
		*(unsigned short *)args->value = v;
		lua_pop(L, 1);
		return SPROTO_CC_SHORT_SIZE;
	}
	case SPROTO_CC_INT:
	case SPROTO_CC_UINT:
	{
		int v = lua_tointeger(L, -1);
		if (!lua_isnumber(L, -1)) {
			return luaL_error(L, ".%s[%d] is not a char (Is a %s)",
				args->tagname, args->index, lua_typename(L, lua_type(L, -1)));
		}
		*(uint32_t *)args->value = v;
		lua_pop(L, 1);
		return SPROTO_CC_INT_SIZE;
	}
	case SPROTO_CC_LONGLONG:
	{
		if (!lua_isnumber(L, -1)) {
			return luaL_error(L, ".%s[%d] is not an integer (Is a %s)",
				args->tagname, args->index, lua_typename(L, lua_type(L, -1)));
		}
		int64_t v = lua_tonumber(L, -1);
		*(uint64_t *)args->value = v;
		lua_pop(L, 1);
		return SPROTO_CC_LONGLONG_SIZE;
	}
	case SPROTO_CC_DOUBLE:
	{
		if (!lua_isnumber(L, -1)) {
			return luaL_error(L, ".%s[%d] is not an integer (Is a %s)",
				args->tagname, args->index, lua_typename(L, lua_type(L, -1)));
		}
		double v = lua_tonumber(L, -1);
		*(double *)args->value = v;
		lua_pop(L, 1);
		return SPROTO_CC_DOUBLE_SIZE;
	}
	case SPROTO_CC_STRING:
	{
		size_t sz = 0;
		const char * str;
		if (!lua_isstring(L, -1)) 
		{
			return luaL_error(L, ".%s[%d] is not a string (Is a %s)",
				args->tagname, args->index, lua_typename(L, lua_type(L, -1)));
		}
		else 
		{
			str = lua_tolstring(L, -1, &sz);
		}
		if (sz > args->length)
			return SPROTO_CB_ERROR;
		memcpy(args->value, str, sz);
		lua_pop(L, 1);
		return sz;
	}
	case SPROTO_CC_EXA_CNETBASICVALUE:
	{
		size_t sz = 0;
		if (lua_isuserdata(L, -1))
		{
			basiclib::CNetBasicValue* pValue = (basiclib::CNetBasicValue*)*((void**)lua_touserdata(L, 1));
			sz = pValue->GetSeriazeLength();
			if (sz > args->length)
				return SPROTO_CB_ERROR;
			pValue->Seriaze((char*)args->value, args->length);
		}
		else
		{
			basiclib::CNetBasicValue value;
			if (lua_isnil(L, -1))
			{
				//不做任何事情
			}
			else
			{
				int nType = lua_type(L, -1);
				switch (nType)
				{
				case LUA_TBOOLEAN:
				{
					int v = lua_toboolean(L, -1);
					value.SetLong(v);
				}
				break;
				case LUA_TNUMBER:
				{
					uint64_t tmpValue = lua_tonumber(L, -1);
					value.SetLongLong(tmpValue);
				}
				break;
				case LUA_TSTRING:
				{
					const char * str = lua_tolstring(L, -1, &sz);
					if (sz > args->length)
						return SPROTO_CB_ERROR;
					value.SetString(str, sz);
				}
				break;
				default:
				{
					assert(0);
					return luaL_error(L, ".%s[%d] is not a userdata basicvalue (Is a %s)",
						args->tagname, args->index, lua_typename(L, lua_type(L, -1)));
				}
				break;
				}
			}

			sz = value.GetSeriazeLength();
			if (sz > args->length)
				return SPROTO_CB_ERROR;
			value.Seriaze((char*)args->value, args->length);
		}
		lua_pop(L, 1);
		return sz;
	}
	case SPROTO_CC_STRUCT:
	{
		struct encode_ud sub;
		int r;
		int top = lua_gettop(L);
		if (!lua_istable(L, top)) 
		{
			return luaL_error(L, ".%s[%d] is not a table (Is a %s)",
				args->tagname, args->index, lua_typename(L, lua_type(L, -1)));
		}
		sub.L = L;
		sub.st = args->subtype;
		sub.tbl_index = top;
		sub.array_tag = NULL;
		sub.array_index = 0;
		sub.deep = self->deep + 1;
		lua_pushnil(L);	// prepare an iterator slot
		sub.iter_index = sub.tbl_index + 1;
		r = sproto_encode(args->subtype, args->value, args->length, encode, &sub);
		lua_settop(L, top - 1);	// pop the value
		if (r < 0)
			return SPROTO_CB_ERROR;
		return r;
	}
	default:
		return luaL_error(L, "Invalid field type %d", args->type);
	}
}

static void *
expand_buffer(lua_State *L, int osz, int nsz) {
	void *output;
	do {
		osz *= 2;
	} while (osz < nsz);
	if (osz > ENCODE_MAXSIZE) {
		luaL_error(L, "object is too large (>%d)", ENCODE_MAXSIZE);
		return NULL;
	}
	output = lua_newuserdata(L, osz);
	lua_replace(L, lua_upvalueindex(1));
	lua_pushinteger(L, osz);
	lua_replace(L, lua_upvalueindex(2));

	return output;
}

/*
	lightuserdata sproto_type
	table source

	return string
 */
#include <basic.h>

static int
lencode(lua_State *L) 
{
	struct encode_ud self;
	basiclib::CBasicBitstream* m_pSMBuf = (basiclib::CBasicBitstream*)*((void**)lua_touserdata(L, 1));
	void * buffer = m_pSMBuf->GetDataBuffer();
	int sz = m_pSMBuf->GetAllocBufferLength();
	int tbl_index = 3;
	struct sproto_type * st = (struct sproto_type *)lua_touserdata(L, 2);
	if (st == NULL) 
	{
		return luaL_argerror(L, 1, "Need a sproto_type object");
	}
	luaL_checktype(L, tbl_index, LUA_TTABLE);
	luaL_checkstack(L, ENCODE_DEEPLEVEL*2 + 8, NULL);
	self.L = L;
	self.st = st;
	self.tbl_index = tbl_index;
	for (;;) {
		int r;
		self.array_tag = NULL;
		self.array_index = 0;
		self.deep = 0;

		lua_settop(L, tbl_index);
		lua_pushnil(L);	// for iterator (stack slot 3)
		self.iter_index = tbl_index+1;

		r = sproto_encode(st, buffer, sz, encode, &self);
		if (r<0) 
		{
			if (r == -1)
			{
				m_pSMBuf->SetDataLength(sz * 2);
				buffer = m_pSMBuf->GetDataBuffer();
				sz = m_pSMBuf->GetAllocBufferLength();
			}
			else
			{
				assert(0);
				return luaL_argerror(L, 1, "return error");;
			}
		} 
		else 
		{
			m_pSMBuf->SetDataLength(r);
			//lua_pushlightuserdata(L, st);
			//lua_pushlstring(L, (const char*)buffer, r);
			return 1;
		}
	}
}

struct decode_ud 
{
	lua_State *L;
	const char * array_tag;
	int array_index;
	int result_index;
	int deep;

	int key_index;
};

static int
decode(const struct sproto_arg *args) 
{
	int nDecodeLength = 0;
	struct decode_ud * self = (decode_ud *)args->ud;
	lua_State *L = self->L;
	if (self->deep >= ENCODE_DEEPLEVEL)
		return luaL_error(L, "The table is too deep");
	if (args->index != 0) 
	{
		// It's array
		if (args->tagname != self->array_tag) 
		{
			self->array_tag = args->tagname;
			lua_newtable(L);
			lua_pushvalue(L, -1);
			lua_setfield(L, self->result_index, args->tagname);
			if (self->array_index) 
			{
				lua_replace(L, self->array_index);
			} 
			else 
			{
				self->array_index = lua_gettop(L);
			}
			if (args->index < 0) 
			{
				// It's a empty array, return now.
				return 0;
			}
		}
		if (args->m_nMapKeyType > 0)
		{
			//map,先压入key
			switch (args->m_nMapKeyType)
			{
			case SPROTO_CC_CHAR:
			{
				lua_Number v = *(int8_t*)args->m_pMapKeyValue;
				lua_pushnumber(L, v);
			}
			break;
			case SPROTO_CC_SHORT:
			{
				lua_Number v = *(int16_t*)args->m_pMapKeyValue;
				lua_pushnumber(L, v);
			}
			break;
			case SPROTO_CC_INT:
			{
				lua_Number v = *(int32_t*)args->m_pMapKeyValue;
				lua_pushnumber(L, v);
			}
			break;
			case SPROTO_CC_UCHAR:
			case SPROTO_CC_USHORT:
			case SPROTO_CC_UINT:
			case SPROTO_CC_LONGLONG:
			{
				lua_Number v = *args->m_pMapKeyValue;
				lua_pushnumber(L, v);
			}
			break;
			case SPROTO_CC_DOUBLE:
			{
				uint64_t vValue = *args->m_pMapKeyValue;
				lua_Number v = *(double*)&vValue;
				lua_pushnumber(L, v);
			}
			break;
			default:
				assert(0);
				return luaL_error(L, "key not support type %d", args->m_nMapKeyType);
			}
		}
	}
	switch (args->type) 
	{
	case SPROTO_CC_CHAR:
	{
		lua_Integer v = *(int8_t*)args->value;
		lua_pushinteger(L, v);
		nDecodeLength = SPROTO_CC_CHAR_SIZE;
		break;
	}
	case SPROTO_CC_UCHAR:
	{
		lua_Integer v = *(uint64_t*)args->value;
		lua_pushinteger(L, v);
		nDecodeLength = SPROTO_CC_CHAR_SIZE;
		break;
	}
	case SPROTO_CC_SHORT:
	{
		lua_Integer v = *(int16_t*)args->value;
		lua_pushinteger(L, v);
		nDecodeLength = SPROTO_CC_SHORT_SIZE;
		break;
	}
	case SPROTO_CC_USHORT:
	{
		lua_Integer v = *(uint64_t*)args->value;
		lua_pushinteger(L, v);
		nDecodeLength = SPROTO_CC_SHORT_SIZE;
		break;
	}
	case SPROTO_CC_INT:
	{
		lua_Integer v = *(int32_t*)args->value;
		lua_pushinteger(L, v);
		nDecodeLength = SPROTO_CC_INT_SIZE;
		break;
	}
	case SPROTO_CC_UINT:
	{
		lua_Integer v = *(uint64_t*)args->value;
		lua_pushinteger(L, v);
		nDecodeLength = SPROTO_CC_INT_SIZE;
		break;
	}
	case SPROTO_CC_LONGLONG:
	{
		lua_Number v = *(uint64_t*)args->value;
		lua_pushnumber(L, v);
		nDecodeLength = SPROTO_CC_LONGLONG_SIZE;
		break;
	}
	case SPROTO_CC_DOUBLE:
	{
		lua_Number v = *(double*)args->value;
		lua_pushnumber(L, v);
		nDecodeLength = SPROTO_CC_DOUBLE_SIZE;
		break;
	}
	case SPROTO_CC_STRING:
	{
		lua_pushlstring(L, (const char*)args->value, args->length);
		nDecodeLength = args->length;
		break;
	}
	case SPROTO_CC_EXA_CNETBASICVALUE:
	{
		basiclib::CNetBasicValue netvalue;
		nDecodeLength = netvalue.UnSeriaze((const char*)args->value, args->length);
		switch (netvalue.GetDataType())
		{
		case basiclib::NETVALUE_NULL:
			lua_pushnil(L);
			break;
		case basiclib::NETVALUE_CHAR:
		case basiclib::NETVALUE_LONG:
		case basiclib::NETVALUE_LONGLONG:
		{
			lua_Number v = netvalue.GetLongLong();
			lua_pushnumber(L, v);
		}
			break;
		case basiclib::NETVALUE_DOUBLE:
		{
			lua_Number v = netvalue.GetDouble();
			lua_pushnumber(L, v);
		}
			break;
		case basiclib::NETVALUE_STRING:
			lua_pushlstring(L, (const char*)netvalue.GetStringRef(), netvalue.GetDataLength());
			break;
		default:
			assert(0);
			break;
		}
	}
		break;
	case SPROTO_CC_STRUCT:
	{
		struct decode_ud sub;
		int r;
		lua_newtable(L);
		sub.L = L;
		sub.result_index = lua_gettop(L);
		sub.deep = self->deep + 1;
		sub.array_index = 0;
		sub.array_tag = NULL;
		sub.key_index = 0;
		nDecodeLength = sproto_decode(args->subtype, args->value, args->length, decode, &sub);
		if (nDecodeLength < 0)
			return SPROTO_CB_ERROR;
		lua_settop(L, sub.result_index);
		break;
	}
	default:
	{
		assert(0);
		luaL_error(L, "Invalid type");
	}
	}
	if (args->index > 0) 
	{
		if (args->m_nMapKeyType > 0)
		{
			//map
			lua_settable(L, self->array_index);
		}
		else
		{
			//array
			lua_seti(L, self->array_index, args->index);
		}
	} 
	else 
	{
		lua_setfield(L, self->result_index, args->tagname);
	}

	return nDecodeLength;
}

static const void *
getbuffer(lua_State *L, int index, size_t *sz) 
{
	const void * buffer = NULL;
	int t = lua_type(L, index);
	if (t == LUA_TSTRING) {
		buffer = lua_tolstring(L, index, sz);
	} else {
		if (t != LUA_TUSERDATA && t != LUA_TLIGHTUSERDATA) {
			luaL_argerror(L, index, "Need a string or userdata");
			return NULL;
		}
		buffer = lua_touserdata(L, index);
		*sz = luaL_checkinteger(L, index+1);
	}
	return buffer;
}

/*
	lightuserdata sproto_type
	string source	/  (lightuserdata , integer)
	return table
 */
static int
ldecode(lua_State *L) 
{
	basiclib::CBasicBitstream* m_pSMBuf = (basiclib::CBasicBitstream*)*((void**)lua_touserdata(L, 1));

	struct sproto_type * st = (struct sproto_type *)lua_touserdata(L, 2);
	const void * buffer;
	struct decode_ud self;
	size_t sz;
	int r;
	if (st == NULL) 
	{
		return luaL_argerror(L, 1, "Need a sproto_type object");
	}
	sz = 0;
	
	buffer = m_pSMBuf->GetDataBuffer(); //getbuffer(L, 2, &sz);
	sz = m_pSMBuf->GetDataLength();
	if (!lua_istable(L, -1)) 
	{
		lua_newtable(L);
	}
	luaL_checkstack(L, ENCODE_DEEPLEVEL*3 + 8, NULL);
	self.L = L;
	self.result_index = lua_gettop(L);
	self.array_index = 0;
	self.array_tag = NULL;
	self.deep = 0;
	self.key_index = 0;
	r = sproto_decode(st, buffer, (int)sz, decode, &self);
	if (r < 0) 
	{
		if (m_pSMBuf->GetDataLength() > 8)
		{
			const unsigned char* pBegin = (const unsigned char*)m_pSMBuf->GetDataBuffer();
			//解析协议,读取前面8个字节
			Net_UInt nFlopKey = pBegin[0] | pBegin[1] << 8 | pBegin[2] << 16 | pBegin[3] << 24;
			Net_UInt nMethod = pBegin[4] | pBegin[5] << 8 | pBegin[6] << 16 | pBegin[7] << 24;
			char szBuf[32] = { 0 };
			sprintf(szBuf, "decode error %d %d", nFlopKey, nMethod);
			return luaL_error(L, szBuf);
		}
		else
		{
			return luaL_error(L, "decode error");
		}
	}
	lua_settop(L, self.result_index);
	lua_pushinteger(L, r);
	return 2;
}

static int
ldumpproto(lua_State *L) {
	struct sproto * sp = (sproto *)lua_touserdata(L, 1);
	if (sp == NULL) {
		return luaL_argerror(L, 1, "Need a sproto_type object");
	}
	sproto_dump(sp);

	return 0;
}


/*
	string source	/  (lightuserdata , integer)
	return string
 */
static int
lpack(lua_State *L) {
	size_t sz=0;
	const void * buffer = getbuffer(L, 1, &sz);
	// the worst-case space overhead of packing is 2 bytes per 2 KiB of input (256 words = 2KiB).
	size_t maxsz = (sz + 2047) / 2048 * 2 + sz + 2;
	void * output = lua_touserdata(L, lua_upvalueindex(1));
	int bytes;
	int osz = lua_tointeger(L, lua_upvalueindex(2));
	if (osz < maxsz) {
		output = expand_buffer(L, osz, maxsz);
	}
	bytes = sproto_pack(buffer, sz, output, maxsz);
	if (bytes > maxsz) {
		return luaL_error(L, "packing error, return size = %d", bytes);
	}
	lua_pushlstring(L, (const char*)output, bytes);

	return 1;
}

static int
lunpack(lua_State *L) {
	size_t sz=0;
	const void * buffer = getbuffer(L, 1, &sz);
	void * output = lua_touserdata(L, lua_upvalueindex(1));
	int osz = lua_tointeger(L, lua_upvalueindex(2));
	int r = sproto_unpack(buffer, sz, output, osz);
	if (r < 0)
		return luaL_error(L, "Invalid unpack stream");
	if (r > osz) {
		output = expand_buffer(L, osz, r);
		r = sproto_unpack(buffer, sz, output, r);
		if (r < 0)
			return luaL_error(L, "Invalid unpack stream");
	}
	lua_pushlstring(L, (const char*)output, r);
	return 1;
}

static void
pushfunction_withbuffer(lua_State *L, const char * name, lua_CFunction func) {
	lua_newuserdata(L, ENCODE_BUFFERSIZE);
	lua_pushinteger(L, ENCODE_BUFFERSIZE);
	lua_pushcclosure(L, func, 2);
	lua_setfield(L, -2, name);
}

/* global sproto pointer for multi states
   NOTICE : It is not thread safe
 */
static struct sproto * G_sproto[MAX_GLOBALSPROTO];

static int
lsaveproto(lua_State *L) {
	struct sproto * sp = (sproto*)lua_touserdata(L, 1);
	int index = luaL_optinteger(L, 2, 0);
	if (index < 0 || index >= MAX_GLOBALSPROTO) {
		return luaL_error(L, "Invalid global slot index %d", index);
	}
	/* TODO : release old object (memory leak now, but thread safe)*/
	G_sproto[index] = sp;
	return 0;
}

static int
lloadproto(lua_State *L) {
	int index = luaL_optinteger(L, 1, 0);
	struct sproto * sp;
	if (index < 0 || index >= MAX_GLOBALSPROTO) {
		return luaL_error(L, "Invalid global slot index %d", index);
	}
	sp = G_sproto[index];
	if (sp == NULL) {
		return luaL_error(L, "nil sproto at index %d", index);
	}

	lua_pushlightuserdata(L, sp);

	return 1;
}

static int
encode_default(const struct sproto_arg *args) {
	lua_State *L = (lua_State*)args->ud;
	lua_pushstring(L, args->tagname);
	if (args->index > 0) 
	{
		lua_newtable(L);
		lua_rawset(L, -3);
		return SPROTO_CB_NOARRAY;
	} 
	else 
	{
		switch(args->type) 
		{
		case SPROTO_CC_CHAR:
		case SPROTO_CC_UCHAR:
			lua_pushinteger(L, 0);
			break;
		case SPROTO_CC_SHORT:
		case SPROTO_CC_USHORT:
			lua_pushinteger(L, 0);
			break;
		case SPROTO_CC_INT:
		case SPROTO_CC_UINT:
			lua_pushinteger(L, 0);
			break;
		case SPROTO_CC_LONGLONG:
			lua_pushinteger(L, 0);
			break;
		case SPROTO_CC_DOUBLE:
			lua_pushinteger(L, 0);
			break;
		case SPROTO_CC_STRING:
			lua_pushliteral(L, "");
			break;
		case SPROTO_CC_STRUCT:
			lua_createtable(L, 0, 1);
			lua_pushstring(L, sproto_name(args->subtype));
			lua_setfield(L, -2, "__type");
			break;
		case SPROTO_CC_EXA_CNETBASICVALUE:
			lua_pushinteger(L, 0);
			break;
		}
		lua_rawset(L, -3);
		return SPROTO_CB_NIL;
	}
}

/*
	lightuserdata sproto_type
	return default table
 */
static int
ldefault(lua_State *L) {
	int ret;
	// 64 is always enough for dummy buffer, except the type has many fields ( > 27).
	char dummy[64];
	struct sproto_type * st = (struct sproto_type *)lua_touserdata(L, 1);
	if (st == NULL) {
		return luaL_argerror(L, 1, "Need a sproto_type object");
	}
	lua_newtable(L);
	ret = sproto_encode(st, dummy, sizeof(dummy), encode_default, L);
	if (ret<0) {
		// try again
		int sz = sizeof(dummy) * 2;
		void * tmp = lua_newuserdata(L, sz);
		lua_insert(L, -2);
		for (;;) {
			ret = sproto_encode(st, tmp, sz, encode_default, L);
			if (ret >= 0)
				break;
			sz *= 2;
			tmp = lua_newuserdata(L, sz);
			lua_replace(L, -3);
		}
	}
	return 1;
}

int
luaopen_sproto_core(lua_State *L) {
#ifdef luaL_checkversion
	luaL_checkversion(L);
#endif
	luaL_Reg l[] = {
		{ "newproto", lnewproto },
		{ "deleteproto", ldeleteproto },
		{ "dumpproto", ldumpproto },
		{ "querytype", lquerytype },
		{ "decode", ldecode },
		{ "loadproto", lloadproto },
		{ "saveproto", lsaveproto },
		{ "default", ldefault },
		{ NULL, NULL },
	};
	//luaL_register(L, "sproto.core", l);
	//luaL_newlib(L,l);
	pushfunction_withbuffer(L, "encode", lencode);
	pushfunction_withbuffer(L, "pack", lpack);
	pushfunction_withbuffer(L, "unpack", lunpack);

	return 1;
}
