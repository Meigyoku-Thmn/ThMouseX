#pragma warning disable S4200
using System.Runtime.InteropServices;

#if WIN32
[assembly: Guid("2AD4FA86-3961-4F4B-AC10-8AC5716E526A"), ComVisible(false)]
#else
[assembly: Guid("A4F44FF8-CB06-4567-AFD1-83C3CB4FA38C"), ComVisible(false)]
#endif

namespace ThMouseXServer;

#if WIN32
[Guid("A834A3C5-5355-484D-878D-219311B0E3D9"), ComVisible(true)]
#else
[Guid("F39B2500-A7A1-4FC0-BE81-379951F7FBB7"), ComVisible(true)]
#endif
public interface IComServer
{
    bool GetGameConfig(string processName, out GameConfig gameConfig, out CommonConfig commonConfig);
}

#if WIN32
[Guid("42F2DB78-D04E-4889-9B27-3F43830A6400"), ComVisible(true)]
#else
[Guid("58EBED2B-95A8-4AEA-8364-00EFB771A7AC"), ComVisible(true)]
#endif
public class ComServer : IComServer
{
#if (WIN32)
    const string ClientFileName = "ThMouseX.dll";
#else
    const string ClientFileName = "ThMouseX.64.dll";
#endif

    [DllImport(ClientFileName,
        EntryPoint = nameof(GetGameConfig), CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    [return: MarshalAs(UnmanagedType.I1)]
    static extern bool _GetGameConfig(string processName, out GameConfig gameConfig, out CommonConfig commonConfig);

    public bool GetGameConfig(string processName, out GameConfig gameConfig, out CommonConfig commonConfig)
        => _GetGameConfig(processName, out gameConfig, out commonConfig);
}