using System.Runtime.InteropServices;
using Windows.Win32;
using Windows.Win32.Foundation;
using Windows.Win32.System.DataExchange;
using Windows.Win32.UI.WindowsAndMessaging;

namespace ThMouseXGUI;

using static PInvoke;
using static SEND_MESSAGE_TIMEOUT_FLAGS;

unsafe public class ServerForm : NativeWindow
{
    [DllImport(Program.DllName, CallingConvention = CallingConvention.Cdecl)]
    static extern nuint GetMemBlockSize(void* address);
    [DllImport(Program.DllName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    static extern bool GetGameConfig(void* processName, out IntPtr commonConfig, out uint commonConfigSize, out IntPtr gameConfig, out uint gameConfigSize);
    [DllImport("kernel32.dll")]
    static extern void CopyMemory(IntPtr destination, IntPtr source, nuint length);

    const int Timeout = 1000 * 60 * 10;

    const string ServerWindowName = "{BC9D7991-41A0-47F3-BC4D-6CFF2BA205FD}";
    static readonly HWND HWND_MESSAGE = new(new IntPtr(-3));

    const int GET_CONFIG_EVENT = 1;
    const int GET_CONFIG_EVENT_RETURN = 2;
    const int GET_MEM_BLOCK_EVENT_RETURN = 3;

    const uint GET_MEM_BLOCK_MSG = WM_APP + 1;

    protected override void WndProc(ref Message m)
    {
        switch ((uint)m.Msg)
        {
            case WM_COPYDATA:
                {
                    var data = (COPYDATASTRUCT*)m.LParam;
                    if (data->dwData == GET_CONFIG_EVENT)
                    {
                        m.Result = OnGetGameConfig(new HWND(m.HWnd), new HWND(m.WParam), data);
                        return;
                    }
                    break;
                }
            case GET_MEM_BLOCK_MSG:
                {
                    m.Result = OnGetMemBlock(new HWND(m.HWnd), new HWND(m.WParam), m.LParam);
                    return;
                }
        }
        base.WndProc(ref m);
    }

    IntPtr OnGetGameConfig(HWND destHwnd, HWND sourceHwnd, COPYDATASTRUCT* data)
    {
        var rs = GetGameConfig(data->lpData, out var commonConfigPtr, out var commonConfigSize, out var gameConfigPtr, out var gameConfigSize);
        if (!rs)
            return IntPtr.Zero;

        var mem = Marshal.AllocHGlobal(new IntPtr(commonConfigSize + gameConfigSize));
        using var _ = new Defer(() => Marshal.FreeHGlobal(mem));
        CopyMemory(mem, commonConfigPtr, commonConfigSize);
        CopyMemory(IntPtr.Add(mem, (int)commonConfigSize), gameConfigPtr, gameConfigSize);

        var _data = new COPYDATASTRUCT {
            dwData = GET_CONFIG_EVENT_RETURN,
            cbData = commonConfigSize + gameConfigSize,
            lpData = mem.ToPointer(),
        };
        var result = SendMessageTimeout(sourceHwnd, WM_COPYDATA, new WPARAM((nuint)destHwnd.Value), new LPARAM((nint)(void*)&_data), SMTO_NOTIMEOUTIFNOTHUNG | SMTO_ERRORONEXIT, Timeout);
        if (result == 0)
        {
            Logging.ToFile("Failed to send game config to the client window.");
            return IntPtr.Zero;
        }
        return new IntPtr(1);
    }

    IntPtr OnGetMemBlock(HWND destHwnd, HWND sourceHwnd, IntPtr address)
    {
        var size = GetMemBlockSize(address.ToPointer());
        if (size == 0)
            return IntPtr.Zero;

        var data = new COPYDATASTRUCT {
            dwData = GET_MEM_BLOCK_EVENT_RETURN,
            cbData = (uint)size,
            lpData = address.ToPointer(),
        };
        var result = SendMessageTimeout(sourceHwnd, WM_COPYDATA, new WPARAM((nuint)destHwnd.Value), new LPARAM((nint)(void*)&data), SMTO_NOTIMEOUTIFNOTHUNG | SMTO_ERRORONEXIT, Timeout);
        if (result == 0)
        {
            Logging.ToFile("Failed to send game config to the client window.");
            return IntPtr.Zero;
        }
        return new IntPtr(1);
    }

    public bool CreateWindow()
    {
        if (Handle == IntPtr.Zero) CreateHandle(new() {
            Style = 0,
            ExStyle = 0,
            ClassStyle = 0,
            Caption = ServerWindowName,
            Parent = (IntPtr)HWND_MESSAGE
        });
        return Handle != IntPtr.Zero;
    }

    public void DestroyWindow() => DestroyWindow(true, HWND.Null);

    public override void DestroyHandle()
    {
        DestroyWindow(false, HWND.Null);
        base.DestroyHandle();
    }

    void DestroyWindow(bool forReal, HWND dispatchingHwnd)
    {
        if (dispatchingHwnd == HWND.Null)
            dispatchingHwnd = new HWND(Handle);
        var isCalledFromDifferentThread =
            dispatchingHwnd != IntPtr.Zero &&
            GetWindowThreadProcessId(dispatchingHwnd) != GetCurrentThreadId();
        if (isCalledFromDifferentThread)
        {
            PostMessage(dispatchingHwnd, WM_CLOSE, 0, 0);
            return;
        }
        if (forReal)
            base.DestroyHandle();
    }
}
