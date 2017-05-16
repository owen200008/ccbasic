local lpeg = require "lpeg"
local table = require "table"

local packbytes
local packvalue

if _VERSION == "Lua 5.3" then
	function packbytes(str)
		return string.pack("<s4",str)
	end

	function packvalue(id)
		id = (id + 1) * 2
		return string.pack("<I2",id)
	end
else
	function packbytes(str)
		local size = #str
		local a = size % 256
		size = math.floor(size / 256)
		local b = size % 256
		size = math.floor(size / 256)
		local c = size % 256
		size = math.floor(size / 256)
		local d = size
		return string.char(a)..string.char(b)..string.char(c)..string.char(d) .. str
	end

	function packvalue(id)
		id = (id + 1) * 2
		assert(id >=0 and id < 65536)
		local a = id % 256
		local b = math.floor(id / 256)
		return string.char(a) .. string.char(b)
	end
end

local P = lpeg.P
local S = lpeg.S
local R = lpeg.R
local C = lpeg.C
local Ct = lpeg.Ct
local Cg = lpeg.Cg
local Cc = lpeg.Cc
local V = lpeg.V

local function count_lines(_,pos, parser_state)
	if parser_state.pos < pos then
		parser_state.line = parser_state.line + 1
		parser_state.pos = pos
	end
	return pos
end

local exception = lpeg.Cmt( lpeg.Carg(1) , function ( _ , pos, parser_state)
	error(string.format("syntax error at [%s] line (%d)", parser_state.file or "", parser_state.line))
	return pos
end)

local eof = P(-1)
local newline = lpeg.Cmt((P"\n" + "\r\n") * lpeg.Carg(1) ,count_lines)
local line_comment = "//" * (1 - newline) ^0 * (newline + eof)
local blank = S" \t" + newline + line_comment
local blank0 = blank ^ 0
local blanks = blank ^ 1
local alpha = R"az" + R"AZ" + "_"
local alnum = alpha + R"09"
local word = alpha * alnum ^ 0
local name = C(word)
local alphaChild = alpha + "."
local alnumChild = alphaChild + R"09"
local wordChild = alphaChild * alnumChild ^ 0
local nameChild = C(wordChild)
local typename = C(word * ("." * word) ^ 0)
local typedefword = C(word * ("<" * word * (blank0 * "," * blank0 * word) ^ 0 * ">") ^ 0)
local tag = S'-'^-1 * R"09" ^ 1 / tonumber
local endCCode = lpeg.Cmt((P";") * lpeg.Carg(1) ,count_lines)

local function multipat(pat)
	return Ct(blank0 * (pat * blanks) ^ 0 * pat^0 * blank0)
end

local function namedpat(name, pat)
	return Ct(Cg(Cc(name), "type") * Cg(pat))
end

local typedef = P {
	"ALL",
    COMMENTSTAR = namedpat("commentstar", P"/*" * (1 - P"*/") ^0 * P"*/"),
    COMMENTDEFINE = namedpat("commentdefine", P"#" * (1 - newline) ^0 * (newline + eof)),
    FIELD = namedpat("field", (typename * blank0 * (C"*")^-1 * blank0 * name * blank0 * ";")),
    TYPEDEFFIELD = namedpat("typedeffield", (P"typedef" * blanks * typedefword * blanks * typename * blank0 * ";")),
    DEFAULTVALUE = namedpat("defaultvalue", (name * blank0 * "=" * blank0 * tag * blank0 * ";")),
	DEFAULTNULLPTR = namedpat("defaultnullptr", (name * blank0 * "=" * blank0 * (1 - endCCode) ^1 * blank0 * ";")),
	DEFAULTVALUECHILD = namedpat("defaultvaluechild", (nameChild * blank0 * "=" * blank0 * (1 - endCCode) ^1 * blank0 * ";")),
    DEFAULTSTRUCT = P"{" * multipat(V"DEFAULTVALUE" + V"DEFAULTNULLPTR" + V"DEFAULTVALUECHILD") * P"}",
    DEFAULT = namedpat("default", (name * P"()" * blank0 * V"DEFAULTSTRUCT")),
	STRUCT = P"{" * multipat(V"FIELD" + V"COMMENTSTAR" + V"TYPEDEFFIELD") * blank0 * V"DEFAULT"^0 * blank0 * P"};",
	TYPE = namedpat("type", P"struct"* blanks * name * blank0 * V"STRUCT" ),
	ALL = multipat(V"TYPE" + V"COMMENTSTAR" + V"COMMENTDEFINE" + V"TYPEDEFFIELD" + blanks),
}

