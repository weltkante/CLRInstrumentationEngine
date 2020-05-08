#include "main.h"
#include "engine.h"
#include "..\src\ExtensionsCommon\DataItemContainer.h"
#include "..\src\ExtensionsCommon\WellKnownInstrumentationDefs.h"

namespace Agent { namespace Interop { static const GUID GUID_MethodInfoDataItem = CLSID_MockingEngine; } } // our unique key for IDataContainer entries

EXTERN_C const GUID CLSID_MockingEngine = __uuidof(CMockingEngine);
OBJECT_ENTRY_AUTO(CLSID_MockingEngine, CMockingEngine)

static HRESULT STDMETHODCALLTYPE DecorateStub(
    _In_ CMockingEngine* pThis,
    _In_ const __int64 assemblyPtr,
    _In_ const __int64 modulePtr,
    _In_ const __int64 typeNamePtr,
    _In_ const __int64 methodNamePtr,
    _In_ const __int32 argumentCount,
    _In_ const __int32 methodId)
{
    return pThis->Decorate(assemblyPtr, modulePtr, typeNamePtr, methodNamePtr, argumentCount, methodId);
}

HRESULT CMockingEngine::InternalInitialize(const IProfilerManagerSptr& spHost)
{
    auto hr = S_OK;

    Agent::Instrumentation::CMethodInstrumentationInfoCollectionSptr spInteropMethods;
    IfFailRet(Create(spInteropMethods));

    Agent::Instrumentation::Native::CNativeInstanceMethodInstrumentationInfoSptr spInteropMethod;

    // For all AppInsights extensions, turn ngen off.
    IUnknownSptr spIUnknown;
    IfFailRet(spHost->GetCorProfilerInfo(&spIUnknown));
    ICorProfilerInfoQiSptr spICorProfilerInfo = spIUnknown;
    IfFailRet(spICorProfilerInfo->SetEventMask(COR_PRF_DISABLE_ALL_NGEN_IMAGES));

    IfFailRet(
        Create(
            spInteropMethod,
            Agent::Interop::WildcardName,
            Agent::Interop::WildcardName,
            L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.NativeMethods.Decorate",
            6,
            0,
            reinterpret_cast<UINT_PTR>(this),
            reinterpret_cast<UINT_PTR>(&DecorateStub)));
    IfFailRet(spInteropMethods->Add(spInteropMethod));

    IfFailRet(CreateAndBuildUp(m_spInteropHandler, spInteropMethods));

    m_spHost = spHost;
    return hr;
}

