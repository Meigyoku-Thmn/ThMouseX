ThMouseX_DllPath = {}

function InitializeForLuaJIT()    
    local ffi = require("ffi")

    ffi.cdef [[
        typedef unsigned long DWORD;
        typedef void* LPVOID;
        typedef const wchar_t* LPCWSTR;
        typedef const char* LPCSTR;
        typedef void (*UninitializeCallbackType)(bool isProcessTerminating);

        int         Lua_CreateHookApi       (LPCSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, LPVOID *ppOriginal);
        DWORD       Lua_ReadUInt32          (DWORD address);
        DWORD       Lua_ResolveAddress      (DWORD* offsets, int length);
        void        Lua_OpenConsole         ();
        void        Lua_SetPositionAddress  (DWORD address);
        int         Lua_GetDataType         ();

        void        Lua_RegisterUninitializeCallback    (UninitializeCallbackType callback);

        int         __stdcall MH_CreateHook     (LPVOID pTarget, LPVOID pDetour, LPVOID *ppOriginal);
        int         __stdcall MH_EnableHook     (LPVOID pTarget);
        int         __stdcall MH_RemoveHook     (LPVOID pTarget);
        int         __stdcall MH_DisableHook    (LPVOID pTarget);
        const char* __stdcall MH_StatusToString (int status);
    ]]

    local ThMouseX = ffi.load(ThMouseX_DllPath)

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
        return ThMouseX.Lua_CreateHookApi(module, procName, detour, outOriginal)
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
end