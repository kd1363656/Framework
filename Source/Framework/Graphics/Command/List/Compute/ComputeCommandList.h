#pragma once

namespace FWK::Graphics
{
	class ComputeCommandList final : public DirectAndComputeCommandListBase<D3D12_COMMAND_LIST_TYPE_COMPUTE>
	{
	public:

		 ComputeCommandList()          = default;
		~ComputeCommandList() override = default;

		void Reset(const TypeAlias::ComputeCommandAllocator& a_computeCommandAllocator);

	protected:

		void SetupRootSignature(ID3D12GraphicsCommandList6& a_commandList, ID3D12RootSignature& a_rootSignature) override;
	};
}