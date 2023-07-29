local ffi = require("ffi")

ffi.cdef [[
    DWORD       Lua_ReadUInt32          (DWORD address);
    DWORD       Lua_ResolveAddress      (DWORD* offsets, int length);
    void        Lua_OpenConsole         ();
    void        Lua_SetPositionAddress  (DWORD address);
    int         Lua_GetDataType         ();

    typedef void (*UninitializeCallbackType)(bool isProcessTerminating);

    void        Lua_RegisterUninitializeCallback    (UninitializeCallbackType callback)
    
    wchar_t*    Lua_ConvertToUtf16Alloc (const char* utf8str);
    void        Lua_DeleteUtf16Alloc    (wchar_t* utf16);

    int         stdcall MH_CreateHook       (LPVOID pTarget, LPVOID pDetour, LPVOID *ppOriginal);
    int         stdcall MH_CreateHookApi    (LPCWSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, LPVOID *ppOriginal);
    int         stdcall MH_EnableHook       (LPVOID pTarget);
    int         stdcall MH_RemoveHook       (LPVOID pTarget);
    int         stdcall MH_DisableHook      (LPVOID pTarget);
    const char* stdcall MH_StatusToString   (int status);
]]

-- {} should be replaced by a path to ThMouseX.dll
local ThMouseX = ffi.load("{}")

function ReadUInt32(address)
    return ThMouseX.Lua_ReadUInt32(address)
end

function ResolveAddress(addressChain, length)
    return ThMouseX.Lua_ResolveAddress(addressChain, length)
end

function OpenConsole()
    return ThMouseX.Lua_OpenConsole()
end

function SetPositionAddress(address)
    return ThMouseX.Lua_SetPositionAddress(address)
end

function GetDataType()
    return ThMouseX.Lua_GetDataType()
end

function RegisterUninitializeCallback(callback)
    return ThMouseX.Lua_RegisterUninitializeCallback(callback)
end

function CreateHook(target, detour, outOriginal)
    return MH_CreateHook(target, detour, outOriginal)
end

function CreateHookApi(module, procName, detour, outOriginal)
    local moduleUtf16 = ThMouseX.Lua_ConvertToUtf16Alloc(module)
    local rs = ThMouseX.MH_CreateHookApi(moduleUtf16, procName, detour, outOriginal)
    ThMouseX.Lua_DeleteUtf16Alloc(moduleUtf16)
    return rs;
end

function EnableHook(target)
    return ThMouseX.MH_EnableHook(target)
end

function RemoveHook(target)
    return ThMouseX.MH_RemoveHook(target)
end

function DisableHook(target)
    return ThMouseX.MH_DisableHook(target)
end

function HookStatusToString(status)
    return ffi.string(ThMouseX.MH_StatusToString(status))
end