using System.Diagnostics;

namespace ThMouseXGUI;

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

    AboutForm aboutForm;

    void ShowDialog()
    {
        if (aboutForm != null)
        {
            aboutForm.Focus();
            return;
        }
        aboutForm = new AboutForm();
        var rs = aboutForm.ShowDialog();
        aboutForm = null;
        if (rs == DialogResult.Abort)
            Exit();
    }

    void OpenInstallDir()
    {
        try
        {
            Process.Start(new ProcessStartInfo { UseShellExecute = true, FileName = Program.RootDir });
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
            Process.Start(new ProcessStartInfo { UseShellExecute = true, FileName = Program.LogPath });
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