HRESULT CMockingEngine::InternalShouldInstrumentMethod(const IMethodInfoSptr& spMethodInfo, BOOL isRejit, BOOL* pbInstrument)
{
    *pbInstrument = FALSE;

    HRESULT hr = S_OK;
    IfFailRet(hr = m_spInteropHandler->ShouldInstrument(spMethodInfo));

    if (hr == S_FALSE) {
        std::shared_ptr<CMockingInjection> record;
        IfFailRet(hr = m_dataAdapter.GetDataItem(spMethodInfo, record));
        if (hr == S_FALSE) {
            ATL::CComBSTR bstrFullMethodName;
            IfFailRet(spMethodInfo->GetFullName(&bstrFullMethodName));

            ATL::CComPtr<IEnumMethodParameters> ptrMethodParameters;
            IfFailRet(spMethodInfo->GetParameters(&ptrMethodParameters));

            WCHAR buffer[500];
            swprintf_s(buffer, L"%s\n", bstrFullMethodName.m_str);
            OutputDebugStringW(buffer);

            if (bstrFullMethodName == L"InjectionMocking.Helper.GetText") {
                int i = 53;
            }

            if (bstrFullMethodName == L"InjectionMocking.Helper.ShowValue") {
                int i = 53;
            }

            auto dwParametersCount = DWORD();
            IfFailRet(ptrMethodParameters->GetCount(&dwParametersCount));

            if (bstrFullMethodName == L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.NativeMethods.Register") {
                if (dwParametersCount == 2) {
                    IfFailRet(m_dataAdapter.SetDataItem(spMethodInfo, std::make_shared<CMockingAPI>(L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.Stubs", L"RegisterStub")));
                    hr = S_OK;
                }
            }
            else if (bstrFullMethodName == L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.NativeMethods.Unregister") {
                if (dwParametersCount == 2) {
                    IfFailRet(m_dataAdapter.SetDataItem(spMethodInfo, std::make_shared<CMockingAPI>(L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.Stubs", L"RemoveStub")));
                    hr = S_OK;
                }
            }
            else {
                IModuleInfoSptr sptrModuleInfo;
                IfFailRet(spMethodInfo->GetModuleInfo(&sptrModuleInfo));

                ATL::CComBSTR bstrModuleName;
                IfFailRet(sptrModuleInfo->GetModuleName(&bstrModuleName));

                IAssemblyInfoSptr sptrAssemblyInfo;
                IfFailRet(sptrModuleInfo->GetAssemblyInfo(&sptrAssemblyInfo));

                ATL::CComBSTR bstrAssemblyName;
                IfFailRet(sptrAssemblyInfo->GetName(&bstrAssemblyName));

                ATL::CComBSTR bstrMethodName;
                IfFailRet(spMethodInfo->GetName(&bstrMethodName));

                std::wstring sFullMethodName(bstrFullMethodName);
                std::wstring sTypeName = sFullMethodName.substr(0, sFullMethodName.length() - bstrMethodName.Length() - 1);

                // TODO: converter from CComBSTR to wstring that respects bstring length instead of doing null termination
                CMockingTarget target(std::wstring(bstrAssemblyName), std::wstring(bstrModuleName), sTypeName, std::wstring(bstrMethodName), dwParametersCount);

                auto it = m_mockedMethods.find(target);
                if (it != m_mockedMethods.end()) {
                    IfFailRet(m_dataAdapter.SetDataItem(spMethodInfo, it->second));
                    hr = S_OK;
                }
            }
        }
    }

    *pbInstrument = (hr == S_OK);
    return S_OK;
}

HRESULT CMockingEngine::InternalInstrumentMethod(const IMethodInfoSptr& spMethodInfo, BOOL isRejit)
{
    HRESULT hr = S_OK;

    std::shared_ptr<CMockingInjection> ip;
    IfFailRetHresult(m_dataAdapter.GetDataItem(spMethodInfo, ip), hr);
    if (hr == S_OK) {
        IfFailRet(ip->Instrument(m_spHost, spMethodInfo));
        return S_OK;
    }

    IfFailRet(m_spInteropHandler->Instrument(spMethodInfo));
    return S_OK;
}

HRESULT CMockingEngine::InternalAllowInlineSite(const IMethodInfoSptr& spMethodInfoInlinee, const IMethodInfoSptr& spMethodInfoCaller, BOOL* pbAllowInline)
{
    HRESULT hr = S_OK;

    IfFailRetHresult(
        m_spInteropHandler->AllowInline(
            spMethodInfoInlinee, spMethodInfoCaller), hr);

    *pbAllowInline = (hr != S_OK);

    return hr;
}

HRESULT CMockingEngine::Decorate(const __int64 assemblyPtr, const __int64 modulePtr, const __int64 typeNamePtr, const __int64 methodNamePtr, const __int32 argumentCount, const __int32 methodId)
{
    ATL::CComBSTR bstrAssemblyName(reinterpret_cast<const wchar_t*>(assemblyPtr));
    ATL::CComBSTR bstrModuleName(reinterpret_cast<const wchar_t*>(modulePtr));
    ATL::CComBSTR bstrTypeName(reinterpret_cast<const wchar_t*>(typeNamePtr));
    ATL::CComBSTR bstrMethodName(reinterpret_cast<const wchar_t*>(methodNamePtr));

    m_mockedMethods.insert(std::make_pair(
        CMockingTarget(std::wstring(bstrAssemblyName), std::wstring(bstrModuleName), std::wstring(bstrTypeName), std::wstring(bstrMethodName), argumentCount),
        std::make_shared<CMockingRecord>(methodId)));

    //IUnknownSptr spIUnknown;
    //IfFailRet(m_spHost->GetCorProfilerInfo(&spIUnknown));
    //ICorProfilerInfoQiSptr spICorProfilerInfo = spIUnknown;
    //if (!spICorProfilerInfo)
    //    return E_FAIL;

    //ATL::CComPtr<ICorProfilerModuleEnum> pModuleIter;
    //if (FAILED(spICorProfilerInfo->EnumModules(&pModuleIter)))
    //    return E_FAIL;

    ATL::CComPtr<IAppDomainCollection> pAppDomains;
    if (FAILED(m_spHost->GetAppDomainCollection(&pAppDomains)))
        return E_FAIL;

    ATL::CComPtr<IEnumAppDomainInfo> pAppDomainIter;
    if (FAILED(pAppDomains->GetAppDomains(&pAppDomainIter)))
        return E_FAIL;

    BOOL foundInCurrentDomain = FALSE;
    BOOL foundInAnyDomain = FALSE;
    for (;;) {
        ULONG n;
        ATL::CComPtr<IAppDomainInfo> pAppDomain;
        if (FAILED(pAppDomainIter->Next(1, &pAppDomain, &n)) || !pAppDomain || !n)
            break;

        ATL::CComPtr<IEnumModuleInfo> pModuleIter;
        if (FAILED(pAppDomain->GetModuleInfosByName(bstrModuleName, &pModuleIter)))
            continue;

        for (;;) {
            ATL::CComPtr<IModuleInfo> pModule;
            if (FAILED(pModuleIter->Next(1, &pModule, &n)) || !pModule || !n)
                break;

            ATL::CComPtr<IAssemblyInfo> pAssembly;
            if (FAILED(pModule->GetAssemblyInfo(&pAssembly)))
                continue;

            ATL::CComBSTR bstrThisAssemblyName;
            if (FAILED(pAssembly->GetName(&bstrThisAssemblyName)) || bstrThisAssemblyName != bstrAssemblyName)
                continue;

            ATL::CComPtr<IMetaDataImport2> pMetaDataImport;
            if (FAILED(pModule->GetMetaDataImport((IUnknown**)&pMetaDataImport)))
                continue;

            // TODO: does this understand nested type names ('+') or do we need to split those off manually? how about generic arity?
            mdTypeDef typeId;
            if (FAILED(pMetaDataImport->FindTypeDefByName(bstrTypeName, mdTokenNil, &typeId)))
                continue;

            mdMethodDef methodIds[8];
            ULONG methodCount = 0;
            HCORENUM hMethodIter = NULL;
            for (;;) {
                if (FAILED(pMetaDataImport->EnumMethodsWithName(&hMethodIter, typeId, bstrMethodName, methodIds, ARRAYSIZE(methodIds), &methodCount)) || !methodCount)
                    break;

                for (ULONG methodIndex = 0; methodIndex < methodCount; ++methodIndex) {
                    mdMethodDef methodId = methodIds[methodIndex];

                    ATL::CComPtr<IMethodInfo> pMethod;
                    if (FAILED(pModule->GetMethodInfoByToken(methodId, &pMethod)))
                        continue;

                    // TODO: check if the signature matches, if it does request a rejit
                    if (FAILED(pModule->RequestRejit(methodId)))
                        continue;

                    foundInAnyDomain = true;
                }
            }
            pMetaDataImport->CloseEnum(hMethodIter);
        }
    }

    return S_OK;

    //if (!foundInAnyDomain)
    //    return E_FAIL;

    //return foundInCurrentDomain ? S_OK : S_FALSE;
}

#pragma region CMockingRecord

CMockingTarget::CMockingTarget(std::wstring AssemblyName, std::wstring ModuleName, std::wstring TypeName, std::wstring MethodName, int32_t ArgumentCount)
    : AssemblyName(std::move(AssemblyName))
    , ModuleName(std::move(ModuleName))
    , TypeName(std::move(TypeName))
    , MethodName(std::move(MethodName))
    , ArgumentCount(ArgumentCount)
{
}

#pragma endregion

HRESULT CMockingAPI::Instrument(const IProfilerManagerSptr& spManager, const IMethodInfoSptr& spMethodInfo)
{
    HRESULT hr = S_OK;

    BYTE argsCount = 0;

    // Get signature
    BYTE pSignature[256] = {}; // magical number.
    DWORD cbSignature = 0;
    IfFailRet(spMethodInfo->GetCorSignature(_countof(pSignature), pSignature, &cbSignature));

    // Get arguments count
    IfFalseRet(cbSignature > 2, S_FALSE);
    argsCount = pSignature[1];

    // Get signature token
    IModuleInfoSptr spModuleInfo;
    IfFailRet(spMethodInfo->GetModuleInfo(&spModuleInfo));

    ATL::CComBSTR bstrMethodName;
    IfFailRet(spMethodInfo->GetName(&bstrMethodName));
    std::wstring methodName((LPWSTR)bstrMethodName);

    std::shared_ptr<Agent::Reflection::CWellKnownMethodInfo> spMethodToRedirect(
        new Agent::Reflection::CWellKnownMethodInfo(ExtensionsBaseAssemblyName, ExtensionsBaseModuleName, TargetType, TargetMethod, pSignature, cbSignature));
    mdToken tkMethodRef;
    IfFailRet(m_spReflectionHelper->DefineMethodToken(spModuleInfo, spMethodToRedirect, tkMethodRef));


    IMetaDataEmitSptr spIMetaDataEmit;
    IfFailRet(spModuleInfo->GetMetaDataEmit(reinterpret_cast<IUnknown**>(&spIMetaDataEmit)));

    //Create instructions
    IInstructionFactorySptr spInstructionFactory;
    IfFailRet(spMethodInfo->GetInstructionFactory(&spInstructionFactory));

    IInstructionGraphSptr spInstructionGraph;
    IfFailRet(spMethodInfo->GetInstructions(&spInstructionGraph));
    spInstructionGraph->RemoveAll();

    IInstructionSptr spCurrent;
    IfFailRet(spInstructionGraph->GetFirstInstruction(&spCurrent));

    for (USHORT i = 0; i < argsCount; i++)
    {
        IInstructionSptr spLoadArg;

        IfFailRet(spInstructionFactory->CreateLoadArgInstruction(i, &spLoadArg));
        IfFailRet(spInstructionGraph->InsertAfter(spCurrent, spLoadArg));

        spCurrent = spLoadArg;
    }

    IInstructionSptr spCallMethod;

    IfFailRet(spInstructionFactory->CreateTokenOperandInstruction(Cee_Call, tkMethodRef, &spCallMethod));
    IfFailRet(spInstructionGraph->InsertAfter(spCurrent, spCallMethod));
    spCurrent = spCallMethod;

    IInstructionSptr spReturn;

    IfFailRet(spInstructionFactory->CreateInstruction(Cee_Ret, &spReturn));
    IfFailRet(spInstructionGraph->InsertAfter(spCurrent, spReturn));
    spCurrent = spReturn;

    return hr;
}

static LPCWSTR GetDelegateName(BOOL hasReturnValue, DWORD argumentCount)
{
    switch (argumentCount) {
        case 0: return hasReturnValue ? L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.MockingFunc`1" : L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.MockingAction";
        case 1: return hasReturnValue ? L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.MockingFunc`2" : L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.MockingAction`1";
        case 2: return hasReturnValue ? L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.MockingFunc`3" : L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.MockingAction`2";
        case 3: return hasReturnValue ? L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.MockingFunc`4" : L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.MockingAction`3";
        case 4: return hasReturnValue ? L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.MockingFunc`5" : L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.MockingAction`4";
        case 5: return hasReturnValue ? L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.MockingFunc`6" : L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.MockingAction`5";
        case 6: return hasReturnValue ? L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.MockingFunc`7" : L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.MockingAction`6";
        case 7: return hasReturnValue ? L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.MockingFunc`8" : L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.MockingAction`7";
        case 8: return hasReturnValue ? L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.MockingFunc`9" : L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.MockingAction`8";
        default: return NULL;
    }
}

HRESULT CMockingRecord::Instrument(const IProfilerManagerSptr& spManager, const IMethodInfoSptr& spMethodInfo)
{
    IModuleInfoSptr spModuleInfo;
    IfFailRet(spMethodInfo->GetModuleInfo(&spModuleInfo));

    ITypeCreatorSptr spTypeFactory;
    IfFailRet(spModuleInfo->CreateTypeFactory(&spTypeFactory));

    ATL::CComPtr<ISignatureParser> pSigParse;
    IfFailRet(spTypeFactory->QueryInterface(IID_PPV_ARGS(&pSigParse)));

    ITypeSptr spTypeBool;
    IfFailRet(spTypeFactory->FromCorElement(CorElementType::ELEMENT_TYPE_BOOLEAN, &spTypeBool));

    DWORD cbSignature;
    IfFailRet(spMethodInfo->GetCorSignature(0, NULL, &cbSignature));
    std::vector<BYTE> signature(cbSignature);
    IfFailRet(spMethodInfo->GetCorSignature(signature.size(), &signature[0], &cbSignature));
    signature.resize(cbSignature);
    ATL::CComPtr<IType> pMethodReturnType;
    ATL::CComPtr<IEnumTypes> pMethodParameters;
    IfFailRet(pSigParse->ParseMethodSignature(&signature[0], signature.size(), NULL, &pMethodReturnType, &pMethodParameters, NULL, NULL));

    CorElementType dwReturnType;
    IfFailRet(pMethodReturnType->GetCorElementType(&dwReturnType));
    BOOL hasReturnValue = (dwReturnType != CorElementType::ELEMENT_TYPE_VOID);
    DWORD dwMockingArgCount;
    IfFailRet(pMethodParameters->GetCount(&dwMockingArgCount));
    DWORD dwGenericArgCount = dwMockingArgCount + (hasReturnValue ? 1 : 0);

    ATL::CComPtr<ISignatureBuilder> pSigGetDelegateTemplate;
    IfFailRet(spManager->CreateSignatureBuilder(&pSigGetDelegateTemplate));
    BYTE sigGetDelegateTemplateHeader = IMAGE_CEE_CS_CALLCONV_DEFAULT | IMAGE_CEE_CS_CALLCONV_GENERIC;
    IfFailRet(pSigGetDelegateTemplate->AddData(&sigGetDelegateTemplateHeader, 1));
    IfFailRet(pSigGetDelegateTemplate->Add(1));
    IfFailRet(pSigGetDelegateTemplate->Add(1));
    IfFailRet(pSigGetDelegateTemplate->AddElementType(CorElementType::ELEMENT_TYPE_MVAR));
    IfFailRet(pSigGetDelegateTemplate->Add(0));
    IfFailRet(pSigGetDelegateTemplate->AddElementType(CorElementType::ELEMENT_TYPE_I4));
    PCCOR_SIGNATURE sigGetDelegateTemplateData;
    IfFailRet(pSigGetDelegateTemplate->GetCorSignaturePtr(&sigGetDelegateTemplateData));
    DWORD sigGetDelegateTemplateSize;
    IfFailRet(pSigGetDelegateTemplate->GetSize(&sigGetDelegateTemplateSize));

    auto spGetDelegateMethod = std::make_shared<Agent::Reflection::CWellKnownMethodInfo>(
        ExtensionsBaseAssemblyName, ExtensionsBaseModuleName, L"Microsoft.Diagnostics.Instrumentation.Extensions.Mocking.Stubs", L"GetCallback", sigGetDelegateTemplateData, sigGetDelegateTemplateSize);

    mdToken tkGetDelegateTemplate;
    IfFailRet(m_spReflectionHelper->DefineMethodToken(spModuleInfo, spGetDelegateMethod, tkGetDelegateTemplate));

    LPCWSTR delegateTypeName = GetDelegateName(hasReturnValue, dwMockingArgCount);
    if (!delegateTypeName) return E_FAIL;
    auto pDelegateTypeTemplate = std::make_shared<Agent::Reflection::CWellKnownTypeInfo>(ExtensionsBaseAssemblyName, ExtensionsBaseModuleName, delegateTypeName);

    mdToken tkDelegateTypeTemplate;
    IfFailRet(m_spReflectionHelper->DefineTypeToken(spModuleInfo, pDelegateTypeTemplate, tkDelegateTypeTemplate));

    ATL::CComPtr<IMetaDataEmit2> spMetaDataEmit;
    IfFailRet(spModuleInfo->GetMetaDataEmit(reinterpret_cast<IUnknown**>(&spMetaDataEmit)));

    ATL::CComPtr<ISignatureBuilder> pSigDelegateTypeSpecific;
    spManager->CreateSignatureBuilder(&pSigDelegateTypeSpecific);
    IfFailRet(pSigDelegateTypeSpecific->AddElementType(CorElementType::ELEMENT_TYPE_GENERICINST));
    IfFailRet(pSigDelegateTypeSpecific->AddElementType(CorElementType::ELEMENT_TYPE_CLASS));
    IfFailRet(pSigDelegateTypeSpecific->AddToken(tkDelegateTypeTemplate));
    IfFailRet(pSigDelegateTypeSpecific->Add(dwGenericArgCount));
    IfFailRet(pMethodParameters->Reset());
    for (DWORD i = 0; i < dwMockingArgCount; ++i) {
        DWORD n;
        ATL::CComPtr<IType> pArgumentType;
        IfFailRet(pMethodParameters->Next(1, &pArgumentType, &n));
        if (!n) return E_FAIL;
        IfFailRet(pArgumentType->AddToSignature(pSigDelegateTypeSpecific));
    }
    if (hasReturnValue) {
        IfFailRet(pMethodReturnType->AddToSignature(pSigDelegateTypeSpecific));
    }
    PCCOR_SIGNATURE sigDelegateTypeSpecificData;
    IfFailRet(pSigDelegateTypeSpecific->GetCorSignaturePtr(&sigDelegateTypeSpecificData));
    DWORD sigDelegateTypeSpecificSize;
    IfFailRet(pSigDelegateTypeSpecific->GetSize(&sigDelegateTypeSpecificSize));

    mdToken tkDelegateTypeSpecific;
    spMetaDataEmit->GetTokenFromTypeSpec(sigDelegateTypeSpecificData, sigDelegateTypeSpecificSize, &tkDelegateTypeSpecific);

    ATL::CComPtr<ISignatureBuilder> pSigGetDelegateSpecific;
    IfFailRet(spManager->CreateSignatureBuilder(&pSigGetDelegateSpecific));
    BYTE sigGetDelegateSpecificHeader = IMAGE_CEE_CS_CALLCONV_GENERICINST;
    IfFailRet(pSigGetDelegateSpecific->AddData(&sigGetDelegateSpecificHeader, 1));
    IfFailRet(pSigGetDelegateSpecific->Add(1));
    IfFailRet(pSigGetDelegateSpecific->AddElementType(CorElementType::ELEMENT_TYPE_GENERICINST));
    IfFailRet(pSigGetDelegateSpecific->AddElementType(CorElementType::ELEMENT_TYPE_CLASS));
    IfFailRet(pSigGetDelegateSpecific->AddToken(tkDelegateTypeTemplate));
    IfFailRet(pSigGetDelegateSpecific->Add(dwGenericArgCount));
    IfFailRet(pMethodParameters->Reset());
    for (DWORD i = 0; i < dwMockingArgCount; ++i) {
        DWORD n;
        ATL::CComPtr<IType> pArgumentType;
        IfFailRet(pMethodParameters->Next(1, &pArgumentType, &n));
        if (!n) return E_FAIL;
        IfFailRet(pArgumentType->AddToSignature(pSigGetDelegateSpecific));
    }
    if (hasReturnValue) {
        IfFailRet(pMethodReturnType->AddToSignature(pSigGetDelegateSpecific));
    }
    PCCOR_SIGNATURE sigGetDelegateSpecificData;
    IfFailRet(pSigGetDelegateSpecific->GetCorSignaturePtr(&sigGetDelegateSpecificData));
    DWORD sigGetDelegateSpecificSize;
    IfFailRet(pSigGetDelegateSpecific->GetSize(&sigGetDelegateSpecificSize));

    mdToken tkGetDelegateSpecific;
    IfFailRet(spMetaDataEmit->DefineMethodSpec(tkGetDelegateTemplate, sigGetDelegateSpecificData, sigGetDelegateSpecificSize, &tkGetDelegateSpecific));

    ATL::CComPtr<ISignatureBuilder> pSigDelegateInvokeTemplate;
    IfFailRet(spManager->CreateSignatureBuilder(&pSigDelegateInvokeTemplate));
    BYTE sigDelegateInvokeTemplateHeader = IMAGE_CEE_CS_CALLCONV_DEFAULT | IMAGE_CEE_CS_CALLCONV_HASTHIS;
    IfFailRet(pSigDelegateInvokeTemplate->AddData(&sigDelegateInvokeTemplateHeader, 1));
    IfFailRet(pSigDelegateInvokeTemplate->Add(dwMockingArgCount + 1));
    if (hasReturnValue) {
        IfFailRet(pSigDelegateInvokeTemplate->AddElementType(CorElementType::ELEMENT_TYPE_VAR));
        IfFailRet(pSigDelegateInvokeTemplate->Add(dwMockingArgCount)); // return type comes after the arguments
    }
    else {
        IfFailRet(pSigDelegateInvokeTemplate->AddElementType(CorElementType::ELEMENT_TYPE_VOID));
    }
    IfFailRet(pSigDelegateInvokeTemplate->AddElementType(CorElementType::ELEMENT_TYPE_BYREF));
    IfFailRet(pSigDelegateInvokeTemplate->AddElementType(CorElementType::ELEMENT_TYPE_BOOLEAN));
    for (DWORD i = 0; i < dwMockingArgCount; ++i) {
        IfFailRet(pSigDelegateInvokeTemplate->AddElementType(CorElementType::ELEMENT_TYPE_VAR));
        IfFailRet(pSigDelegateInvokeTemplate->Add(i));
    }
    PCCOR_SIGNATURE sigDelegateInvokeTemplateData;
    IfFailRet(pSigDelegateInvokeTemplate->GetCorSignaturePtr(&sigDelegateInvokeTemplateData));
    DWORD sigDelegateInvokeTemplateSize;
    IfFailRet(pSigDelegateInvokeTemplate->GetSize(&sigDelegateInvokeTemplateSize));

    mdToken tkDelegateInvokeSpecific;
    spMetaDataEmit->DefineMemberRef(tkDelegateTypeSpecific, L"Invoke", sigDelegateInvokeTemplateData, sigDelegateInvokeTemplateSize, &tkDelegateInvokeSpecific);

    // Create instructions
    IInstructionFactorySptr spInstructionFactory;
    IfFailRet(spMethodInfo->GetInstructionFactory(&spInstructionFactory));

    IInstructionGraphSptr spInstructionGraph;
    IfFailRet(spMethodInfo->GetInstructions(&spInstructionGraph));

    IInstructionSptr spMethodStart;
    IfFailRet(spInstructionGraph->GetFirstInstruction(&spMethodStart));

    ILocalVariableCollectionSptr spLocals;
    IfFailRet(spMethodInfo->GetLocalVariables(&spLocals));

    DWORD dwHandledLocal;
    IfFailRet(spLocals->AddLocal(spTypeBool, &dwHandledLocal));

    IInstructionSptr spLoadCallbackId;
    IfFailRet(spInstructionFactory->CreateLoadConstInstruction(CallbackId, &spLoadCallbackId));
    IfFailRet(spInstructionGraph->InsertBefore(spMethodStart, spLoadCallbackId));

    IInstructionSptr spPopGetDelegateOrResult;
    IfFailRet(spInstructionFactory->CreateInstruction(ILOrdinalOpcode::Cee_Pop, &spPopGetDelegateOrResult));
    IfFailRet(spInstructionGraph->InsertBefore(spMethodStart, spPopGetDelegateOrResult));

    IInstructionSptr spCallGetDelegate;
    IfFailRet(spInstructionFactory->CreateTokenOperandInstruction(ILOrdinalOpcode::Cee_Call, tkGetDelegateSpecific, &spCallGetDelegate));
    IfFailRet(spInstructionGraph->InsertAfter(spLoadCallbackId, spCallGetDelegate));

    IInstructionSptr spDupGetDelegate;
    IfFailRet(spInstructionFactory->CreateInstruction(ILOrdinalOpcode::Cee_Dup, &spDupGetDelegate));
    IfFailRet(spInstructionGraph->InsertAfter(spCallGetDelegate, spDupGetDelegate));

    IInstructionSptr spCheckGetDelegate;
    IfFailRet(spInstructionFactory->CreateBranchInstruction(ILOrdinalOpcode::Cee_Brfalse, spPopGetDelegateOrResult, &spCheckGetDelegate));
    IfFailRet(spInstructionGraph->InsertAfter(spDupGetDelegate, spCheckGetDelegate));

    IInstructionSptr spLoadRefHandled;
    IfFailRet(spInstructionFactory->CreateLoadLocalAddressInstruction(dwHandledLocal, &spLoadRefHandled));
    IfFailRet(spInstructionGraph->InsertAfter(spCheckGetDelegate, spLoadRefHandled));

    IInstructionSptr spCurrent = spLoadRefHandled;
    for (USHORT i = 0; i < dwMockingArgCount; i++)
    {
        IInstructionSptr spLoadArg;
        IfFailRet(spInstructionFactory->CreateLoadArgInstruction(i, &spLoadArg));
        IfFailRet(spInstructionGraph->InsertAfter(spCurrent, spLoadArg));
        spCurrent = spLoadArg;
    }

    IInstructionSptr spCallInvokeDelegate;
    IfFailRet(spInstructionFactory->CreateTokenOperandInstruction(ILOrdinalOpcode::Cee_Callvirt, tkDelegateInvokeSpecific, &spCallInvokeDelegate));
    IfFailRet(spInstructionGraph->InsertAfter(spCurrent, spCallInvokeDelegate));

    IInstructionSptr spLoadHandled;
    IfFailRet(spInstructionFactory->CreateLoadLocalInstruction(dwHandledLocal, &spLoadHandled));
    IfFailRet(spInstructionGraph->InsertAfter(spCallInvokeDelegate, spLoadHandled));

    IInstructionSptr spCheckHandled;
    IfFailRet(spInstructionFactory->CreateBranchInstruction(ILOrdinalOpcode::Cee_Brfalse, hasReturnValue ? spPopGetDelegateOrResult : spMethodStart, &spCheckHandled));
    IfFailRet(spInstructionGraph->InsertAfter(spLoadHandled, spCheckHandled));

    IInstructionSptr spReturnResult;
    IfFailRet(spInstructionFactory->CreateInstruction(ILOrdinalOpcode::Cee_Ret, &spReturnResult));
    IfFailRet(spInstructionGraph->InsertAfter(spCheckHandled, spReturnResult));

    return S_OK;
}
