namespace ThMouseX.DotNet;

public static class Handlers
{
    static public int Initialize(string _)
    {
        try
        {
            LuaApi.Initialize();
            Scripting.Uninitialize();
            Scripting.Initialize(Environment.GetEnvironmentVariable("ThMouseX_ScriptPath"));
            LuaApi.RegisterUninitializeCallback(uninitializeDelegate);
            return 0;
        }
        catch (Exception e)
        {
            Logging.ToFile("[NeoLua] {0}", e);
            Scripting.Uninitialize();
            LuaApi.Uninitialize(false);
            return 1;
        }
    }

    static readonly LuaApi.UninitializeDelegate uninitializeDelegate = Uninitialize;
    static public void Uninitialize(bool isProcessTerminating)
    {
        Scripting.Uninitialize(isProcessTerminating);
        LuaApi.Uninitialize(isProcessTerminating);
    }
}