local arrayormap = P {
    "ALL",
    ARRAY = namedpat("array", P"Net_Vector<" * blank0 * typename * blank0 * P">"),
    MAP = namedpat("map", P"Net_Map<" * blank0 * typename * blank0 * P"," * blank0 * typename * blank0 * P">"),
    ALL = V"ARRAY" + V"MAP",
}

local proto = blank0 * typedef * blank0
local protoarraymap = blank0 * arrayormap * blank0

local convert = {}

function convert.type(all, obj, set)
	local result = {}
	local typename = obj[1]
	local names = {}
	for _, f in ipairs(obj[2]) do
		if f.type == "field" then
            local fieldtype = f[1]
            local bStar = 0
            local name = f[2]
            if f[2] == "*" then
                name = f[3]
                bStar = 1
            end
            
            if names[name] then
				error(string.format("redefine %s in type %s", name, typename))
			end
			names[name] = true
            local field = { name = name, star = bStar, array = 0, keytype = 0, typename = fieldtype, defaultvalue = 0}
			table.insert(result, field)
        elseif f.type == "commentstar" then
            --do nothing
        elseif f.type == "typedeffield" then
            local typedeffiled = f[1]
            local name = f[2]
            if set[typedeffiled] then
                set[name] = set[typedeffiled]
            else 
                set[name] = typedeffiled
            end
		else
			assert(f.type == "type")	-- nest type
			local nesttypename = typename .. "." .. f[1]
			f[1] = nesttypename
			assert(all.type[nesttypename] == nil, "redefined " .. nesttypename)
			all.type[nesttypename] = convert.type(all, f)
		end
	end
    if obj[3] then
        for _,defaultvalue in pairs(obj[3][2]) do
            local bFind = false
            for _, field in pairs(result) do
                if field.name == defaultvalue[1] then
                    field.defaultvalue = defaultvalue[2]
                    bFind = true
                    break
                end
            end
        end
    end
	return result
end

function convert.typedeffield(all, obj, set)
	local typedeffiled = obj[1]
    local name = obj[2]
    if set[name] then
        return set[name]
    end
    return typedeffiled
end

local function adjust(r)
	local result = { type = {}, typedeffield = {} }

	for _, obj in ipairs(r) do
        if obj.type ~= "commentstar" and obj.type ~= "commentdefine" and obj.type ~= "typedeffield" then
        	local set = result[obj.type]
		    local name = obj[1]
        
		    assert(set[name] == nil , "redefined " .. name)
		    set[name] = convert[obj.type](result,obj,result["typedeffield"])
            assert(set[name] ~= nil , "error " .. name)
        elseif obj.type == "typedeffield" then
            local set = result[obj.type]
		    local name = obj[2]
        
		    assert(set[name] == nil , "redefined " .. name)
		    set[name] = convert[obj.type](result,obj,result["typedeffield"])
            assert(set[name] ~= nil , "error " .. name)
        end
	end

	return result
end

local buildin_types = {
    Net_Char = 10,
    Net_Short = 11,
    Net_Int = 12,
    Net_LONGLONG = 13,
    Net_Double = 14,
    Net_UChar = 15,
    Net_UShort = 16,
    Net_UInt = 17,
    Net_CBasicString = 18,
	Net_CBasicBitstream = 18,
    --外置的类型
    Net_CNetBasicValue = 100,
	
}

local function checktype(types, ptype, t)
	if buildin_types[t] then
		return t
	end
	local fullname = ptype .. "." .. t
	if types[fullname] then
		return fullname
	else
		ptype = ptype:match "(.+)%..+$"
		if ptype then
			return checktype(types, ptype, t)
		elseif types[t] then
			return t
		end
	end
