using System.Diagnostics;
using System.IO.Compression;
using System.Net.Http;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security;
using System.Text;
using Windows.Win32;

namespace PostBuildTool;

using static PInvoke;

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
                case nameof(ImportResource):
                    ImportResource(directiveArgs);
                    break;
                case nameof(GenerateComServerManifest):
                    GenerateComServerManifest(directiveArgs);
                    break;
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

    static void GenerateComServerManifest(string[] args)
    {
        var name = args.ElementAtOrDefault(0);
        var typelib = args.ElementAtOrDefault(1);
        var processorArchitecture = args.ElementAtOrDefault(2);
        var targetPath = args.ElementAtOrDefault(3);
        var manifestPath = args.ElementAtOrDefault(4);
        var assembly = Assembly.ReflectionOnlyLoadFrom(targetPath);
        var assemblyGuid = (string)assembly.GetCustomAttributesData<GuidAttribute>().ConstructorArguments[0].Value;
        var exportTypes = assembly.GetTypes()
            .Where(t => t.IsInterface)
            .Where(t => t.GetCustomAttributesData<ComVisibleAttribute>() != null && t.GetCustomAttributesData<GuidAttribute>() != null)
            .Where(t => Equals(t.GetCustomAttributesData<ComVisibleAttribute>().ConstructorArguments[0].Value, true));
        using var outFile = File.Open(manifestPath, FileMode.Create, FileAccess.Write, FileShare.ReadWrite);
        using var outStream = new StreamWriter(outFile, new UTF8Encoding(true));
        outStream.WriteLine($$"""
            <?xml version="1.0" encoding="utf-8"?>
            <assembly manifestVersion="1.0" xmlns="urn:schemas-microsoft-com:asm.v1">
                <assemblyIdentity version="1.0.0.0" name="{{SecurityElement.Escape(name)}}" type="win32" processorArchitecture="{{SecurityElement.Escape(processorArchitecture)}}"/>

                <trustInfo xmlns="urn:schemas-microsoft-com:asm.v2">
                    <security>
                        <requestedPrivileges xmlns="urn:schemas-microsoft-com:asm.v3">
                            <requestedExecutionLevel level="asInvoker" uiAccess="false" />
                        </requestedPrivileges>
                    </security>
                </trustInfo>

                <file name="{{SecurityElement.Escape(typelib)}}">
                    <typelib
                        tlbid="{{{assemblyGuid}}}"
                        resourceid="1"
                        version="1.0"
                        helpdir="" />
                </file>

            """);
        foreach (var type in exportTypes)
        {
            var guid = type.GetCustomAttributesData<GuidAttribute>().ConstructorArguments[0].Value;
            outStream.WriteLine($$"""
                <comInterfaceExternalProxyStub
                    iid="{{{guid}}}"
                    name="{{type.Name}}"
                    tlbid="{{{assemblyGuid}}}"
                    proxyStubClsid32="{00020424-0000-0000-C000-000000000046}" />

            """);
        }
        outStream.WriteLine("</assembly>");
    }

    unsafe static void ImportResource(string[] args)
    {
        var targetPath = args.ElementAtOrDefault(0);
        var resourcePath = args.ElementAtOrDefault(1);
        using var resourceType = args.ElementAtOrDefault(2).ToResourceType();
        using var resourceId = args.ElementAtOrDefault(3).ToResourceId();
        var data = File.ReadAllBytes(resourcePath);
        if (data.Length == 0)
            throw new InvalidDataException("Resource is empty!");
        var handle = BeginUpdateResource(targetPath, false).ThrowIfError();
        fixed (void* pData = data)
            UpdateResource(handle, resourceType, resourceId, 0, pData, (uint)data.Length).ThrowIfError();
        EndUpdateResource(handle, false).ThrowIfError();
    }

    const string StyluaUrl = "https://github.com/JohnnyMorganz/StyLua/releases/latest/download/stylua-windows-x86_64.zip";
    static void FormatLuaScript(string[] args)
    {
        if (!Environment.Is64BitOperatingSystem)
        {
            Console.WriteLine("This cmdline relies on StyLua which doesn't have a 32-bit executable file, skip Lua script formatting.");
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
            Console.WriteLine($"stylua.exe is not available, download from {StyluaUrl}");
            var tmpPath = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName());
            using var tmpStream = new FileStream(tmpPath, FileMode.Create, FileAccess.ReadWrite, FileShare.Delete);
            File.Delete(tmpPath);
            using var httpClient = new HttpClient();
            using var internetStream = httpClient.GetStreamAsync(StyluaUrl).Result;
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