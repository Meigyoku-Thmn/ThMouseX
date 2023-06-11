using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ThMouseX
{
    public partial class AboutForm : Form
    {
        public const string AppName = "ThMouseX";
        public static readonly Icon AppIcon = Icon.ExtractAssociatedIcon(Assembly.GetExecutingAssembly().Location);
        public static readonly Image Logo = AppIcon.ToBitmap();
        public AboutForm()
        {
            InitializeComponent();
            Icon = AppIcon;
            imgMouse.Image = Logo;
        }

        private void AboutForm_Shown(object sender, EventArgs e)
        {
            ShowIcon = false;
        }

        private void cmdOK_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void cmdQuit_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.Abort;
            Close();
        }
    }
}
