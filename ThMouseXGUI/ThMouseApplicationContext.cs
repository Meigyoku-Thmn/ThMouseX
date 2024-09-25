using System.Diagnostics;

namespace ThMouseXGUI;

class ThMouseApplicationContext : ApplicationContext
{
    readonly NotifyIcon notifyIcon = new();
    public bool RestartFlag { get; private set; }

    public ThMouseApplicationContext()
    {
        notifyIcon.Visible = true;
        notifyIcon.Icon = AboutForm.AppIcon;
        notifyIcon.DoubleClick += ShowDialog;
        notifyIcon.ContextMenu = new ContextMenu([
            new MenuItem("About", ShowDialog),
            new MenuItem("Open ThMouseX's Directory", OpenInstallDir),
            new MenuItem("Open Log", OpenLog),
            new MenuItem("Restart", Restart),
            new MenuItem("Exit", Exit),
        ]);
        notifyIcon.ShowBalloonTip(2000, Program.AppName, $"{Program.AppName} is activated.", ToolTipIcon.Info);
    }

    AboutForm aboutForm;

    void ShowDialog(object sender, EventArgs e)
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
            Exit(sender, e);
    }

    void OpenInstallDir(object sender, EventArgs e)
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

    void OpenLog(object sender, EventArgs e)
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

    void Restart(object sender, EventArgs e)
    {
        RestartFlag = true;
        Exit(sender, e);
    }

    void Exit(object sender, EventArgs e)
    {
        notifyIcon.Visible = false;
        Application.Exit();
    }
}
