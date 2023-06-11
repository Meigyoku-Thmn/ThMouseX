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
        const string ThMCore = "ThMCore.dll";
        [DllImport(ThMCore, CallingConvention = CallingConvention.Cdecl)]
        static extern bool TestRegisteredWindowMessages();
        [DllImport(ThMCore, CallingConvention = CallingConvention.Cdecl)]
        static extern bool InstallHooks();
        [DllImport(ThMCore, CallingConvention = CallingConvention.Cdecl)]
        static extern void RemoveHooks();
        [DllImport(ThMCore, CallingConvention = CallingConvention.Cdecl)]
        static extern bool PopulateMethodRVAs();
        [DllImport(ThMCore, CallingConvention = CallingConvention.Cdecl)]
        static extern bool ReadGamesFile();
        [DllImport(ThMCore, CallingConvention = CallingConvention.Cdecl)]
        static extern bool ReadGeneralConfigFile();

        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            using var mutex = new Mutex(true, "ThMouseX", out var mutexIsCreated);
            if (!mutexIsCreated)
            {
                MessageBox.Show("ThMouseX is already running.", "ThMouseX", MessageBoxButtons.OK, MessageBoxIcon.Information);
                Environment.Exit(1);
            }

            if (!TestRegisteredWindowMessages())
                Environment.Exit(1);

            if (!PopulateMethodRVAs())
                Environment.Exit(1);

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
