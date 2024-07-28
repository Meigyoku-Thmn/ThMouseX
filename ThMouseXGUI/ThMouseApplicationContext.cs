using System.Diagnostics;
using System.Reflection;

namespace ThMouseX;

class ThMouseApplicationContext : ApplicationContext
{
    readonly NotifyIcon notifyIcon = new();

    public ThMouseApplicationContext()
    {
        notifyIcon.Visible = true;
        notifyIcon.Icon = AboutForm.AppIcon;
        notifyIcon.DoubleClick += (_, __) => ShowDialog();
        notifyIcon.ContextMenu = new ContextMenu([
            new MenuItem("About", (_, __) => ShowDialog()),
            new MenuItem("Open ThMouseX's Directory", (_, __) => OpenInstallDir()),
            new MenuItem("Open Log", (_, __) => OpenLog()),
            new MenuItem("Exit", (_, __) => Exit()),
        ]);
        notifyIcon.ShowBalloonTip(2000, Program.AppName, $"{Program.AppName} is activated.", ToolTipIcon.Info);
    }

    void ShowDialog()
    {
        if (new AboutForm().ShowDialog() == DialogResult.Abort)
            Exit();
    }

    void OpenInstallDir()
    {
        try
        {
            new Process { StartInfo = new() { UseShellExecute = true, FileName = Program.RootDir } }.Start();
        }
        catch (Exception ex)
        {
            MessageBox.Show(ex.Message, "Error opening ThMouseX's directory", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
    }

    void OpenLog()
    {
        try
        {
            new FileStream(Program.LogPath, FileMode.Append, FileAccess.Write, FileShare.ReadWrite, 1).Dispose();
            new Process { StartInfo = new() { UseShellExecute = true, FileName = Program.LogPath } }.Start();
        }
        catch (Exception ex)
        {
            MessageBox.Show(ex.Message, "Error opening log file", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
    }

    void Exit()
    {
        notifyIcon.Visible = false;
        Application.Exit();
    }
}
