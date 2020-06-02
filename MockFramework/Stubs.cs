using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Net.Http.Headers;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Diagnostics.Instrumentation.Extensions.Base
{
    internal static class Callbacks
    {
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static void Add(int methodId, Func<object> onBegin, Func<object, object, object> onEnd, Action<object, Exception> onException)
        {
            throw new InvalidOperationException("Instrumentation Agent is not active");
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        public static void Add(int methodId, Func<object, object> onBegin, Func<object, object, object, object> onEnd, Action<object, Exception, object> onException)
        {
            throw new InvalidOperationException("Instrumentation Agent is not active");
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        public static void Add(int methodId, Func<object, object, object> onBegin, Func<object, object, object, object, object> onEnd, Action<object, Exception, object, object> onException)
        {
            throw new InvalidOperationException("Instrumentation Agent is not active");
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        public static void Add(int methodId, Func<object, object, object, object> onBegin, Func<object, object, object, object, object, object> onEnd, Action<object, Exception, object, object, object> onException)
        {
            throw new InvalidOperationException("Instrumentation Agent is not active");
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        public static void Add(int methodId, Func<object, object, object, object, object> onBegin, Func<object, object, object, object, object, object, object> onEnd, Action<object, Exception, object, object, object, object> onException)
        {
            throw new InvalidOperationException("Instrumentation Agent is not active");
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        public static void Add(int methodId, Func<object, object, object, object, object, object> onBegin, Func<object, object, object, object, object, object, object, object> onEnd, Action<object, Exception, object, object, object, object, object> onException)
        {
            throw new InvalidOperationException("Instrumentation Agent is not active");
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        public static void Add(int methodId, Func<object, object, object, object, object, object, object> onBegin, Func<object, object, object, object, object, object, object, object, object> onEnd, Action<object, Exception, object, object, object, object, object, object> onException)
        {
            throw new InvalidOperationException("Instrumentation Agent is not active");
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        public static void Remove(int methodId, int argsCount)
        {
            throw new InvalidOperationException("Instrumentation Agent is not active");
        }
    }

    internal static class NativeMethods
    {
        [MethodImpl(MethodImplOptions.NoInlining)]
        internal static uint IsAttached()
        {
            return 1u; // S_FALSE
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        internal static uint Attach(long modulePath, long module, long classGuid, int priority)
        {
            return 1u; // S_FALSE
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        internal static uint GetAgentVersion(uint bufferSize, long agentVersionPtr)
        {
            return 0x80004001u; // E_NOTIMPL
        }
    }
}

namespace Microsoft.Diagnostics.Instrumentation.Extensions.Intercept.Internal
{
    internal static class NativeMethods
    {
        [MethodImpl(MethodImplOptions.NoInlining)]
        internal static int Decorate(int methodId, long assemblyNamePtr, long moduleNamePtr, long typeNamePtr, long methodNamePtr, uint argsCount)
        {
            return 0;
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        internal static int GetTrace(uint bufferSize, long buffer, out uint size, out bool hasMore)
        {
            size = 0;
            hasMore = false;
            return 0;
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        internal static int EnableTrace(int severity, int keywords)
        {
            return 0;
        }
    }
}

namespace Microsoft.Diagnostics.Instrumentation.Extensions.Mocking
{
    internal static class NativeMethods
    {
        [MethodImpl(MethodImplOptions.NoInlining)]
        internal static int DecorateUnsafe(long assemblyNamePtr, long moduleNamePtr, long typeNamePtr, long methodNamePtr, uint argsCount, long methodPtr)
        {
            throw new InvalidOperationException("Mocking Engine has not been loaded.");
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        internal static int Decorate(long assemblyNamePtr, long moduleNamePtr, long typeNamePtr, long methodNamePtr, uint argsCount, int methodId)
        {
            throw new InvalidOperationException("Mocking Engine has not been loaded.");
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        internal static int Register(MethodInfo method, object target)
        {
            throw new InvalidOperationException("Mocking Engine has not been loaded.");
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        internal static void Unregister(int stubId)
        {
            throw new InvalidOperationException("Mocking Engine has not been loaded.");
        }
    }
}
