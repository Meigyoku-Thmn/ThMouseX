using System.Runtime.InteropServices;

namespace ThMouseX.DotNet;

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
