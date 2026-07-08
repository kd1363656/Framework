#include "ComputeCommandList.h"

void FWK::Graphics::ComputeCommandList::Reset(const TypeAlias::ComputeCommandAllocator& a_computeCommandAllocator)
{
	DirectAndComputeCommandListBase::Reset(a_computeCommandAllocator);
}

void FWK::Graphics::ComputeCommandList::SetupComputePipeline(const std::weak_ptr<ComputePipelineState>& a_pipelineState)
{

}
void FWK::Graphics::ComputeCommandList::SetupConstantBufferView(const D3D12_GPU_VIRTUAL_ADDRESS& a_gpuVirtualAddress, const RootSignature& a_rootSignature, const Enum::RootParameterType a_rootParameterType) const
{

}

void FWK::Graphics::ComputeCommandList::Dispatch(const UINT a_threadGroupCountX, const UINT a_threadGroupCountY, const UINT a_threadGroupCountZ)
{

}

void FWK::Graphics::ComputeCommandList::SetupRootSignature(ID3D12GraphicsCommandList6& a_commandList, ID3D12RootSignature& a_rootSignature)
{
	a_commandList.SetComputeRootSignature(&a_rootSignature);
}