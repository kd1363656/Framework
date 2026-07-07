#pragma once

namespace FWK::Graphics
{
	class ComputeCommandList final : public DirectAndComputeCommandListBase<D3D12_COMMAND_LIST_TYPE_COMPUTE>
	{
	public:

		 ComputeCommandList()          = default;
		~ComputeCommandList() override = default;

		void Reset(const TypeAlias::ComputeCommandAllocator& a_computeCommandAllocator);
	};
}