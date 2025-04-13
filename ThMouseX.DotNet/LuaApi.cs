#pragma warning disable S4200
using System.Runtime.InteropServices;

namespace ThMouseX.DotNet;

internal static class LuaApi
{
    [DllImport("kernel32", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern IntPtr GetModuleHandle([MarshalAs(UnmanagedType.LPWStr)] string lpModuleName);
    [DllImport("kernel32", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
    static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

    static IntPtr GetFunction(this IntPtr module, string funcName) => GetProcAddress(module, funcName);
    static T GetDelegate<T>(this IntPtr funcPtr) where T : Delegate
        => Marshal.GetDelegateForFunctionPointer(funcPtr, typeof(T)) as T;

    [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    public delegate void UninitializeDelegate(bool isProcessTerminating);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void RegisterUninitializeCallbackDelegate(UninitializeDelegate callback);
    public static RegisterUninitializeCallbackDelegate RegisterUninitializeCallback { get; private set; }

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void SetPositionAddressDelegate(IntPtr address);
    public static SetPositionAddressDelegate SetPositionAddress { get; private set; }

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate PointDataType GetDataTypeDelegate();
    public static GetDataTypeDelegate GetDataType { get; private set; }

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void OpenConsoleDelegate();
    public static OpenConsoleDelegate OpenConsole { get; private set; }

    static public void Initialize()
    {
        var envValue = Environment.GetEnvironmentVariable("ThMouseX_ModuleHandle");
        var ThMouseX_ModuleHandle = Environment.Is64BitProcess
            ? new IntPtr((long)ulong.Parse(envValue))
            : new IntPtr((int)uint.Parse(envValue));
        RegisterUninitializeCallback = ThMouseX_ModuleHandle.GetFunction("Lua_RegisterUninitializeCallback")
            .GetDelegate<RegisterUninitializeCallbackDelegate>();
        SetPositionAddress = ThMouseX_ModuleHandle.GetFunction("Lua_SetPositionAddress")
            .GetDelegate<SetPositionAddressDelegate>();
        GetDataType = ThMouseX_ModuleHandle.GetFunction("Lua_GetDataType")
            .GetDelegate<GetDataTypeDelegate>();
        OpenConsole = ThMouseX_ModuleHandle.GetFunction("Lua_OpenConsole")
            .GetDelegate<OpenConsoleDelegate>();
    }

    static public void Uninitialize(bool isProcessTerminating)
    {
        if (isProcessTerminating)
            return;
        RegisterUninitializeCallback = null;
        SetPositionAddress = null;
        GetDataType = null;
        OpenConsole = null;
    }
}
