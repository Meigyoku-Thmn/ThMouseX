using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
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
            notifyIcon.ShowBalloonTip(2000, AboutForm.AppName, $"{AboutForm.AppName} is activated.", ToolTipIcon.Info);
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
