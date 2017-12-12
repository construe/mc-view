using System;
using System.Runtime.InteropServices;

namespace LevelDbBinding
{
    public static class DllInterface
    {
        [DllImport("leveldb.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr OpenDatabase(string name);

        [DllImport("leveldb.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void CloseDatabase(IntPtr db);

        [DllImport("leveldb.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr NewIterator(IntPtr db);

        [DllImport("leveldb.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void DisposeIterator(IntPtr iterator);

        [DllImport("leveldb.dll", CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool IteratorSeekToFirst(IntPtr iterator);

        [DllImport("leveldb.dll", CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool IteratorNext(IntPtr iterator);

        [StructLayout(LayoutKind.Sequential)]
        public struct KeyValueData
        {
            public uint keySize;
            public IntPtr keyData;
            public uint valueSize;
            public IntPtr valueData;
        }

        [DllImport("leveldb.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern KeyValueData IteratorGetData(IntPtr iterator);
    }
}
