#pragma once

// {79D4B1DB-C401-471C-8F28-ABEE76073E43}
//[Guid("79D4B1DB-C401-471C-8F28-ABEE76073E43")]
//DEFINE_GUID(CLSID_MockingEngine, 0x79d4b1db, 0xc401, 0x471c, 0x8f, 0x28, 0xab, 0xee, 0x76, 0x7, 0x3e, 0x43);
//static const GUID CLSID_MockingEngine = { 0x79d4b1db, 0xc401, 0x471c, { 0x8f, 0x28, 0xab, 0xee, 0x76, 0x7, 0x3e, 0x43 } };

EXTERN_C extern const GUID CLSID_MockingEngine;

class ATL_NO_VTABLE DECLSPEC_UUID("79D4B1DB-C401-471C-8F28-ABEE76073E43") CMockingEngine
    : public CComObjectRootEx<CComMultiThreadModelNoCS>
    , public CComCoClass<CMockingEngine, &CLSID_MockingEngine>
    , public IInstrumentationMethod
{
public:
    DECLARE_NO_REGISTRY()
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    DECLARE_NOT_AGGREGATABLE(CMockingEngine)
    BEGIN_COM_MAP(CMockingEngine)
        COM_INTERFACE_ENTRY(IInstrumentationMethod)
    END_COM_MAP()

    STDMETHOD(Initialize)(
        /* [in] */ __RPC__in_opt IProfilerManager * pProfilerManager) override;

    STDMETHOD(OnAppDomainCreated)(
        /* [in] */ __RPC__in_opt IAppDomainInfo * pAppDomainInfo) override;

    STDMETHOD(OnAppDomainShutdown)(
        /* [in] */ __RPC__in_opt IAppDomainInfo * pAppDomainInfo) override;

    STDMETHOD(OnAssemblyLoaded)(
        /* [in] */ __RPC__in_opt IAssemblyInfo * pAssemblyInfo) override;

    STDMETHOD(OnAssemblyUnloaded)(
        /* [in] */ __RPC__in_opt IAssemblyInfo * pAssemblyInfo) override;

    STDMETHOD(OnModuleLoaded)(
        /* [in] */ __RPC__in_opt IModuleInfo * pModuleInfo) override;

    STDMETHOD(OnModuleUnloaded)(
        /* [in] */ __RPC__in_opt IModuleInfo * pModuleInfo) override;

    STDMETHOD(OnShutdown)() override;

    STDMETHOD(ShouldInstrumentMethod)(
        /* [in] */ __RPC__in_opt IMethodInfo * pMethodInfo,
        /* [in] */ BOOL isRejit,
        /* [out] */ __RPC__out BOOL * pbInstrument) override;

    STDMETHOD(BeforeInstrumentMethod)(
        /* [in] */ __RPC__in_opt IMethodInfo * pMethodInfo,
        /* [in] */ BOOL isRejit) override;

    STDMETHOD(InstrumentMethod)(
        /* [in] */ __RPC__in_opt IMethodInfo * pMethodInfo,
        /* [in] */ BOOL isRejit) override;

    STDMETHOD(OnInstrumentationComplete)(
        /* [in] */ __RPC__in_opt IMethodInfo * pMethodInfo,
        /* [in] */ BOOL isRejit) override;

    STDMETHOD(AllowInlineSite)(
        /* [in] */ __RPC__in_opt IMethodInfo * pMethodInfoInlinee,
        /* [in] */ __RPC__in_opt IMethodInfo * pMethodInfoCaller,
        /* [out] */ __RPC__out BOOL * pbAllowInline) override;

private:
    ComPtr<IProfilerManagerLogging> mLogger;
    ComPtr<ICorProfilerInfo3> mProfiler;
    AppDomainID mAttachedAppDomainId = 0;
    AppDomainID mSharedAppDomainId = 0;
};
