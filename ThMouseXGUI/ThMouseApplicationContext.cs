using System.Windows.Forms;

namespace ThMouseX
{
    class ThMouseApplicationContext : ApplicationContext
    {
        readonly NotifyIcon notifyIcon = new NotifyIcon();

        public ThMouseApplicationContext()
        {
            notifyIcon.Visible = true;
            notifyIcon.Icon = AboutForm.AppIcon;
            notifyIcon.DoubleClick += (_, __) => ShowDialog();
            notifyIcon.ContextMenu = new ContextMenu(new[] {
                new MenuItem("About", (_, __) => ShowDialog()),
                new MenuItem("Exit", (_, __) => Exit()),
            });
            notifyIcon.ShowBalloonTip(2000, Program.AppName, $"{Program.AppName} is activated.", ToolTipIcon.Info);
        }

        void ShowDialog()
        {
            if (new AboutForm().ShowDialog() == DialogResult.Abort)
                Exit();
        }

        void Exit()
        {
            notifyIcon.Visible = false;
            Application.Exit();
        }
    }
}
