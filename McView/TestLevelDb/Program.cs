using System;
using LevelDbBinding;

namespace TestLevelDb
{
    internal class Program
    {
        private static void Main()
        {
            using (var db = new Database("world/db"))
            {
                using (var enumerator = db.GetEnumerator())
                {
                    int i = 0;
                    while (enumerator.MoveNext())
                    {
                        Console.WriteLine(++i);
                    }
                }
            }
            Console.ReadKey();
        }
    }
}