using System.Runtime.InteropServices;
using Windows.Win32.Foundation;

namespace PostBuildTool;

unsafe class CStringLike(PCWSTR value1, string value2) : IDisposable
{
    GCHandle value2Handle = GCHandle.Alloc(value2, GCHandleType.Pinned);

    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    public PCWSTR Value => value1.Value != null ? value1 : (char*)value2Handle.AddrOfPinnedObject();

    public static implicit operator PCWSTR(CStringLike value) => value.Value;

    protected virtual void Dispose(bool disposing)
    {
        value2Handle.Free();
    }
}
