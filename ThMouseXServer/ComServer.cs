using System.Runtime.InteropServices;

[assembly: Guid("2AD4FA86-3961-4F4B-AC10-8AC5716E526A"), ComVisible(false)]

namespace ThMouseXServer;

[Guid("A834A3C5-5355-484D-878D-219311B0E3D9"), ComVisible(true)]
public interface IComServer
{
    bool GetGameConfig(string processName, out GameConfig gameConfig);
}

[Guid("42F2DB78-D04E-4889-9B27-3F43830A6400"), ComVisible(true)]
public class ComServer : IComServer
{
    [DllImport("ThMouseX.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    static extern ref GameConfig? GetGameConfig(string processName);

    public bool GetGameConfig(string processName, out GameConfig gameConfig)
    {
        var rs = GetGameConfig(processName);
        if (rs == null)
        {
            gameConfig = default;
            return false;
        }
        gameConfig = rs.Value;
        return true;
    }
}