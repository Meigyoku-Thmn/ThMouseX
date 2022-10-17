using HarmonyLib;
using Neo.IronLua;
using Sigil;
using Sigil.NonGeneric;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;

namespace NeoLuaBootstrap
{
    using DelegateMap = Dictionary<string, (Delegate Delegate, GCHandle Handle)>;
    public static class Handlers
    {
        [DllImport("Common", CallingConvention = CallingConvention.Cdecl)]
        static extern void SetOnClose(IntPtr address);

        static readonly DelegateMap EventDelegates = new DelegateMap();

        static Handlers()
        {
            var onCloseDelegate = new OnClose(OnClose_Impl);
            var gcOnCloseHandle = GCHandle.Alloc(onCloseDelegate);
            EventDelegates.Add(nameof(OnClose), (onCloseDelegate, gcOnCloseHandle));
        }

        static public int OnInit(string scriptPath)
        {
            Scripting.Patch(scriptPath);
            SetOnClose(Marshal.GetFunctionPointerForDelegate(EventDelegates[nameof(OnClose)].Delegate));
            return 0;
        }

        [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        delegate void OnClose();
        static public void OnClose_Impl()
        {
            Scripting.Unpatch();
        }
    }

    static class Logging
    {
        static readonly string LogPath = Path.Combine(
            Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), "log.txt");
        static StreamWriter _log;
        public static StreamWriter Log {
            get {
                if (_log == null) _log = new StreamWriter(new FileStream(
                        LogPath, FileMode.Append, FileAccess.Write, FileShare.ReadWrite, 1), Encoding.UTF8, 1) {
                    AutoFlush = true
                };
                return _log;
            }
        }
    }

    static class Scripting
    {
        [StructLayout(LayoutKind.Sequential)]
        public class Position<T>
        {
            public T X;
            public T Y;
        }

