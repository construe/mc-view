using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using LevelDbBinding;

namespace TestLevelDb
{
    internal class Program
    {
        private static void Main()
        {
            using (var db = new Database("world/db"))
            using (var enumerator = db.GetEnumerator())
            using (var writer = new StreamWriter("out.txt", false))
            {
                while (enumerator.MoveNext())
                {
                    var current = enumerator.Current;
                    if (current != null)
                    {
                        HexDump(current.Key, writer, 16);
                        writer.WriteLine();
                        writer.WriteLine();
                        HexDump(current.Value, writer, 16);
                        writer.WriteLine();
                        writer.WriteLine("=====================================================");
                    }
                }
            }
        }

        public static void HexDump(IEnumerable<byte> bytes, TextWriter writer, int bytesPerLine)
        {
            using (var enumerator = bytes.GetEnumerator())
            {
                int currentByteIndex = 0;
                while (enumerator.MoveNext())
                {
                    if (currentByteIndex % bytesPerLine == 0)
                    {
                        if (currentByteIndex > 0)
                            writer.WriteLine();
                        writer.Write($"{currentByteIndex:X4}");
                    }

                    writer.Write($" {enumerator.Current:X2}");
                    ++currentByteIndex;
                }
            }
        }
    }
}