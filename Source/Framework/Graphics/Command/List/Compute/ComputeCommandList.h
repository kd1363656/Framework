#pragma once

namespace FWK::Graphics
{
	class ComputeCommandList final : public DirectAndComputeCommandListBase<D3D12_COMMAND_LIST_TYPE_COMPUTE>
	{
	public:

		 ComputeCommandList()          = default;
		~ComputeCommandList() override = default;

		void Reset(const TypeAlias::ComputeCommandAllocator& a_computeCommandAllocator);

		void SetupComputePipeline(const std::weak_ptr<ComputePipelineState>& a_pipelineState);

		void SetupConstantBufferView(const D3D12_GPU_VIRTUAL_ADDRESS& a_gpuVirtualAddress, const RootSignature& a_rootSignature, const Enum::RootParameterType a_rootParameterType) const override;

		void Dispatch(const UINT a_threadGroupCountX, const UINT a_threadGroupCountY, const UINT a_threadGroupCountZ);

	protected:

		void SetupRootSignature(ID3D12GraphicsCommandList6& a_commandList, ID3D12RootSignature& a_rootSignature) override;
	};
}