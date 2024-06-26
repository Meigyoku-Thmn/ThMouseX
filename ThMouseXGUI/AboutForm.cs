﻿using System;
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
        public static readonly Icon AppIcon = Icon.ExtractAssociatedIcon(Assembly.GetExecutingAssembly().Location);
        public static readonly Image Logo = AppIcon.ToBitmap();
        public AboutForm()
        {
            InitializeComponent();
            Icon = AppIcon;
            imgMouse.Image = Logo;
            Text = string.Format(Text, Program.AppName);
            lblDescription1.Text = string.Format(lblDescription1.Text, Program.AppName);
            lblDescription2.Text = string.Format(lblDescription2.Text, Program.AppName);
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
