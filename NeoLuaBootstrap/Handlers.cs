using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace NeoLuaBootstrap
{
    public static class Handlers
    {
        [DefaultDllImportSearchPaths(Scripting.SearchPath)]
        [DllImport(Scripting.AppName, CallingConvention = CallingConvention.Cdecl)]
        static extern void Lua_RegisterUninitializeCallback(OnClose callback);

        static public int OnInit(string scriptPath)
        {
            try
            {
                Scripting.Uninitialize();
                Scripting.Initialize(scriptPath);
                Lua_RegisterUninitializeCallback(OnCloseDelegate);
                return 0;
            }
            catch (Exception e)
            {
                Logging.ToFile("[NeoLua] {0}", e);
                Scripting.Uninitialize();
                return 1;
            }
        }

        [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        delegate void OnClose(bool isProcessTerminating);

        static readonly OnClose OnCloseDelegate = OnClose_Impl;
        static public void OnClose_Impl(bool isProcessTerminating)
        {
            Scripting.Uninitialize(isProcessTerminating);
        }
    }
}
