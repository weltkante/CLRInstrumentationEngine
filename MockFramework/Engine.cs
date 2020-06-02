using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Diagnostics.Instrumentation.Extensions.Base;

namespace InjectionMocking
{
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

        public static void Decorate(Type type, string method, uint arguments, MethodInfo callback, object target)
        {
            Decorate(type.Assembly.GetName().Name, type.Module.Name, type.FullName, method, arguments, callback, target);
        }

        public static void Decorate(string assembly, string module, string type, string method, uint arguments, MethodInfo callback, object target)
        {
            IntPtr pAssemblyName = IntPtr.Zero;
            IntPtr pModuleName = IntPtr.Zero;
            IntPtr pTypeName = IntPtr.Zero;
            IntPtr pMethodName = IntPtr.Zero;
            try
            {
                pAssemblyName = Marshal.StringToHGlobalUni(assembly);
                pModuleName = Marshal.StringToHGlobalUni(module);
                pTypeName = Marshal.StringToHGlobalUni(type);
                pMethodName = Marshal.StringToHGlobalUni(method);

                bool useV1 = false;

                if (useV1 || callback is DynamicMethod || target != null)
                {
                    var callbackId = Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.NativeMethods.Register(callback, target);
                    if (Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.NativeMethods.Decorate(pAssemblyName.ToInt64(), pModuleName.ToInt64(), pTypeName.ToInt64(), pMethodName.ToInt64(), arguments, callbackId) != 0)
                        throw new InvalidOperationException("Failed to attach extension");
                }
                else
                {
                    System.Runtime.CompilerServices.RuntimeHelpers.PrepareMethod(callback.MethodHandle);
                    if (Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.NativeMethods.DecorateUnsafe(pAssemblyName.ToInt64(), pModuleName.ToInt64(), pTypeName.ToInt64(), pMethodName.ToInt64(), arguments, callback.MethodHandle.GetFunctionPointer().ToInt64()) != 0)
                        throw new InvalidOperationException("Failed to attach extension");
                }
            }
            finally
            {
                Marshal.FreeHGlobal(pMethodName);
                Marshal.FreeHGlobal(pTypeName);
                Marshal.FreeHGlobal(pModuleName);
                Marshal.FreeHGlobal(pAssemblyName);
            }
        }
    }
}
