-- LuaDllName = lua.dll
InitializeForLuaJIT();
local f = io.open("log.txt", "a")
f:write("Hello World\n")
f:close()