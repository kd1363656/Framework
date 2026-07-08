#include "ComputeCommandList.h"

void FWK::Graphics::ComputeCommandList::Reset(const TypeAlias::ComputeCommandAllocator& a_computeCommandAllocator)
{
	DirectAndComputeCommandListBase::Reset(a_computeCommandAllocator);
}

void FWK::Graphics::ComputeCommandList::SetupRootSignature(ID3D12GraphicsCommandList6& a_commandList, ID3D12RootSignature& a_rootSignature)
{
	a_commandList.SetComputeRootSignature(&a_rootSignature);
}