end

local function flattypename(r)
	for typename, t in pairs(r.type) do
		for _, f in pairs(t) do
			local ftype = f.typename
			local fullname = checktype(r.type, typename, ftype)
			if fullname == nil then
                --匹配是否是array或者map
                if r.typedeffield[ftype] then
                    ftype = r.typedeffield[ftype]
                end
                local state = { file = "", pos = 0, line = 1 }
                local fieldtyper = lpeg.match(protoarraymap * -1 + exception , ftype , 1, state )
                if fieldtyper == nil then
                    error(string.format("type match error %s in type %s %d", ftype, typename))
                end
                if fieldtyper.type == "array" then
                    --标示数组
                    f.array = 1
                    --检查内部结构是否可以序列化
                    fullname = checktype(r.type, typename, fieldtyper[1])
                elseif fieldtyper.type == "map" then
                    --标示数组
                    f.array = 1
                    --检查内部结构是否可以序列化
                    f.keytype = checktype(r.type, typename, fieldtyper[1])
                    fullname = checktype(r.type, typename, fieldtyper[2])
                else
                    error(string.format("unknow type match %s in type %s", ftype, typename))
                end

                if fullname == nil then
                    error(string.format("Undefined type %s in type %s", ftype, typename))
                end
			end
			f.typename = fullname
		end
	end

	return r
end

local function parser(text,filename)
	local state = { file = filename, pos = 0, line = 1 }
	local r = lpeg.match(proto * -1 + exception , text , 1, state )
	return flattypename(adjust(r))
end

local function packfield(f)
	local strtbl = {}
    table.insert(strtbl, "\7\0")  -- 7 fields

    table.insert(strtbl, "\0\0")	-- name	(index = 0, ref an object)
    if f.buildin then
		table.insert(strtbl, packvalue(f.buildin))	-- buildin (index = 1)
		table.insert(strtbl, "\1\0")	-- skip (index = 2)
		table.insert(strtbl, packvalue(f.star))		-- star (index = 3)
	else
		table.insert(strtbl, "\1\0")	-- skip (index = 1)
		table.insert(strtbl, packvalue(f.type))		-- type (index = 2)
		table.insert(strtbl, packvalue(f.star))		-- star (index = 3)
	end
    table.insert(strtbl, packvalue(f.array))	-- array = true (index = 4)
    table.insert(strtbl, packvalue(f.keytype)) -- key (index = 5)
    table.insert(strtbl, packvalue(f.defaultvalue)) -- default (index = 6)
    
	table.insert(strtbl, packbytes(f.name)) -- external object (name)
	return packbytes(table.concat(strtbl))
end

local function packtype(name, t, alltypes)
	local fields = {}
	local tmp = {}
	for _, f in ipairs(t) do
		tmp.array = f.array
		tmp.name = f.name
		tmp.star = f.star
        tmp.defaultvalue = f.defaultvalue
        tmp.keytype = buildin_types[f.keytype] or 0

		tmp.buildin = buildin_types[f.typename]
		local subtype
		if not tmp.buildin then
			subtype = assert(alltypes[f.typename])
			tmp.type = subtype.id
		else
			tmp.type = nil
		end

		table.insert(fields, packfield(tmp))
	end
	local data
	if #fields == 0 then
		data = {
			"\1\0",	-- 1 fields
			"\0\0",	-- name	(id = 0, ref = 0)
			packbytes(name),
		}
	else
		data = {
			"\2\0",	-- 2 fields
			"\0\0",	-- name	(ref = 0)
			"\0\0", -- field[]	(ref = 1)
			packbytes(name),
			packbytes(table.concat(fields)),
		}
	end

	return packbytes(table.concat(data))
end

