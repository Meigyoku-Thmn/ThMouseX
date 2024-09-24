-- expected global variables: ThMouseX_ModuleHandle

function InitializeForLuaJIT()
    local ffi = require("ffi")

    ffi.cdef [[
        typedef unsigned long DWORD;
        typedef void* LPVOID;
        typedef LPVOID HMODULE;
        typedef const wchar_t* LPCWSTR;
        typedef const char* LPCSTR;
        typedef void (*UninitializeCallbackType)(bool isProcessTerminating);
        
        LPVOID          GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
        typedef DWORD   (*Lua_ReadUInt32)(DWORD address);
        typedef DWORD   (*Lua_ResolveAddress)(DWORD* offsets, size_t length);
        typedef void    (*Lua_OpenConsole)();
        typedef void    (*Lua_SetPositionAddress)(DWORD address);
        typedef int     (*Lua_GetDataType)();
        typedef void    (*Lua_RegisterUninitializeCallback)(UninitializeCallbackType callback);
        
        typedef int     (__stdcall *MH_CreateHook)(LPVOID pTarget, LPVOID pDetour, LPVOID *ppOriginal);
        typedef int     (*Lua_CreateHookApi)(LPCSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, LPVOID *ppOriginal);
        typedef int     (__stdcall *MH_EnableHook)(LPVOID pTarget);
        typedef int     (__stdcall *MH_RemoveHook)(LPVOID pTarget);
        typedef int     (__stdcall *MH_DisableHook)(LPVOID pTarget);
        typedef LPCSTR  (__stdcall *MH_StatusToString)(int status);
    ]]

    local hModule = ffi.cast("HMODULE", ThMouseX_ModuleHandle)
    local config = {
        { 'ReadUInt32',                     'Lua_ReadUInt32'                    },
        { 'ResolveAddress',                 'Lua_ResolveAddress'                },
        { 'OpenConsole',                    'Lua_OpenConsole'                   },
        { 'SetPositionAddress',             'Lua_SetPositionAddress'            },
        { 'GetDataType',                    'Lua_GetDataType'                   },
        { 'RegisterUninitializeCallback',   'Lua_RegisterUninitializeCallback'  },
        { 'CreateHook',                     'MH_CreateHook'                     },
        { 'CreateHookApi',                  'Lua_CreateHookApi'                 },
        { 'EnableHook',                     'MH_EnableHook'                     },
        { 'RemoveHook',                     'MH_RemoveHook'                     },
        { 'DisableHook',                    'MH_DisableHook'                    },
        { '_HookStatusToString',            'MH_StatusToString'                 },
    }
    for i = 1, #config do
        local item = config[i]
        _G[item[1]] = ffi.cast(item[2], ffi.C.GetProcAddress(hModule, item[2]))
    end
    function HookStatusToString(status)
        return ffi.string(_HookStatusToString(status))
    end
end