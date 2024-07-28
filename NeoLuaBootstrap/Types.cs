using System.Runtime.InteropServices;

namespace NeoLuaBootstrap
{
    [StructLayout(LayoutKind.Sequential)]
    public class Position<T>
    {
        public T X;
        public T Y;
    }

    public enum PointDataType
    {
        None, Int, Float, Short, Double
    };
}
