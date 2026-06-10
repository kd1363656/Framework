#include "RootSignature.h"

void FWK::Graphics::RootSignature::SetCreateDesc(const Struct::RootSignatureCreateDesc& a_createDesc)
{
	m_createDesc = a_createDesc;

	BuildRootParameterIndexMap();
}

void FWK::Graphics::RootSignature::BuildRootParameterIndexMap()
{
}

UINT FWK::Graphics::RootSignature::FindVALRootParameterIndex(Enum::RootParameterSlot a_rootParameterSlot) const
{
	return 0;
}