using System;
using System.Runtime.InteropServices;

namespace LevelDbBinding
{
    public class PointerHandle : SafeHandle
    {
        private readonly Action<IntPtr> _releaseHandle;

        public PointerHandle(IntPtr handle, Action<IntPtr> releaseHandle) : base(IntPtr.Zero, true)
        {
            SetHandle(handle);
            _releaseHandle = releaseHandle ?? throw new ArgumentNullException(nameof(releaseHandle));
        }

        protected override bool ReleaseHandle()
        {
            _releaseHandle(handle);
            return true;
        }

        public override bool IsInvalid => handle == IntPtr.Zero;
    }
}
