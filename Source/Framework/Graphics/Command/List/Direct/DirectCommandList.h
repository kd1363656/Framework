#pragma once

namespace FWK::Graphics
{
	class DirectCommandList final : public CommandListBase
	{
	public:

		 DirectCommandList();
		~DirectCommandList() override;

		void TransitionResourceBarrier(const TypeAlias::ComPtr<ID3D12Resource>& a_resource, const D3D12_RESOURCE_STATES a_beforeState, const D3D12_RESOURCE_STATES a_afterState) const;

		void FlushResourceBarrierTransitionBatch();

		void AddTransitionResourceBarrier(const TypeAlias::ComPtr<ID3D12Resource>& a_resource, const D3D12_RESOURCE_STATES& a_beforeState, const D3D12_RESOURCE_STATES& a_afterState);

		void SetupRenderTarget(const TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, const UINT a_rtvDescriptorIndex) const;

		void ClearRenderTarget(const TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, const UINT a_rtvDescriptorIndex, const TypeAlias::Math::Color& a_clearColor = Constant::k_whiteColor) const;

	private:

		static constexpr UINT k_singleSetupBarrierNUM      = 1U;
		static constexpr UINT k_singleSetupRenderTargetNUM = 1U;
		static constexpr UINT k_allRECTClear			   = 0U;

		std::vector<D3D12_RESOURCE_BARRIER> m_resourceBarrierTransitionBatchList;
	};
}