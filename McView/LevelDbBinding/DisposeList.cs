using System;
using System.Collections.Generic;

namespace LevelDbBinding
{
    public sealed class DisposeList : IDisposable
    {
        private readonly List<WeakReference<IDisposable>> _resources = new List<WeakReference<IDisposable>>();

        public void Add(IDisposable resource)
        {
            var index = _resources.FindIndex(c => !c.TryGetTarget(out var _));
            if (index >= 0)
                _resources[index].SetTarget(resource);
            else
                _resources.Add(new WeakReference<IDisposable>(resource));
        }

        public void Dispose()
        {
            _resources.ForEach(c =>
            {
                if (c.TryGetTarget(out var resource))
                    resource.Dispose();
            });
        }
    }
}
