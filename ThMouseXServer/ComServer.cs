#pragma warning disable S4200
using System.Runtime.InteropServices;

[assembly: Guid("2AD4FA86-3961-4F4B-AC10-8AC5716E526A"), ComVisible(false)]

namespace ThMouseXServer;

[Guid("A834A3C5-5355-484D-878D-219311B0E3D9"), ComVisible(true)]
public interface IComServer
{
    bool GetGameConfig(string processName, out GameConfig gameConfig, out CommonConfig commonConfig);
}

[Guid("42F2DB78-D04E-4889-9B27-3F43830A6400"), ComVisible(true)]
public class ComServer : IComServer
{
    [DllImport("ThMouseX.dll",
        EntryPoint = nameof(GetGameConfig), CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    [return: MarshalAs(UnmanagedType.I1)]
    static extern bool _GetGameConfig(string processName, out GameConfig gameConfig, out CommonConfig commonConfig);

    public bool GetGameConfig(string processName, out GameConfig gameConfig, out CommonConfig commonConfig)
        => _GetGameConfig(processName, out gameConfig, out commonConfig);
}