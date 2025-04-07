using System.Diagnostics;
using System.IO.Compression;
using System.Net.Http;
using System.Reflection;

namespace PostBuildTool;

static class Program
{
    static int Main(string[] args)
    {
        try
        {
            var directive = args.ElementAtOrDefault(0);
            var directiveArgs = args.Skip(1).ToArray();
            switch (directive)
            {
                case nameof(FormatLuaScript):
                    FormatLuaScript(directiveArgs);
                    break;
                default:
                    throw new InvalidOperationException($"Unknown directive '{directive}'.");
            }
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine(ex);
            return -1;
        }
        return 0;
    }

    static void FormatLuaScript(string[] args)
    {
        if (!Environment.Is64BitOperatingSystem)
        {
            Console.WriteLine("This cmdline relies on StyLua which doesn't have a 32-bit executable file, skip Lua script formatting.");
            return;
        }
        var styluaUrl = Environment.GetEnvironmentVariable("StyluaUrl");
        if (string.IsNullOrWhiteSpace(styluaUrl))
        {
            Console.WriteLine("StyluaUrl is unset, skip Lua script formatting.");
            return;
        }
        var inputPath = args.ElementAtOrDefault(0);
        var outputPath = args.ElementAtOrDefault(1);
        var styluaDirPath = Environment.GetEnvironmentVariable("PostBuildTool_InternalDir");
        if (string.IsNullOrEmpty(styluaDirPath))
            styluaDirPath = Path.GetDirectoryName(Assembly.GetEntryAssembly().Location);
        var styluaPath = Path.Combine(styluaDirPath, "stylua.exe");
        if (!File.Exists(styluaPath))
        {
            Console.WriteLine($"stylua.exe is not available, download from {styluaUrl}");
            var tmpPath = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName());
            using var tmpStream = new FileStream(tmpPath, FileMode.Create, FileAccess.ReadWrite, FileShare.Delete);
            File.Delete(tmpPath);
            using var httpClient = new HttpClient();
            using var internetStream = httpClient.GetStreamAsync(styluaUrl).Result;
            internetStream.CopyTo(tmpStream);
            tmpStream.Seek(0, SeekOrigin.Begin);
            using var tmpZipStream = new ZipArchive(tmpStream);
            tmpZipStream.ExtractToDirectory(styluaDirPath);
        }
        Console.WriteLine($"Format lua script: {outputPath}");
        Process.Start(new ProcessStartInfo(styluaPath, $"--column-width 999 --indent-type Spaces \"{inputPath}\"") {
            UseShellExecute = false,
        }).WaitForExit();
        if (File.Exists(outputPath))
        {
            var newContent = File.ReadAllText(inputPath);
            var oldContent = File.ReadAllText(outputPath);
            if (newContent == oldContent)
            {
                Console.WriteLine($"{outputPath} has no change, keep modified date.");
                return;
            }
        }
        File.Copy(inputPath, outputPath, true);
    }
}