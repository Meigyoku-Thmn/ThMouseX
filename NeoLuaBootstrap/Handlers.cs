using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace NeoLuaBootstrap
{
    using DelegateMap = Dictionary<string, (Delegate Delegate, GCHandle Handle)>;
    public static class Handlers
    {
        [DllImport(Scripting.AppName, CallingConvention = CallingConvention.Cdecl)]
        static extern void NeoLua_SetOnClose(IntPtr address);

        static readonly DelegateMap EventDelegates = new DelegateMap();

        static Handlers()
        {
            var onCloseDelegate = new OnClose(OnClose_Impl);
            var gcOnCloseHandle = GCHandle.Alloc(onCloseDelegate);
            EventDelegates.Add(nameof(OnClose), (onCloseDelegate, gcOnCloseHandle));
        }

        static public int OnInit(string scriptPath)
        {
            try
            {
                Scripting.Uninitialize();
                Scripting.Initialize(scriptPath);
                NeoLua_SetOnClose(Marshal.GetFunctionPointerForDelegate(EventDelegates[nameof(OnClose)].Delegate));
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
        delegate void OnClose();
        static public void OnClose_Impl()
        {
            Scripting.Uninitialize();
        }
    }
}
