using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ThMouseX
{
    static class Program
    {
        public const string AppName = "ThMouseX";
        public const string DllName = AppName + ".dll";

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

            using var mutex = new Mutex(true, AppName, out var mutexIsCreated);
            if (!mutexIsCreated)
            {
                MessageBox.Show($"{AppName} is already running.", AppName, MessageBoxButtons.OK, MessageBoxIcon.Information);
                Environment.Exit(1);
            }

            if (!ReadGamesFile())
                Environment.Exit(1);

            if (!ReadGeneralConfigFile())
                Environment.Exit(1);

            if (!InstallHooks())
                Environment.Exit(1);

            Application.Run(new ThMouseApplicationContext());

            RemoveHooks();
        }
    }
}
