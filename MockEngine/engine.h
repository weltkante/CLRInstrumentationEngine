#pragma once

#include "../src/ExtensionsCommon/InstrumentationMethodBase.h"
#include "../src/ExtensionsCommon/ICodeInjectorFwd.h"
#include "../src/ExtensionsCommon/DecorationCallbacksInfoStaticReader.h"
#include "../src/ExtensionsCommon/NativeInstanceMethodInstrumentationInfo.h"
#include "../src/ExtensionsCommon/MethodInstrumentationInfoCollection.h"
#include "../src/ExtensionsCommon/InteropInstrumentationHandler.h"

// {79D4B1DB-C401-471C-8F28-ABEE76073E43}
//[Guid("79D4B1DB-C401-471C-8F28-ABEE76073E43")]
//DEFINE_GUID(CLSID_MockingEngine, 0x79d4b1db, 0xc401, 0x471c, 0x8f, 0x28, 0xab, 0xee, 0x76, 0x7, 0x3e, 0x43);
//static const GUID CLSID_MockingEngine = { 0x79d4b1db, 0xc401, 0x471c, { 0x8f, 0x28, 0xab, 0xee, 0x76, 0x7, 0x3e, 0x43 } };

EXTERN_C extern const GUID CLSID_MockingEngine;

class ATL_NO_VTABLE DECLSPEC_UUID("79D4B1DB-C401-471C-8F28-ABEE76073E43") CMockingEngine
    : public ATL::CComObjectRootEx<ATL::CComMultiThreadModelNoCS>
    , public ATL::CComCoClass<CMockingEngine, &CLSID_MockingEngine>
    , public CInstrumentationMethodBase
{
public:
    DECLARE_NO_REGISTRY()
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    DECLARE_NOT_AGGREGATABLE(CMockingEngine)
    BEGIN_COM_MAP(CMockingEngine)
        COM_INTERFACE_ENTRY(IInstrumentationMethod)
    END_COM_MAP()

    HRESULT InternalInitialize(
        _In_ const IProfilerManagerSptr & spHost) override final;

    _Success_(return == 0) HRESULT InternalShouldInstrumentMethod(
        _In_ const IMethodInfoSptr & spMethodInfo,
        _In_ BOOL isRejit,
        _Out_ BOOL * pbInstrument) override final;

    _Success_(return == 0) HRESULT InternalInstrumentMethod(
        _In_ const IMethodInfoSptr & spMethodInfo,
        _In_ BOOL isRejit) override final;

    _Success_(return == 0) HRESULT InternalAllowInlineSite(
        _In_ const IMethodInfoSptr & spMethodInfoInlinee,
        _In_ const IMethodInfoSptr & spMethodInfoCaller,
        _Out_ BOOL * pbAllowInline) override final;

    HRESULT Decorate(
        _In_ const __int32 methodId,
        _In_ const __int64 assemblyPtr,
        _In_ const __int64 modulePtr,
        _In_ const __int64 typeNamePtr,
        _In_ const __int64 methodNamePtr,
        _In_ const __int32 argumentCount);

private:
    IProfilerManagerSptr m_spHost;
    Agent::Interop::CInteropInstrumentationHandlerUptr m_spInteropHandler;
};
