#pragma once

namespace FWK::Graphics
{
	class DirectCommandList final : public CommandListBase
	{
	public:

		 DirectCommandList();
		~DirectCommandList() override;

		void TransitionResource(const TypeAlias::ComPtr<ID3D12Resource>& a_resource, const D3D12_RESOURCE_STATES a_beforeState, const D3D12_RESOURCE_STATES a_afterState) const;

		void AddTransitionResourceBarrier(const TypeAlias::ComPtr<ID3D12Resource>& a_resource, const D3D12_RESOURCE_STATES& a_beforeState, const D3D12_RESOURCE_STATES& a_afterState);

		void FlushResourceBarrierTransitionBatch();

	private:

		static constexpr UINT k_sendBarrierNUM = 1U;

		std::vector<D3D12_RESOURCE_BARRIER> m_resourceBarrierTransitionBatchList;
	};
}