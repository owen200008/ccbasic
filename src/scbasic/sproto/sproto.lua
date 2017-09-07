local core = require "sproto.core"
local assert = assert

local sproto = {}
local host = {}

local weak_mt = { __mode = "kv" }
local sproto_mt = { __index = sproto }
local sproto_nogc = { __index = sproto }
local host_mt = { __index = host }

function sproto_mt:__gc()
	core.deleteproto(self.__cobj)
end

function sproto.new(bin)
	local cobj = assert(core.newproto(bin))
	local self = {
		__cobj = cobj,
		__tcache = setmetatable( {} , weak_mt ),
		__pcache = setmetatable( {} , weak_mt ),
	}
	return setmetatable(self, sproto_mt)
end

function sproto.sharenew(cobj)
	local self = {
		__cobj = cobj,
		__tcache = setmetatable( {} , weak_mt ),
		__pcache = setmetatable( {} , weak_mt ),
	}
	return setmetatable(self, sproto_nogc)
end

function sproto.parse(ptext, filename)
	local parser = require "sprotoparser"
	local pbin = parser.parse(ptext)
    if filename then
        os.remove(filename)
        io.writefile(filename, pbin, "wb+")
    end
	return sproto.new(pbin)
end

function sproto:host( packagename )
	packagename = packagename or  "package"
	local obj = {
		__proto = self,
		__package = assert(core.querytype(self.__cobj, packagename), "type package not found"),
		__session = {},
	}
	return setmetatable(obj, host_mt)
end

local function querytype(self, typename)
	local v = self.__tcache[typename]
	if not v then
		v = assert(core.querytype(self.__cobj, typename), "type not found")
		self.__tcache[typename] = v
	end

	return v
end

function sproto:exist_type(typename)
	local v = self.__tcache[typename]
	if not v then
		return core.querytype(self.__cobj, typename) ~= nil
	else
		return true
	end
end

function sproto:encode(smbuf, typename, tbl)
	local st = querytype(self, typename)
	return core.encode(smbuf, st, tbl)
end

function sproto:encodestr(typename, tbl, bBase64, bZip)
	local st = querytype(self, typename)
	return core.encodestr(st, tbl, bBase64, bZip)
end

function sproto:decode(smbuf, typename)
	local st = querytype(self, typename)
	return core.decode(smbuf, st)
end

function sproto:decodestr(str, typename, bBase64)
	local st = querytype(self, typename)
	return core.decodestr(str, st, bBase64)
end

function sproto:pencode(typename, tbl)
	local st = querytype(self, typename)
	return core.pack(core.encode(st, tbl))
end

function sproto:pdecode(typename, ...)
	local st = querytype(self, typename)
	return core.decode(st, core.unpack(...))
end

sproto.pack = core.pack
sproto.unpack = core.unpack

function sproto:default(typename)
	return core.default(querytype(self, typename))
end

return sproto
