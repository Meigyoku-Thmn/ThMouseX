using System.Diagnostics;
using System.Reflection;
using System.Text;

namespace ThMouseXGUI;

static class Logging
{
    static readonly string LogPath = Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), "log.txt");
    static StreamWriter _file;
    static StreamWriter File {
        get {
            if (_file == null)
            {
                var baseStream = new FileStream(LogPath, FileMode.Append, FileAccess.Write, FileShare.ReadWrite, 1);
                _file = new StreamWriter(baseStream, new UTF8Encoding(false), 1) {
                    AutoFlush = true,
                    NewLine = "\n"
                };
            }
            return _file;
        }
    }
    public static void ToFile(string format, params object[] args)
    {
        File.Write("[{0} {1:dd/MM/yyyy HH:mm:ss}] ", Process.GetCurrentProcess().ProcessName, DateTime.Now);
        File.WriteLine(format, args);
    }
    public static void Close()
    {
        File.Close();
        _file = null;
    }
}