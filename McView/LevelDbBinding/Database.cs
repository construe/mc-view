using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace LevelDbBinding
{
    public sealed class Database : IDisposable, IEnumerable<KeyValueData>
    {
        private readonly SafeHandle _databaseHandle;

        private readonly DisposeList _resources = new DisposeList();

        public Database(string name)
        {
            _databaseHandle = new PointerHandle(DllInterface.OpenDatabase(name), DllInterface.CloseDatabase);
            if(_databaseHandle.IsInvalid)
                throw new InvalidOperationException("Could not open database");
        }

        public void Dispose()
        {
            _resources.Dispose();
            _databaseHandle.Dispose();
        }

        public IEnumerator<KeyValueData> GetEnumerator()
        {
            if (_databaseHandle.IsClosed)
                throw new ObjectDisposedException(nameof(Database));

            var iteratorHandle = new PointerHandle(DllInterface.NewIterator(_databaseHandle.DangerousGetHandle())
                , DllInterface.DisposeIterator);
            var iterator = new Iterator(iteratorHandle);
            _resources.Add(iterator);
            return iterator;
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
    }
}
