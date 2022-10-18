using System.IO;
using System.Reflection;
using System.Text;

namespace NeoLuaBootstrap
{
    static class Logging
    {
        static readonly string LogPath = Path.Combine(
            Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), "log.txt");
        static StreamWriter _file;
        public static StreamWriter File {
            get {
                if (_file == null)
                {
                    _file = new StreamWriter(
                        new FileStream(LogPath, FileMode.Append, FileAccess.Write, FileShare.ReadWrite, 1), Encoding.UTF8, 1);
                    _file.AutoFlush = true;
                }
                return _file;
            }
        }
    }
}
