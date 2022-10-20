using System;
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
        static StreamWriter File {
            get {
                if (_file == null)
                {
                    _file = new StreamWriter(
                        new FileStream(LogPath, FileMode.Append, FileAccess.Write, FileShare.ReadWrite, 1),
                        new UTF8Encoding(false), 1) {
                        AutoFlush = true,
                        NewLine = "\n"
                    };
                }
                return _file;
            }
        }
        public static void ToFile(string format, params object[] args)
        {
            File.Write("[{0:dd/MM/yyyy HH:mm:ss}] ", DateTime.Now);
            File.WriteLine(format, args);
        }
    }
}
