#pragma once

namespace FWK::Graphics
{
	class DirectCommandList final : public CommandListBase
	{
	public:

		 DirectCommandList();
		~DirectCommandList() override;

		void TransitionResourceBarrier(const TypeAlias::ComPtr<ID3D12Resource2>& a_resource, const D3D12_RESOURCE_STATES a_beforeState, const D3D12_RESOURCE_STATES a_afterState) const;

		void FlushResourceBarrierTransitionBatch();

		void SetupRenderTarget(const TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, const UINT a_rtvDescriptorIndex)																					   const;
		void ClearRenderTarget(const TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, const UINT a_rtvDescriptorIndex, const TypeAlias::Math::Color& a_clearColor = Constant::k_defaultBackBufferClearColor) const;

		void SetupRenderArea(const RenderArea& a_renderArea) const;

		template <D3D12_DESCRIPTOR_HEAP_TYPE Type>
		void SetupDescriptorHeap(const DescriptorPool<Type>& a_descriptorPool) const
		{
			const auto& l_directCommandList = GetREFCommandList();

			FWK_ASSERT_RETURN_IF_FAILED(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、ディスクリプタヒープの設定ができませんでした。");

			const auto& l_shaderVisibleDescriptorHeap = a_descriptorPool.GetREFShaderVisibleDescriptorHeap();

			FWK_ASSERT_RETURN_IF_FAILED(!l_shaderVisibleDescriptorHeap, "ShaderVisibleなディストラクタヒープが無効になっており、ディスクリプタヒープの設定ができませんでした。");

			const auto& l_descriptorHeap = l_shaderVisibleDescriptorHeap->GetREFDescriptorHeap();

			FWK_ASSERT_RETURN_IF_FAILED(!l_descriptorHeap, "ShaderVisibleなディスクリプタヒープが作成されておらず、ディスクリプタヒープの設定ができませんでした。");

			ID3D12DescriptorHeap* l_descriptorHeapList[] =
			{
				l_descriptorHeap.Get()
			};

			// シェーダーから参照するDescriptorHeapを設定する
			// SetDescriptorHeap(設定するヒープ数,
			//					 ヒープ配列の先頭アドレス);
			l_directCommandList->SetDescriptorHeaps(k_setDescriptorHeapNUM, l_descriptorHeapList);
		}

		void SetupRenderPipeline(const std::weak_ptr<PipelineState>& a_pipelineState) const;

		void SetupConstantBufferView(const D3D12_GPU_VIRTUAL_ADDRESS& a_gpuVirtualAddress, const RootSignature& a_rootSignature, const Enum::RootParameterType a_rootParameterType) const;

		void DispatchMesh(const UINT a_threadCountGroupX, const UINT a_threadCountGroupY, const UINT a_threadCountGroupZ) const;

		void AddTransitionResourceBarrier(const TypeAlias::ComPtr<ID3D12Resource2>& a_resource, const D3D12_RESOURCE_STATES& a_beforeState, const D3D12_RESOURCE_STATES& a_afterState);

	private:

		static constexpr UINT k_singleSetupBarrierNUM      = 1U;
		static constexpr UINT k_singleSetupRenderTargetNUM = 1U;
		static constexpr UINT k_allRECTClear			   = 0U;

		static constexpr UINT k_setViewportNUM       = 1U;
		static constexpr UINT k_setScissorRectNUM    = 1U;

		static constexpr UINT k_setDescriptorHeapNUM = 1U;

		std::vector<D3D12_RESOURCE_BARRIER> m_resourceBarrierTransitionBatchList;
	};
}