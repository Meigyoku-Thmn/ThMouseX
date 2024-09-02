using System.Reflection;
using System.Runtime.InteropServices;
using Windows.Win32;
using Windows.Win32.Foundation;

namespace PostBuildTool;

static class Helper
{
    public static HANDLE ThrowIfError(this HANDLE handle)
    {
        if (handle == HANDLE.Null)
            Marshal.ThrowExceptionForHR(Marshal.GetHRForLastWin32Error());
        return handle;
    }

    public static BOOL ThrowIfError(this BOOL rs)
    {
        if (!rs)
            Marshal.ThrowExceptionForHR(Marshal.GetHRForLastWin32Error());
        return rs;
    }

    unsafe public static CStringLike ToResourceType(this string resTypeStr)
    {
        resTypeStr = resTypeStr.Trim();
        var value = (typeof(PInvoke)
            .GetField(resTypeStr.ToUpper(), BindingFlags.Public | BindingFlags.Static)?
            .GetValue(null));
        value ??= new PCWSTR(null);
        return new CStringLike((PCWSTR)value, resTypeStr);
    }

    unsafe public static CStringLike ToResourceId(this string resIdStr)
    {
        resIdStr = resIdStr.Trim();
        var value = new PCWSTR(null);
        if (resIdStr.StartsWith("#"))
            value = (PCWSTR)((char*)uint.Parse(resIdStr.Substring(1)));
        return new CStringLike(value, resIdStr);
    }

    public static CustomAttributeData GetCustomAttributesData<AttrT>(this Assembly assembly) where AttrT : Attribute
    {
        return assembly.GetCustomAttributesData().FirstOrDefault(e => e.AttributeType == typeof(AttrT));
    }

    public static CustomAttributeData GetCustomAttributesData<AttrT>(this Type assembly) where AttrT : Attribute
    {
        return assembly.GetCustomAttributesData().FirstOrDefault(e => e.AttributeType == typeof(AttrT));
    }
}
