using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

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
        None, Int, Float, Short
    };
}
