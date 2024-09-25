-- C Processor-Generated file, do not edit.
-- Lua script expects ThMouseX_ModuleHandle as global variable
local ffi = require("ffi")
ffi.cdef([[
    typedef unsigned long DWORD;
    typedef void* LPVOID;
    typedef LPVOID HMODULE;
    typedef const wchar_t* LPCWSTR;
    typedef const char* LPCSTR;
    typedef void(__cdecl *UninitializeCallbackType)(bool isProcessTerminating);
    typedef void(__cdecl *CallbackType)();
    LPVOID GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
]])
local hModule = ffi.cast("HMODULE", ThMouseX_ModuleHandle)

ffi.cdef([[ typedef DWORD (*Lua_ReadUInt32)(DWORD address); ]])
ReadUInt32 = ffi.cast("Lua_ReadUInt32", ffi.C.GetProcAddress(hModule, "Lua_ReadUInt32"))

ffi.cdef([[ typedef DWORD (*Lua_ResolveAddress)(DWORD* offsets, size_t length); ]])
ResolveAddress = ffi.cast("Lua_ResolveAddress", ffi.C.GetProcAddress(hModule, "Lua_ResolveAddress"))

ffi.cdef([[ typedef void (*Lua_OpenConsole)(); ]])
OpenConsole = ffi.cast("Lua_OpenConsole", ffi.C.GetProcAddress(hModule, "Lua_OpenConsole"))

ffi.cdef([[ typedef void (*Lua_SetPositionAddress)(DWORD address); ]])
SetPositionAddress = ffi.cast("Lua_SetPositionAddress", ffi.C.GetProcAddress(hModule, "Lua_SetPositionAddress"))

ffi.cdef([[ typedef int (*Lua_GetDataType)(); ]])
GetDataType = ffi.cast("Lua_GetDataType", ffi.C.GetProcAddress(hModule, "Lua_GetDataType"))

ffi.cdef([[ typedef void (*Lua_RegisterUninitializeCallback)(UninitializeCallbackType callback); ]])
RegisterUninitializeCallback = ffi.cast("Lua_RegisterUninitializeCallback", ffi.C.GetProcAddress(hModule, "Lua_RegisterUninitializeCallback"))

ffi.cdef([[ typedef int (*Lua_CreateHook)(LPVOID pTarget, LPVOID pDetour, LPVOID *ppOriginal); ]])
CreateHook = ffi.cast("Lua_CreateHook", ffi.C.GetProcAddress(hModule, "Lua_CreateHook"))

ffi.cdef([[ typedef int (*Lua_CreateHookApi)(LPCSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, LPVOID *ppOriginal); ]])
CreateHookApi = ffi.cast("Lua_CreateHookApi", ffi.C.GetProcAddress(hModule, "Lua_CreateHookApi"))

ffi.cdef([[ typedef int (*Lua_EnableHook)(LPVOID pTarget); ]])
EnableHook = ffi.cast("Lua_EnableHook", ffi.C.GetProcAddress(hModule, "Lua_EnableHook"))

ffi.cdef([[ typedef int (*Lua_RemoveHook)(LPVOID pTarget); ]])
RemoveHook = ffi.cast("Lua_RemoveHook", ffi.C.GetProcAddress(hModule, "Lua_RemoveHook"))

ffi.cdef([[ typedef int (*Lua_DisableHook)(LPVOID pTarget); ]])
DisableHook = ffi.cast("Lua_DisableHook", ffi.C.GetProcAddress(hModule, "Lua_DisableHook"))

ffi.cdef([[ typedef LPCSTR (*Lua_StatusToString)(int status); ]])
StatusToString = ffi.cast("Lua_StatusToString", ffi.C.GetProcAddress(hModule, "Lua_StatusToString"))

_StatusToString = StatusToString
function StatusToString(status)
    return ffi.string(_StatusToString(status))
end