local function packgroup(t)
	if next(t) == nil then
		return "\0\0"
	end
	local tt
	local alltypes = {}
	for name in pairs(t) do
		table.insert(alltypes, name)
	end
	--table.sort(alltypes)	-- make result stable
	for idx, name in ipairs(alltypes) do
		alltypes[name] = { id = idx - 1 }
	end
	tt = {}
	for _,name in ipairs(alltypes) do
		table.insert(tt, packtype(name, t[name], alltypes))
	end
	tt = packbytes(table.concat(tt))
	
	local result = {
			"\1\0",	-- 1 field
			"\0\0",	-- type[] (id = 0, ref = 0)
			tt,
		}
	return table.concat(result)
end

local function encodeall(r)
	return packgroup(r.type)
end

local sparser = {}

function sparser.dump(str)
	local tmp = ""
	for i=1,#str do
		tmp = tmp .. string.format("%02X ", string.byte(str,i))
		if i % 8 == 0 then
			if i % 16 == 0 then
				print(tmp)
				tmp = ""
			else
				tmp = tmp .. "- "
			end
		end
	end
	print(tmp)
end

function sparser.parse(text, name)
	local r = parser(text, name or "=text")
	local data = encodeall(r)
	return data
end

local function encodetocpp(r, filename)
    local headfileformat = [[
#ifndef AUTO_SPROTOPROTOCAL_H
#define AUTO_SPROTOPROTOCAL_H

#include "%s"

%s
#endif
]];
    local cppfileformat = [[
#include "%s"

%s
]]
    local xlhformathead = [[
basiclib::CBasicBitstream& operator<<(basiclib::CBasicBitstream& ins, const %s& data);
basiclib::CBasicBitstream& operator>>(basiclib::CBasicBitstream& os, %s& data);

]]
    local xlhformatcpp = [[
basiclib::CBasicBitstream& operator<<(basiclib::CBasicBitstream& ins, const %s& data){
    %sreturn ins;
}
basiclib::CBasicBitstream& operator>>(basiclib::CBasicBitstream& os, %s& data){
    %sreturn os;
}

]]
    local fieldstarformat_in = [[data.%s ? ins << (Net_Char)1 << *data.%s : ins << (Net_Char)0;
    ]]
    local fieldstarformat_out = [[os >> cStar;cStar ? os >> *data.%s : os;
    ]]
    local fieldstarformat_out_ext=[[Net_Char cStar = 0;
    ]]
    local fieldformat_in = [[ins << data.%s;
    ]]
    local fieldformat_out = [[os >> data.%s;
    ]]

    local _,_,strPath, strFileName, strHouZhui = filename:find("(.*)[/]([^/]+)([.][^.]*)")
    local writefilenamehead = strPath.."/"..strFileName.."_auto.h"
    local writefilenamecpp = strPath.."/"..strFileName.."_auto.cpp"
    local filenameinclude = strFileName..strHouZhui
    local writeheadfp = io.open(writefilenamehead, "w")
    local writecppfp = io.open(writefilenamecpp, "w")
    local headwritedata = ""
    local cppwritedata = ""
    for name, structtype in pairs(r.type) do
        headwritedata = headwritedata..string.format(xlhformathead, name, name)
        local fieldsin = ""
        local fieldsout = ""
        local bStar = false
        for _, type_fields in ipairs(structtype) do
            if type_fields.star > 0 then
                if bStar == false then
                    fieldsout = fieldstarformat_out_ext..fieldsout
                    bStar = true
                end 
                fieldsin = fieldsin.. string.format(fieldstarformat_in, type_fields.name, type_fields.name)
                fieldsout = fieldsout.. string.format(fieldstarformat_out, type_fields.name)
            else
                fieldsin = fieldsin.. string.format(fieldformat_in, type_fields.name)
                fieldsout = fieldsout.. string.format(fieldformat_out, type_fields.name)
            end
		end
        cppwritedata = cppwritedata..string.format(xlhformatcpp, name, fieldsin, name, fieldsout)
    end

    writeheadfp:write( string.format(headfileformat, filenameinclude, headwritedata))
    writecppfp:write( string.format(cppfileformat, strFileName.."_auto.h", cppwritedata))

    writeheadfp:close()
    writecppfp:close()

end

function sparser.parsetocpp(text, filename)
     local r = parser(text, filename)
     encodetocpp(r, filename)
end

return sparser
