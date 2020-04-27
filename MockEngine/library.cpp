#include "main.h"

struct CCustomAtlModule : ATL::CAtlDllModuleT<CCustomAtlModule>
{
    DECLARE_NO_REGISTRY()
};

static CCustomAtlModule _AtlModule;

EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    return _AtlModule.DllMain(dwReason, lpReserved);
}

STDAPI DllCanUnloadNow()
{
    return _AtlModule.DllCanUnloadNow();
}

STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID FAR* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

STDAPI DllRegisterServer()
{
    return _AtlModule.DllRegisterServer(FALSE);
}

STDAPI DllUnregisterServer()
{
    return _AtlModule.DllRegisterServer(FALSE);
}
