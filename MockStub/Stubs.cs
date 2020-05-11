using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.Remoting.Messaging;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Diagnostics.Instrumentation.Extensions.Mocking
{
    public delegate void MockingAction(out bool handled);
    public delegate void MockingAction<in TArg1>(out bool handled, TArg1 arg1);
    public delegate void MockingAction<in TArg1, in TArg2>(out bool handled, TArg1 arg1, TArg2 arg2);
    public delegate void MockingAction<in TArg1, in TArg2, in TArg3>(out bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3);
    public delegate void MockingAction<in TArg1, in TArg2, in TArg3, in TArg4>(out bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4);
    public delegate void MockingAction<in TArg1, in TArg2, in TArg3, in TArg4, in TArg5>(out bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5);
    public delegate void MockingAction<in TArg1, in TArg2, in TArg3, in TArg4, in TArg5, in TArg6>(out bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, TArg6 arg6);
    public delegate void MockingAction<in TArg1, in TArg2, in TArg3, in TArg4, in TArg5, in TArg6, in TArg7>(out bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, TArg6 arg6, TArg7 arg7);
    public delegate void MockingAction<in TArg1, in TArg2, in TArg3, in TArg4, in TArg5, in TArg6, in TArg7, in TArg8>(out bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, TArg6 arg6, TArg7 arg7, TArg8 arg8);

    public delegate TResult MockingFunc<out TResult>(out bool handled);
    public delegate TResult MockingFunc<in TArg1, out TResult>(out bool handled, TArg1 arg1);
    public delegate TResult MockingFunc<in TArg1, in TArg2, out TResult>(out bool handled, TArg1 arg1, TArg2 arg2);
    public delegate TResult MockingFunc<in TArg1, in TArg2, in TArg3, out TResult>(out bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3);
    public delegate TResult MockingFunc<in TArg1, in TArg2, in TArg3, in TArg4, out TResult>(out bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4);
    public delegate TResult MockingFunc<in TArg1, in TArg2, in TArg3, in TArg4, in TArg5, out TResult>(out bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5);
    public delegate TResult MockingFunc<in TArg1, in TArg2, in TArg3, in TArg4, in TArg5, in TArg6, out TResult>(out bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, TArg6 arg6);
    public delegate TResult MockingFunc<in TArg1, in TArg2, in TArg3, in TArg4, in TArg5, in TArg6, in TArg7, out TResult>(out bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, TArg6 arg6, TArg7 arg7);
    public delegate TResult MockingFunc<in TArg1, in TArg2, in TArg3, in TArg4, in TArg5, in TArg6, in TArg7, in TArg8, out TResult>(out bool handled, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, TArg6 arg6, TArg7 arg7, TArg8 arg8);

    public static class Stubs
    {
        private const int MaxArguments = 8;
        private static Dictionary<int, Delegate> _registry = new Dictionary<int, Delegate>();
        private static int _lastToken;

        private static Type GetDelegateType(MethodInfo method)
        {
            if (method is null)
                throw new ArgumentNullException(nameof(method));

            var returnType = method.ReturnType;
            if (returnType.IsByRef)
                throw new NotSupportedException();
            else if (returnType.IsPointer)
                returnType = typeof(IntPtr);

            var hasReturnType = (returnType != typeof(void));

            var methodArgs = method.GetParameters();
            var argCount = methodArgs.Length - 1;
            if (argCount < 0 || argCount > MaxArguments)
                throw new NotSupportedException();

            if (methodArgs[0].ParameterType != typeof(bool).MakeByRefType())
                throw new NotSupportedException();

            if (hasReturnType)
                argCount++;

            Type[] genericArgs = new Type[argCount];

            for (int i = 1; i < methodArgs.Length; i++)
            {
                var arg = methodArgs[i].ParameterType;
                if (arg.IsByRef)
                    throw new NotSupportedException();
                else if (arg.IsPointer)
                    arg = typeof(IntPtr);

                genericArgs[i - 1] = arg;
            }

            if (hasReturnType)
                genericArgs[argCount - 1] = returnType;

            var delegateType = hasReturnType
                ? GetOpenGenericFunc(argCount)
                : GetOpenGenericAction(argCount);

            if (argCount > 0)
                delegateType = delegateType.MakeGenericType(genericArgs);

            return delegateType;
        }

        private static Type GetOpenGenericAction(int argCount)
        {
            switch (argCount)
            {
                case 0: return typeof(MockingAction);
                case 1: return typeof(MockingAction<>);
                case 2: return typeof(MockingAction<,>);
                case 3: return typeof(MockingAction<,,>);
                case 4: return typeof(MockingAction<,,,>);
                case 5: return typeof(MockingAction<,,,,>);
                case 6: return typeof(MockingAction<,,,,,>);
                case 7: return typeof(MockingAction<,,,,,,>);
                case 8: return typeof(MockingAction<,,,,,,,>);
                default: throw new NotSupportedException();
            }
        }

        private static Type GetOpenGenericFunc(int argCount)
        {
            switch (argCount)
            {
                case 1: return typeof(MockingFunc<>);
                case 2: return typeof(MockingFunc<,>);
                case 3: return typeof(MockingFunc<,,>);
                case 4: return typeof(MockingFunc<,,,>);
                case 5: return typeof(MockingFunc<,,,,>);
                case 6: return typeof(MockingFunc<,,,,,>);
                case 7: return typeof(MockingFunc<,,,,,,>);
                case 8: return typeof(MockingFunc<,,,,,,,>);
                case 9: return typeof(MockingFunc<,,,,,,,,>);
                default: throw new NotSupportedException();
            }
        }

        private static Delegate CreateDelegate(MethodInfo method, object target)
        {
            if (target is null)
                return Delegate.CreateDelegate(GetDelegateType(method), method);
            else
                return Delegate.CreateDelegate(GetDelegateType(method), target, method);
        }

        public static int RegisterStub(MethodInfo method, object target) => RegisterStubImpl(CreateDelegate(method, target));

        private static int RegisterStubImpl(Delegate callback)
        {
            lock (_registry)
            {
                int token = ++_lastToken;
                _registry.Add(token, callback);
                return token;
            }
        }

        public static void RemoveStub(int token)
        {
            lock (_registry)
            {
                _registry.Remove(token);
            }
        }

        public static TDelegate GetCallback<TDelegate>(int token) where TDelegate : Delegate
        {
            lock (_registry)
            {
                if (_registry.TryGetValue(token, out var callback))
                    return (TDelegate)callback;
                else
                    return default;
            }
        }
    }
}