        [DllImport("Common", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetPositionAddress(IntPtr address);

        [DllImport("Common", CallingConvention = CallingConvention.Cdecl)]
        public static extern PointDataType GetDataType();

        static readonly List<Delegate> DelegateStore = new List<Delegate>();
        static readonly FieldInfo DelegateStoreField = AccessTools.Field(typeof(Scripting), nameof(DelegateStore));
        static readonly MethodInfo getItemMethod = DelegateStore.GetType().GetMethod("get_Item");

        public enum PointDataType
        {
            Int, Float, Short
        };
        static readonly PointDataType DataType = GetDataType();
        public static readonly object Pos = DataType == PointDataType.Int ? new Position<int>() :
                                            DataType == PointDataType.Float ? new Position<float>() :
                                            DataType == PointDataType.Short ? new Position<short>() : default(object);
        static readonly GCHandle PosHandle;

        static Scripting()
        {
            if (Pos != null)
                PosHandle = GCHandle.Alloc(Pos, GCHandleType.Pinned);
        }

        static void LoadDelegate(this Emit emitter, Delegate dele)
        {
            DelegateStore.Add(dele);
            emitter.LoadField(DelegateStoreField);
            emitter.LoadConstant(DelegateStore.Count - 1);
            emitter.Call(getItemMethod);
            emitter.CastClass(DelegateStore.Last().GetType());
        }

        static DynamicMethod MakeLuaFuncionWrapper(Delegate luaFunc)
        {
            if (luaFunc == null)
                return null;
            var methodInfo = luaFunc.GetMethodInfo();
            var paramInfos = methodInfo.GetParameters()
                .Where((e, i) => i != 0 || e.ParameterType != typeof(Closure)).ToArray();
            var paramTypes = paramInfos.Select(e => e.ParameterType).ToArray();
            var retType = methodInfo.ReturnType;
#if DEBUG
            var emitter = Emit.NewDynamicMethod(retType, paramTypes, methodInfo.Name, doVerify: true);
#else
            var emitter = Emit.NewDynamicMethod(retType, paramTypes, methodInfo.Name, doVerify: false);
#endif
            emitter.LoadDelegate(luaFunc);
            for (ushort i = 0; i < paramTypes.Length; i++)
                emitter.LoadArgument(i);
            emitter.Call(luaFunc.GetType().GetMethod(nameof(Action.Invoke)));
            emitter.Return();

            var emitterAccessor = Traverse.Create(emitter);
            var module = emitterAccessor.Field("Module").GetValue() as Module;
            var dynaMethod = new DynamicMethod(methodInfo.Name, retType, paramTypes, module, skipVisibility: true);
            for (var i = 0; i < paramInfos.Length; i++)
                dynaMethod.DefineParameter(i + 1, paramInfos[i].Attributes, paramInfos[i].Name);
            emitterAccessor.Field("InnerEmit").Property("DynMethod").SetValue(dynaMethod);

            var outDelegateType = Expression.GetDelegateType(paramTypes.Append(retType).ToArray());
            emitter.CreateDelegate(outDelegateType, OptimizationOptions.All);

            return dynaMethod;
        }

        static readonly Dictionary<MethodBase, DynamicMethod> PrefixStore = new Dictionary<MethodBase, DynamicMethod>();
        static MethodInfo GetPrefix(MethodBase method) => PrefixStore[method];
        static readonly MethodInfo GetPrefixMethod = AccessTools.Method(typeof(Scripting), nameof(GetPrefix));

        static readonly Dictionary<MethodBase, DynamicMethod> PostfixStore = new Dictionary<MethodBase, DynamicMethod>();
        static MethodInfo GetPostfix(MethodBase method) => PostfixStore[method];
        static readonly MethodInfo GetPostfixMethod = AccessTools.Method(typeof(Scripting), nameof(GetPostfix));

        static Lua L;

        const string HarmonyId = "ThMouseX NeoLua Scripting";
        static readonly Harmony HarmonyInst = new Harmony(HarmonyId);
        static public void Unpatch()
        {
            SetPositionAddress(IntPtr.Zero);
            HarmonyInst.UnpatchAll(HarmonyId);
            PrefixStore.Clear();
            PostfixStore.Clear();
            DelegateStore.Clear();
            L?.Dispose();
            L = null;
        }

        const string InitialScript = @"
            const _Traverse typeof HarmonyLib.Traverse
            Traverse = _Traverse
            const Scripting typpeof NeoLuaBootstrap.Scripting
            Pos = Scripting.Pos
        ";

        static public void Patch(string scriptPath)
        {
            try
            {
                if (Pos == null)
                    return;
                SetPositionAddress(PosHandle.AddrOfPinnedObject());

                L = new Lua();
                var g = L.CreateEnvironment();

                g.DoChunk(InitialScript, nameof(InitialScript));

                var chunk = L.CompileChunk(scriptPath, new LuaCompileOptions {
                    DebugEngine = LuaStackTraceDebugger.Default,
                });

                var table = g.DoChunk(chunk).Values.FirstOrDefault() as LuaTable;
                if (table == null)
                    return;

                foreach (var (item, i) in table.Select((e, i) => (e, i)))
                {
                    var config = item.Value as LuaTable;
                    if (config == null)
                        continue;

                    var targetMethodPath = config[1] as string;
                    if (targetMethodPath == null)
                    {
                        Logging.Log.WriteLine($"[NeoLua] Invalid target method path in hook configuration (Ordinal: {i + 1}).");
                        continue;
                    }

                    var prefixHook = config[2] as Delegate;
                    var postfixHook = config[3] as Delegate;
                    if (prefixHook == null && postfixHook == null)
                    {
                        Logging.Log.WriteLine($"[NeoLua] All hook functions are invalid in hook configuration (Ordinal: {i + 1}).");
                        continue;
                    }

                    var original = AccessTools.Method(targetMethodPath);
                    if (original == null)
                    {
                        Logging.Log.WriteLine($"[NeoLua] Failed to get method ${targetMethodPath}.");
                        continue;
                    }

                    var prefixHookWrapper = MakeLuaFuncionWrapper(prefixHook);
                    if (prefixHookWrapper != null)
                        PrefixStore.Add(original, prefixHookWrapper);
                    var prefix = prefixHookWrapper != null ? new HarmonyMethod(GetPrefixMethod) : null;

                    var postfixHookWrapper = MakeLuaFuncionWrapper(postfixHook);
                    if (postfixHookWrapper != null)
                        PostfixStore.Add(original, postfixHookWrapper);
                    var postfix = postfixHookWrapper != null ? new HarmonyMethod(GetPostfixMethod) : null;

                    HarmonyInst.Patch(original, prefix, postfix);
                }
            }
            catch (LuaRuntimeException e)
            {
                Logging.Log.WriteLine("[NeoLua] " + LuaExceptionData.GetData(e).FormatStackTrace(0, false));
                Logging.Log.WriteLine("[NeoLua] " + e);
                Unpatch();
            }
            catch (Exception e) when (e is LuaParseException || e is Exception)
            {
                Logging.Log.WriteLine("[NeoLua] " + e);
                Unpatch();
            }
        }
    }
}
