#pragma warning disable S2743
using System.Runtime.InteropServices;

namespace ThMouseXServer;

[ComImport, Guid("00000001-0000-0000-C000-000000000046"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
internal interface IClassFactory
{
    [PreserveSig]
    int CreateInstance([MarshalAs(UnmanagedType.Interface)] object pUnkOuter, [MarshalAs(UnmanagedType.LPStruct)] Guid riid, out IntPtr ppvObject);

    [PreserveSig]
    int LockServer(bool fLock);
}

public class ComClassFactory<T> : IClassFactory
{
    const int CLASS_E_NOAGGREGATION = unchecked((int)0x80040110);
    const int E_NOINTERFACE = unchecked((int)0x80004002);
    static Guid IID_IUnknown = new("00000000-0000-0000-C000-000000000046");

    int IClassFactory.LockServer(bool fLock) => 0;
    int IClassFactory.CreateInstance(object pUnkOuter, Guid riid, out IntPtr ppvObject)
    {
        var hr = GetValidatedInterfaceType(typeof(T), riid, pUnkOuter, out var interfaceType);
        if (hr != 0)
        {
            ppvObject = IntPtr.Zero;
            return hr;
        }
        var obj = Activator.CreateInstance(typeof(T));
        if (pUnkOuter != null)
            obj = CreateAggregatedObject(pUnkOuter, obj!);
        return GetObjectAsInterface(obj!, interfaceType!, out ppvObject);
    }

    private static int GetValidatedInterfaceType(Type classType, Guid riid, object outer, out Type? type)
    {
        if (riid == IID_IUnknown)
        {
            type = typeof(object);
            return 0;
        }
        if (outer != null)
        {
            type = null;
            return CLASS_E_NOAGGREGATION;
        }
        foreach (var iface in classType.GetInterfaces())
        {
            if (iface.GUID != riid)
                continue;
            type = iface;
            return 0;
        }
        type = null;
        return E_NOINTERFACE;
    }

    private static int GetObjectAsInterface(object obj, Type interfaceType, out IntPtr interfacePtr)
    {
        if (interfaceType == typeof(object))
        {
            interfacePtr = Marshal.GetIUnknownForObject(obj);
            return 0;
        }
        interfacePtr = Marshal.GetComInterfaceForObject(obj, interfaceType, CustomQueryInterfaceMode.Ignore);
        return interfacePtr == IntPtr.Zero ? E_NOINTERFACE : 0;
    }

    private static object CreateAggregatedObject(object unkOuter, object comObject)
    {
        var outerPtr = Marshal.GetIUnknownForObject(unkOuter);
        try
        {
            return Marshal.GetObjectForIUnknown(Marshal.CreateAggregatedObject(outerPtr, comObject));
        }
        finally
        {
            Marshal.Release(outerPtr);
        }
    }
}