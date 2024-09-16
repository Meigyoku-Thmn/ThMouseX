using Windows.Win32;

namespace ThMouseXServer;

using static Windows.Win32.System.Com.CLSCTX;
using static Windows.Win32.System.Com.REGCLS;

static public class ComHelper
{
    static readonly List<uint> cookies = [];

    public static void CoRegisterClassObject<T>()
    {
        var factory = new ComClassFactory<T>();
        var context = CLSCTX_LOCAL_SERVER;
        var options = REGCLS_MULTIPLEUSE | REGCLS_SUSPENDED;
        PInvoke.CoRegisterClassObject(typeof(T).GUID, factory, context, options, out var cookie).ThrowOnFailure();
        cookies.Add(cookie);
    }

    public static void CoResumeClassObjects()
    {
        PInvoke.CoResumeClassObjects().ThrowOnFailure();
    }
}
