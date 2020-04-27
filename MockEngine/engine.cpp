#include "main.h"
#include "engine.h"

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
    //return pThis->Attach(modulePathPtr, modulePtr, classGuidPtr, priority);
    return E_NOTIMPL;
}

STDMETHODIMP CMockingEngine::Initialize(IProfilerManager* pProfilerManager)
{
	HRESULT hr = S_OK;

	if (!pProfilerManager)
		return E_INVALIDARG;

	if (FAILED(hr = pProfilerManager->GetLoggingInstance(&mLogger)))
		return E_FAIL;

	ComPtr<IUnknown> pProfilerUnk;
	if (FAILED(hr = pProfilerManager->GetCorProfilerInfo(&pProfilerUnk)))
		return E_FAIL;

	// we require .NET 4.0 or later
	if (FAILED(hr = pProfilerUnk.As(&mProfiler)))
		return E_FAIL;

	ThreadID thread;
	if (FAILED(hr = mProfiler->GetCurrentThreadID(&thread)) ||
		FAILED(hr = mProfiler->GetThreadAppDomain(thread, &mAttachedAppDomainId)))
		mAttachedAppDomainId = 0;

	ComPtr<IAppDomainCollection> pAppDomainList;
	if (FAILED(hr = pProfilerManager->GetAppDomainCollection(&pAppDomainList)))
		return E_FAIL;

	ComPtr<IEnumAppDomainInfo> pAppDomainIter;
	if (FAILED(hr = pAppDomainList->GetAppDomains(&pAppDomainIter)))
		return E_FAIL;

	for (;;)
	{
		ULONG n;
		ComPtr<IAppDomainInfo> pAppDomain;
		if (FAILED(hr = pAppDomainIter->Next(1, &pAppDomain, &n)) || !n || !pAppDomain)
			break;

		BOOL shared;
		if (FAILED(hr = pAppDomain->GetIsSharedDomain(&shared)))
			return E_FAIL;

		if (shared)
		{
			if (FAILED(hr = pAppDomain->GetAppDomainId(&mSharedAppDomainId)))
				return E_FAIL;

			break;
		}
	}

	return S_OK;
}

STDMETHODIMP CMockingEngine::OnAppDomainCreated(IAppDomainInfo* pAppDomainInfo)
{
	HRESULT hr = S_OK;

	if (!pAppDomainInfo)
		return E_INVALIDARG;

	BOOL shared;
	if (FAILED(hr = pAppDomainInfo->GetIsSharedDomain(&shared)))
		return E_FAIL;

	if (shared && FAILED(hr = pAppDomainInfo->GetAppDomainId(&mSharedAppDomainId)))
		return E_FAIL;

	// TODO: handle app domain creation

	return S_OK;
}

STDMETHODIMP CMockingEngine::OnAppDomainShutdown(IAppDomainInfo* pAppDomainInfo)
{
	if (!pAppDomainInfo)
		return E_INVALIDARG;

	return S_OK;
}

STDMETHODIMP CMockingEngine::OnAssemblyLoaded(IAssemblyInfo* pAssemblyInfo)
{
	if (!pAssemblyInfo)
		return E_INVALIDARG;

	return S_OK;
}

STDMETHODIMP CMockingEngine::OnAssemblyUnloaded(IAssemblyInfo* pAssemblyInfo)
{
	if (!pAssemblyInfo)
		return E_INVALIDARG;

	return S_OK;
}

STDMETHODIMP CMockingEngine::OnModuleLoaded(IModuleInfo* pModuleInfo)
{
	if (!pModuleInfo)
		return E_INVALIDARG;

	return S_OK;
}

STDMETHODIMP CMockingEngine::OnModuleUnloaded(IModuleInfo* pModuleInfo)
{
	if (!pModuleInfo)
		return E_INVALIDARG;

	return S_OK;
}

STDMETHODIMP CMockingEngine::OnShutdown()
{
	return S_OK;
}

STDMETHODIMP CMockingEngine::ShouldInstrumentMethod(IMethodInfo* pMethodInfo, BOOL isRejit, BOOL* pbInstrument)
{
	HRESULT hr = S_OK;

	if (!pMethodInfo)
		return E_INVALIDARG;

	if (!pbInstrument)
		return E_INVALIDARG;

	mdMethodDef tkMethod = mdMethodDefNil;
	if (FAILED(hr = pMethodInfo->GetMethodToken(&tkMethod)))
		return E_FAIL;

	if (tkMethod == mdMethodDefNil)
		return S_FALSE;

	// TODO: detect if we need to instrument the method
	*pbInstrument = FALSE;

	return S_OK;
}

STDMETHODIMP CMockingEngine::BeforeInstrumentMethod(IMethodInfo* pMethodInfo, BOOL isRejit)
{
	if (!pMethodInfo)
		return E_INVALIDARG;

	return S_OK;
}

STDMETHODIMP CMockingEngine::InstrumentMethod(IMethodInfo* pMethodInfo, BOOL isRejit)
{
	if (!pMethodInfo)
		return E_INVALIDARG;

	return S_OK;
}

STDMETHODIMP CMockingEngine::OnInstrumentationComplete(IMethodInfo* pMethodInfo, BOOL isRejit)
{
	if (!pMethodInfo)
		return E_INVALIDARG;

	return S_OK;
}

STDMETHODIMP CMockingEngine::AllowInlineSite(IMethodInfo* pMethodInfoInlinee, IMethodInfo* pMethodInfoCaller, BOOL* pbAllowInline)
{
	if (!pMethodInfoInlinee || !pMethodInfoCaller || !pbAllowInline)
		return E_INVALIDARG;

	*pbAllowInline = TRUE;
	return S_OK;
}
