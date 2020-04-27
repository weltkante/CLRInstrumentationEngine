using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Diagnostics.Instrumentation.Extensions.Base;

namespace InjectionMocking
{
    public delegate void MockingAction(ref bool handled);
    public delegate void MockingAction<in TArg1>(ref bool handled, TArg1 arg1);
    public delegate void MockingAction<in TArg1, in TArg2>(ref bool handled, TArg1 arg1, TArg2 arg2);
    public delegate void MockingAction<in TArg1, in TArg2, in TArg3>(ref bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3);
    public delegate void MockingAction<in TArg1, in TArg2, in TArg3, in TArg4>(ref bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4);
    public delegate void MockingAction<in TArg1, in TArg2, in TArg3, in TArg4, in TArg5>(ref bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5);
    public delegate void MockingAction<in TArg1, in TArg2, in TArg3, in TArg4, in TArg5, in TArg6>(ref bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, TArg6 arg6);
    public delegate void MockingAction<in TArg1, in TArg2, in TArg3, in TArg4, in TArg5, in TArg6, in TArg7>(ref bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, TArg6 arg6, TArg7 arg7);
    public delegate void MockingAction<in TArg1, in TArg2, in TArg3, in TArg4, in TArg5, in TArg6, in TArg7, in TArg8>(ref bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, TArg6 arg6, TArg7 arg7, TArg8 arg8);

    public delegate TResult MockingFunc<out TResult>(ref bool handled);
    public delegate TResult MockingFunc<out TResult, in TArg1>(ref bool handled, TArg1 arg1);
    public delegate TResult MockingFunc<out TResult, in TArg1, in TArg2>(ref bool handled, TArg1 arg1, TArg2 arg2);
    public delegate TResult MockingFunc<out TResult, in TArg1, in TArg2, in TArg3>(ref bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3);
    public delegate TResult MockingFunc<out TResult, in TArg1, in TArg2, in TArg3, in TArg4>(ref bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4);
    public delegate TResult MockingFunc<out TResult, in TArg1, in TArg2, in TArg3, in TArg4, in TArg5>(ref bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5);
    public delegate TResult MockingFunc<out TResult, in TArg1, in TArg2, in TArg3, in TArg4, in TArg5, in TArg6>(ref bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, TArg6 arg6);
    public delegate TResult MockingFunc<out TResult, in TArg1, in TArg2, in TArg3, in TArg4, in TArg5, in TArg6, in TArg7>(ref bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, TArg6 arg6, TArg7 arg7);
    public delegate TResult MockingFunc<out TResult, in TArg1, in TArg2, in TArg3, in TArg4, in TArg5, in TArg6, in TArg7, in TArg8>(ref bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, TArg6 arg6, TArg7 arg7, TArg8 arg8);

    public static class MockingEngine
    {
        public static bool IsLoaded { get; private set; }

        public static void EnsureLoaded()
        {
            if (IsLoaded)
                return;

            if (NativeMethods.IsAttached() != 0)
                throw new InvalidOperationException("Host is not loaded");

            Load(
                Environment.GetEnvironmentVariable("MockingEnginePath"), // directory of native DLL
                Environment.GetEnvironmentVariable("MockingEngineName"), // native DLL without "_x86.dll"/"_x64.dll" suffix
                "{79D4B1DB-C401-471C-8F28-ABEE76073E43}", // GUID of extension
                50); // priority of extension (for injection order)

            IsLoaded = true;
        }

        private static void Load(string modulePath, string module, string classGuid, int priority)
        {
            IntPtr pModulePath = IntPtr.Zero;
            IntPtr pModuleName = IntPtr.Zero;
            IntPtr pClassGuid = IntPtr.Zero;
            try
            {
                pModulePath = Marshal.StringToHGlobalUni(modulePath);
                pModuleName = Marshal.StringToHGlobalUni(module);
                pClassGuid = Marshal.StringToHGlobalUni(classGuid);

                if (NativeMethods.Attach(pModulePath.ToInt64(), pModuleName.ToInt64(), pClassGuid.ToInt64(), priority) != 0)
                    throw new InvalidOperationException("Failed to attach extension");
            }
            finally
            {
                Marshal.FreeHGlobal(pClassGuid);
                Marshal.FreeHGlobal(pModuleName);
                Marshal.FreeHGlobal(pModulePath);
            }
        }

        public static void Decorate(string assembly, string module, string type, string method, int arguments, Delegate callback)
        {
            throw new NotImplementedException();
        }
    }
}
