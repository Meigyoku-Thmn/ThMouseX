using System.Runtime.InteropServices;
namespace ThMouseXServer;

[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
[ComVisible(true)]
public struct GameConfig
{
    [MarshalAs(UnmanagedType.LPWStr)]
    public string ProcessName;
    [MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_UI4)]
    public uint[] Address;
    public ScriptType ScriptType;
    public PointDataType PosDataType;
    public FloatPoint BasePixelOffset;
    public uint BaseHeight;
    public FloatPoint AspectRatio;
    public InputMethod InputMethods;
};

[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
[ComVisible(true)]
public struct CommonConfig
{
    public byte VkCodeForLeftClick;
    public byte VkCodeForMiddleClick;
    public byte VkCodeForRightClick;
    public byte VkCodeForXButton1Click;
    public byte VkCodeForXButton2Click;
    public byte VkCodeForScrollUp;
    public byte VkCodeForScrollDown;
    public byte VkCodeForScrollLeft;
    public byte VkCodeForScrollRight;

    public byte ToggleMouseControl;
    public byte ToggleOsCursorButton;
    public byte ToggleImGuiButton;

    [MarshalAs(UnmanagedType.LPWStr)]
    public string TextureFilePath;
    public uint TextureBaseHeight;

    [MarshalAs(UnmanagedType.LPWStr)]
    public string ImGuiFontPath;
    public uint ImGuiBaseFontSize;
    public uint ImGuiBaseVerticalResolution;
}

[ComVisible(true)]
public enum ScriptType : uint
{
    None, LuaJIT, NeoLua, Lua
}

[ComVisible(true)]
public enum PointDataType : uint
{
    None, Int, Float, Short, Double
}

[StructLayout(LayoutKind.Sequential)]
[ComVisible(true)]
public struct IntPoint
{
    public int X;
    public int Y;
}

[StructLayout(LayoutKind.Sequential)]
[ComVisible(true)]
public struct ShortPoint
{
    public short X;
    public short Y;
}

[StructLayout(LayoutKind.Sequential)]
[ComVisible(true)]
public struct FloatPoint
{
    public float X;
    public float Y;
}

[StructLayout(LayoutKind.Sequential)]
[ComVisible(true)]
public struct DoublePoint
{
    public double X;
    public double Y;
}

[ComVisible(true)]
[Flags]
public enum InputMethod : uint
{
    None/*........*/ = 0,
    DirectInput/*.*/ = 1 << 0,
    GetKeyboardState = 1 << 1,
    SendInput/*...*/ = 1 << 2,
    SendMsg/*.....*/ = 1 << 3
}
