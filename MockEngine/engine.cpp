#include "main.h"
#include "engine.h"
#include "..\src\ExtensionsCommon\DataItemContainer.h"
#include "..\src\ExtensionsCommon\WellKnownInstrumentationDefs.h"
#include "..\src\ExtensionsCommon\NativeInstanceMethodInstrumentationInfo.h"

namespace Agent { namespace Interop { static const GUID GUID_MethodInfoDataItem = CLSID_MockingEngine; } } // our unique key for IDataContainer entries

EXTERN_C const GUID CLSID_MockingEngine = __uuidof(CMockingEngine);
OBJECT_ENTRY_AUTO(CLSID_MockingEngine, CMockingEngine)

static HRESULT STDMETHODCALLTYPE DecorateStub(
	_In_ CMockingEngine* pThis,
	_In_ const __int32 methodId,
	_In_ const __int64 assemblyPtr,
	_In_ const __int64 modulePtr,
	_In_ const __int64 typeNamePtr,
	_In_ const __int64 methodNamePtr,
	_In_ const __int32 argumentCount)
{
	return pThis->Decorate(methodId, assemblyPtr, modulePtr, typeNamePtr, methodNamePtr, argumentCount);
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

	*pbInstrument = (hr == S_OK);
	return S_OK;
}

HRESULT CMockingEngine::InternalInstrumentMethod(const IMethodInfoSptr& spMethodInfo, BOOL isRejit)
{
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

HRESULT CMockingEngine::Decorate(const __int32 methodId, const __int64 assemblyPtr, const __int64 modulePtr, const __int64 typeNamePtr, const __int64 methodNamePtr, const __int32 argumentCount)
{
	return S_OK;
}
