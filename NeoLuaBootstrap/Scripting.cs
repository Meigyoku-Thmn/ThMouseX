using HarmonyLib;
using Neo.IronLua;
using Sigil;
using Sigil.NonGeneric;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace NeoLuaBootstrap
{
    static class Scripting
    {
        static readonly List<Delegate> DelegateStore = new List<Delegate>();
        static readonly FieldInfo DelegateStoreField = AccessTools.Field(typeof(Scripting), nameof(DelegateStore));
        static readonly MethodInfo getItemMethod = DelegateStore.GetType().GetMethod("get_Item");

        static readonly PointDataType DataType = LuaApi.GetDataType();
        public static readonly object Pos = DataType == PointDataType.Int ? new Position<int>() :
                                            DataType == PointDataType.Float ? new Position<float>() :
                                            DataType == PointDataType.Short ? new Position<short>() :
                                            DataType == PointDataType.Double ? new Position<double>() : new object();
        static readonly GCHandle PosHandle = GCHandle.Alloc(Pos, GCHandleType.Pinned);

        static void LoadDelegate(this Emit emitter, Delegate dele)
        {
            DelegateStore.Add(dele);
            emitter.LoadField(DelegateStoreField);
            emitter.LoadConstant(DelegateStore.Count - 1);
            emitter.Call(getItemMethod);
            emitter.CastClass(DelegateStore[DelegateStore.Count - 1].GetType());
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

        const string HarmonyId = Constants.AppName + " NeoLua Scripting";
        static readonly Harmony HarmonyInst = new Harmony(HarmonyId);
        static public void Uninitialize(bool isProcessTerminating = false)
        {
            try
            {
                HarmonyInst.UnpatchAll(HarmonyId);
                PrefixStore.Clear();
                PostfixStore.Clear();
                DelegateStore.Clear();
                L?.Dispose();
                L = null;
            }
            catch (Exception e)
            {
                Logging.ToFile("[NeoLua] {0}", e);
            }
        }

        const string PreparationScript = @"
            const _Traverse typeof HarmonyLib.Traverse
            Traverse = _Traverse
            const Scripting typpeof NeoLuaBootstrap.Scripting
            Position = Scripting.Pos
            OpenConsole = Scripting.Lua_OpenConsole
        ";

        static public void Initialize(string scriptPath)
        {
            try
            {
                if (Pos == null)
                    return;
                LuaApi.SetPositionAddress(PosHandle.AddrOfPinnedObject());

                L = new Lua();
                var g = L.CreateEnvironment();

                g.DoChunk(PreparationScript, nameof(PreparationScript));

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

                    if (!(config[1] is string targetMethodPath))
                    {
                        Logging.ToFile("[NeoLua] Invalid target method path in hook configuration (Index: {0}).", i + 1);
                        continue;
                    }

                    var prefixHook = config[2] as Delegate;
                    var postfixHook = config[3] as Delegate;
                    if (prefixHook == null && postfixHook == null)
                    {
                        Logging.ToFile("[NeoLua] All hook functions are invalid in hook configuration (Index: {0}).", i + 1);
                        continue;
                    }

                    var original = AccessTools.Method(targetMethodPath);
                    if (original == null)
                    {
                        Logging.ToFile("[NeoLua] Failed to get method {0}.", targetMethodPath);
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
                Logging.ToFile("[NeoLua] {0}", LuaExceptionData.GetData(e).FormatStackTrace(0, false));
                Logging.ToFile("[NeoLua] {0}", e);
                Uninitialize();
            }
            catch (Exception e)
            {
                Logging.ToFile("[NeoLua] {0}", e);
                Uninitialize();
            }
        }
    }
}
