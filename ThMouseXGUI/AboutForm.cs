using System.Reflection;

namespace ThMouseX;

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
        var versionStr = Assembly.GetEntryAssembly().GetName().Version.ToString(3);
        lblDescription1.Text = string.Format(lblDescription1.Text, Program.AppName, versionStr);
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
