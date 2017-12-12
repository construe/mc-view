using System.Collections.Generic;

namespace LevelDbBinding
{
    public class KeyValueData
    {
        public IReadOnlyList<byte> Key { get; set; }

        public IReadOnlyList<byte> Value { get; set; }
    }
}
