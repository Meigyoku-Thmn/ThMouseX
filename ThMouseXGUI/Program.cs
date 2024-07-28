using System.Reflection;
using System.Runtime.InteropServices;

namespace ThMouseX;

static class Program
{
    public const string AppName = "ThMouseX";
    public const string DllName = AppName + ".dll";
    public static readonly string RootDir = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
    public static readonly string LogPath = Path.Combine(RootDir, "log.txt");

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    static extern bool MarkThMouseXProcess();
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    static extern bool InstallHooks();
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    static extern void RemoveHooks();
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    static extern bool ReadGamesFile();
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    static extern bool ReadGeneralConfigFile();

    [STAThread]
    static void Main()
    {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);

        if (!MarkThMouseXProcess())
            Environment.Exit(1);

        using var mutex = new Mutex(true, AppName, out var mutexIsCreated);
        if (!mutexIsCreated)
        {
            MessageBox.Show($"{AppName} is already running.", AppName, MessageBoxButtons.OK, MessageBoxIcon.Information);
            Environment.Exit(1);
        }

        if (!ReadGamesFile() || !ReadGeneralConfigFile() || !InstallHooks())
            Environment.Exit(1);

        Application.Run(new ThMouseApplicationContext());

        RemoveHooks();
    }
}
