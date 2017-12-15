using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace LevelDbBinding
{
    public class UnmanagedByteArray : IReadOnlyList<byte>, IDisposable
    {
        private class Enumerator : IEnumerator<byte>
        {
            private int _index = -1;

            private readonly UnmanagedByteArray _array;

            public Enumerator(UnmanagedByteArray array)
            {
                _array = array;
            }

            public bool MoveNext()
            {
                ++_index;
                return _index < _array.Count;
            }

            public void Reset()
            {
                _index = -1;
            }

            public byte Current => _array[_index];

            object IEnumerator.Current => Current;

            public void Dispose()
            {
            }
        }

        private readonly IntPtr _pointer;

        private bool _isDisposed;

        public UnmanagedByteArray(IntPtr pointer, int size)
        {
            _pointer = pointer;
            Count = size;
        }

        public IEnumerator<byte> GetEnumerator()
        {
            return new Enumerator(this);
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        public void Dispose()
        {
            _isDisposed = true;
        }

        public int Count { get; }

        public byte this[int index]
        {
            get
            {
                if(_isDisposed)
                    throw new ObjectDisposedException(nameof(UnmanagedByteArray));

                if (index >= Count)
                    throw new IndexOutOfRangeException();

                return Marshal.ReadByte(_pointer, index);
            }
        }
    }
}
