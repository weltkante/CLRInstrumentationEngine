#pragma once

#include <unordered_map>

#include "..\src\ExtensionsCommon\InstrumentationMethodBase.h"
#include "..\src\ExtensionsCommon\ICodeInjectorFwd.h"
#include "..\src\ExtensionsCommon\DecorationCallbacksInfoStaticReader.h"
#include "..\src\ExtensionsCommon\NativeInstanceMethodInstrumentationInfo.h"
#include "..\src\ExtensionsCommon\MethodInstrumentationInfoCollection.h"
#include "..\src\ExtensionsCommon\InteropInstrumentationHandler.h"
#include "..\src\ExtensionsCommon\ManagedRedirectCodeInjector.h"

class CMockingTarget
{
public:
    const std::wstring AssemblyName;
    const std::wstring ModuleName;
    const std::wstring TypeName;
    const std::wstring MethodName;
    const int32_t ArgumentCount;

    CMockingTarget(
        std::wstring AssemblyName,
        std::wstring ModuleName,
        std::wstring TypeName,
        std::wstring MethodName,
        int32_t ArgumentCount);

    CMockingTarget(const CMockingTarget&) = default;
    CMockingTarget(CMockingTarget&&) = default;
    CMockingTarget& operator=(const CMockingTarget&) = default;
    CMockingTarget& operator=(CMockingTarget&&) = default;

    bool operator==(const CMockingTarget& other) const
    {
        return ArgumentCount == other.ArgumentCount
            && MethodName == other.MethodName
            && TypeName == other.TypeName
            && ModuleName == other.ModuleName
            && AssemblyName == other.AssemblyName;
    }

    bool operator!=(const CMockingTarget& other) const { return !(*this == other); }
};

namespace std
{
    template <>
    struct hash<CMockingTarget>
    {
        size_t operator()(const CMockingTarget& k) const
        {
            // Compute individual hash values for first, second and third
            // http://stackoverflow.com/a/1646913/126995
            size_t res = 17;
            res = res * 31 + hash<wstring>()(k.AssemblyName);
            res = res * 31 + hash<wstring>()(k.ModuleName);
            res = res * 31 + hash<wstring>()(k.TypeName);
            res = res * 31 + hash<wstring>()(k.MethodName);
            res = res * 31 + hash<int>()(k.ArgumentCount);
            return res;
        }
    };
}

class CMockingInjection abstract : std::enable_shared_from_this<CMockingInjection>
{
public:
    CMockingInjection() {}
    virtual ~CMockingInjection() {}
    virtual HRESULT Instrument(const IProfilerManagerSptr& spManager, const IMethodInfoSptr& spMethodInfo) = 0;

    AGENT_DECLARE_NO_LOGGING

    CMockingInjection(const CMockingInjection&) = delete;
    CMockingInjection(CMockingInjection&&) = delete;
    CMockingInjection& operator=(const CMockingInjection&) = delete;
    CMockingInjection& operator=(CMockingInjection&&) = delete;
};

class CMockingAPI final : public CMockingInjection
{
public:
    const std::wstring TargetType;
    const std::wstring TargetMethod;
    const CReflectionHelperSptr m_spReflectionHelper;

    CMockingAPI(std::wstring type, std::wstring method)
        : TargetType(std::move(type))
        , TargetMethod(std::move(method))
    {
    }

    HRESULT Instrument(const IProfilerManagerSptr& spManager, const IMethodInfoSptr& spMethodInfo) override;
};

class CMockingRecord final : public CMockingInjection
{
public:
    const CReflectionHelperSptr m_spReflectionHelper;
    const int32_t CallbackId;
    ATL::CComPtr<IMethodInfo> pMethodInfo;

    explicit CMockingRecord(int32_t cid)
        : CallbackId(cid)
    {
    }

    HRESULT Instrument(const IProfilerManagerSptr& spManager, const IMethodInfoSptr& spMethodInfo) override;
};

EXTERN_C extern const GUID CLSID_MockingEngine;

typedef CDataContainerAdapterImplT<std::shared_ptr<CMockingInjection>, IMethodInfoSptr, &CLSID_MockingEngine, &CLSID_MockingEngine> CMockingDataAdapter;

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
        _In_ const __int64 assemblyPtr,
        _In_ const __int64 modulePtr,
        _In_ const __int64 typeNamePtr,
        _In_ const __int64 methodNamePtr,
        _In_ const __int32 argumentCount,
        _In_ const __int32 methodId);

private:
    IProfilerManagerSptr m_spHost;
    Agent::Interop::CInteropInstrumentationHandlerUptr m_spInteropHandler;

    CMockingDataAdapter m_dataAdapter;
    std::unordered_map<CMockingTarget, std::shared_ptr<CMockingInjection>> m_mockedMethods;
};
