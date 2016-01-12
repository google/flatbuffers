using System;

namespace FlatBuffers.Test
{
    internal static class FuzzTestData
    {
        private static readonly byte[] _overflowInt32 = new byte[] {0x83, 0x33, 0x33, 0x33};
        private static readonly byte[] _overflowInt64 = new byte[] { 0x84, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44 };

        public static readonly bool BoolValue = true;
        public static readonly  sbyte Int8Value = -127;        // 0x81
        public static readonly  byte UInt8Value = 255;         // 0xFF
        public static readonly  short Int16Value = -32222;     // 0x8222;
        public static readonly  ushort UInt16Value = 65262;      // 0xFEEE
        public static readonly int Int32Value = BitConverter.ToInt32(_overflowInt32, 0);
        public static readonly uint UInt32Value = 0xFDDDDDDD;
        public static readonly long Int64Value = BitConverter.ToInt64(_overflowInt64, 0);
        public static readonly ulong UInt64Value = 0xFCCCCCCCCCCCCCCC;
        public static readonly float Float32Value = 3.14159f;
        public static readonly double Float64Value = 3.14159265359;
    }
}