using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;
using ThMouseXServer;

[assembly: Guid("2AD4FA86-3961-4F4B-AC10-8AC5716E526A"), ComVisible(false)]

namespace ThMouseXGUI;

using static ComHelper;

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
    static int Main()
    {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);

        var thisAssembly = Assembly.GetExecutingAssembly();
        var expectedName = thisAssembly.GetName().Name;
        var actualName = Path.GetFileNameWithoutExtension(thisAssembly.Location);
        if (expectedName != actualName)
        {
            MessageBox.Show($"The file name of this executable file must be '{expectedName}.exe'!", AppName, MessageBoxButtons.OK, MessageBoxIcon.Error);
            return 1;
        }

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

            try
            {
                CoRegisterClassObject<ComServer>();
                CoResumeClassObjects();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, AppName + ": Failed to initialize Component Object Models", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return 1;
            }

            if (!InstallHooks())
                return 1;

            var applicationContext = new ThMouseApplicationContext();
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
