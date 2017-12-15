using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;

namespace LevelDbBinding
{
    public class Iterator : IEnumerator<KeyValueData>
    {
        private readonly SafeHandle _iteratorHandle;

        private bool _beforeStart = true;

        private bool _isValid;

        private KeyValueData _current;

        private readonly DisposeList _resources = new DisposeList();

        public Iterator(SafeHandle iteratorHandle)
        {
            _iteratorHandle = iteratorHandle;
        }

        public bool MoveNext()
        {
            if (_iteratorHandle.IsClosed)
                throw new ObjectDisposedException(nameof(Iterator));

            _current = null;

            if (_beforeStart)
            {
                _isValid = DllInterface.IteratorSeekToFirst(_iteratorHandle.DangerousGetHandle());
                _beforeStart = false;
            }
            else if (_isValid)
            {
                _isValid = DllInterface.IteratorNext(_iteratorHandle.DangerousGetHandle());
            }

            return _isValid;
        }

        public void Reset()
        {
            _current = null;
            _beforeStart = true;
            _isValid = false;
        }

        public KeyValueData Current => _current ?? (_current = GetData());

        object IEnumerator.Current => Current;

        private KeyValueData GetData()
        {
            if (_iteratorHandle.IsClosed)
                throw new ObjectDisposedException(nameof(Iterator));

            if (!_isValid)
                throw new InvalidOperationException("Invalid iterator state");

            var data = DllInterface.IteratorGetData(_iteratorHandle.DangerousGetHandle());

            var key = new UnmanagedByteArray(data.keyData, checked((int) data.keySize));
            _resources.Add(key);
            var value = new UnmanagedByteArray(data.valueData, checked((int) data.valueSize));
            _resources.Add(value);

            return new KeyValueData
            {
                Key = key,
                Value = value
            };
        }

        public void Dispose()
        {
            _resources.Dispose();
            _iteratorHandle.Dispose();
        }
    }
}
