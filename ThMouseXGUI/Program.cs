using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;

namespace ThMouseXGUI;

static class Program
{
    public const string AppName = "ThMouseX";
#if (WIN32)
    public const string DllName = AppName + ".dll";
#else
    public const string DllName = AppName + ".64.dll";
#endif
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
    static int Main()
    {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);

        if (!MarkThMouseXProcess())
            return 1;

        var restartFlag = false;
        using (var mutex = new Mutex(true, AppName, out var mutexIsCreated))
        {
            if (!mutexIsCreated)
            {
                MessageBox.Show($"{AppName} is already running.", AppName, MessageBoxButtons.OK, MessageBoxIcon.Information);
                return 1;
            }

            if (!ReadGamesFile() || !ReadGeneralConfigFile())
                return 1;

            var applicationContext = new ThMouseApplicationContext();

            if (!InstallHooks())
                return 1;

            Application.Run(applicationContext);
            restartFlag = applicationContext.RestartFlag;
            RemoveHooks();
        }
        if (restartFlag)
        {
            Process.Start(Application.ExecutablePath);
        }
        return 0;
    }
}
