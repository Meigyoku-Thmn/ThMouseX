using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace NeoLuaBootstrap
{
    internal static class LuaApi
    {
        static readonly string ThMouseX_Path = Path.Combine(
            Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), Constants.AppName + ".dll");

        [DllImport("kernel32", CharSet = CharSet.Unicode, SetLastError = true)]
        public static extern IntPtr GetModuleHandle([MarshalAs(UnmanagedType.LPWStr)] string lpModuleName);
        [DllImport("kernel32", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        static IntPtr ThMouseX_Module;

        static IntPtr GetFunction(this IntPtr module, string funcName) => GetProcAddress(module, funcName);
        static T GetDelegate<T>(this IntPtr funcPtr) where T : Delegate
            => Marshal.GetDelegateForFunctionPointer(funcPtr, typeof(T)) as T;

        [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public delegate void UninitializeDelegate(bool isProcessTerminating);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void RegisterUninitializeCallbackDelegate(UninitializeDelegate callback, bool isFromDotNet);
        public static RegisterUninitializeCallbackDelegate RegisterUninitializeCallback;

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void SetPositionAddressDelegate(IntPtr address);
        public static SetPositionAddressDelegate SetPositionAddress;

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate PointDataType GetDataTypeDelegate();
        public static GetDataTypeDelegate GetDataType;

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void OpenConsoleDelegate();
        public static OpenConsoleDelegate OpenConsole;

        static public void Initialize()
        {
            ThMouseX_Module = GetModuleHandle(ThMouseX_Path);
            RegisterUninitializeCallback = ThMouseX_Module.GetFunction("Lua_RegisterUninitializeCallback")
                .GetDelegate<RegisterUninitializeCallbackDelegate>();
            SetPositionAddress = ThMouseX_Module.GetFunction("Lua_SetPositionAddress")
               .GetDelegate<SetPositionAddressDelegate>();
            GetDataType = ThMouseX_Module.GetFunction("Lua_GetDataType")
               .GetDelegate<GetDataTypeDelegate>();
            OpenConsole = ThMouseX_Module.GetFunction("Lua_OpenConsole")
               .GetDelegate<OpenConsoleDelegate>();
        }

        static public void Uninitialize(bool isProcessTerminating)
        {
            ThMouseX_Module = IntPtr.Zero;
            RegisterUninitializeCallback = null;
            SetPositionAddress = null;
            GetDataType = null;
            OpenConsole = null;
        }
    }
}
