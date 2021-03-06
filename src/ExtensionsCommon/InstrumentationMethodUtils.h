// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

inline HRESULT IsValidMethodInfo(const IMethodInfoSptr& spMethodInfo)
{
	mdMethodDef tkMethod = mdMethodDefNil;
	auto hr = spMethodInfo->GetMethodToken(&tkMethod);
	if (SUCCEEDED(hr))
	{
		return mdMethodDefNil == tkMethod ? S_FALSE : S_OK;
	}

	return hr;
}