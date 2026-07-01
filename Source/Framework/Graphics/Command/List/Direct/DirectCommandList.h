#pragma once

namespace FWK::Graphics
{
	class DirectCommandList final : public CommandListBase
	{
	public:

		 DirectCommandList();
		~DirectCommandList() override;

		void TransitionResourceBarrier(const TypeAlias::ComPtr<ID3D12Resource2>& a_resource, const D3D12_RESOURCE_STATES a_beforeState, const D3D12_RESOURCE_STATES a_afterState) const;

		void SetupRenderTarget(const TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, const UINT a_rtvDescriptorIndex)																					   const;
		void ClearRenderTarget(const TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, const UINT a_rtvDescriptorIndex, const TypeAlias::Math::Color& a_clearColor = Constant::k_defaultBackBufferClearColor) const;

		void SetupRenderTargetAndDepthStencil(const TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool,
											  const TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool,
											  const TypeAlias::DescriptorIndex	  a_rtvDescriptorIndex,
											  const TypeAlias::DescriptorIndex	  a_dsvDescriptorIndex) const;

		void ClearDepthStencil(const TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool,
							   const FLOAT						   a_depthClearValue,
							   const TypeAlias::DescriptorIndex	   a_dsvDescriptorIndex,
							   const UINT8						   a_stencilClearValue) const;

		void SetupRenderArea(const RenderArea& a_renderArea) const;

		template <D3D12_DESCRIPTOR_HEAP_TYPE Type>
		void SetupDescriptorHeap(const DescriptorPool<Type>& a_descriptorPool) const
		{
			const auto& l_directCommandList = GetREFCommandList();

			FWK_ASSERT_RETURN_IF(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、ディスクリプタヒープの設定ができませんでした。");

			const auto& l_shaderVisibleDescriptorHeap = a_descriptorPool.GetREFShaderVisibleDescriptorHeap();

			FWK_ASSERT_RETURN_IF(!l_shaderVisibleDescriptorHeap, "ShaderVisibleなディストラクタヒープが無効になっており、ディスクリプタヒープの設定ができませんでした。");

			const auto& l_descriptorHeap = l_shaderVisibleDescriptorHeap->GetREFDescriptorHeap();

			FWK_ASSERT_RETURN_IF(!l_descriptorHeap, "ShaderVisibleなディスクリプタヒープが作成されておらず、ディスクリプタヒープの設定ができませんでした。");

			ID3D12DescriptorHeap* l_descriptorHeapList[] =
			{
				l_descriptorHeap.Get()
			};

			// シェーダーから参照するDescriptorHeapを設定する
			// SetDescriptorHeap(設定するヒープ数,
			//					 ヒープ配列の先頭アドレス);
			l_directCommandList->SetDescriptorHeaps(k_setDescriptorHeapNUM, l_descriptorHeapList);
		}

		void SetupRenderPipeline(const std::weak_ptr<PipelineStateBase>& a_pipelineState);

		void SetupConstantBufferView(const D3D12_GPU_VIRTUAL_ADDRESS& a_gpuVirtualAddress, const RootSignature& a_rootSignature, const Enum::RootParameterType a_rootParameterType) const;

		void SetupPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY a_primitiveTopology) const;

		void SetupVertexBufferView(const D3D12_VERTEX_BUFFER_VIEW& a_vertexBufferView) const;
		void SetupIndexBufferView (const D3D12_INDEX_BUFFER_VIEW&  a_indexBufferView)  const;

		void DrawIndexedInstanced(const UINT a_indexCount, 
								  const UINT a_instanceCount,
								  const UINT a_startIndexLocation,
								  const UINT a_startInstanceLocation,
								  const INT  a_baseVertexLocation);

		void DrawInstanced(const UINT a_vertexCount,
						   const UINT a_instanceCount,
						   const UINT a_startVertexLocation,
					       const UINT a_startInstanceLocation) const;

		void DispatchMesh(const UINT a_threadCountGroupX, const UINT a_threadCountGroupY, const UINT a_threadCountGroupZ) const;

	private:

		template <typename Type>
		bool IsSameWeakOwner(const std::weak_ptr<Type>& a_left, const std::weak_ptr<Type>& a_right) const
		{
			return !a_left.owner_before (a_right) &&
				   !a_right.owner_before(a_left);
		}

		static constexpr UINT k_singleSetupBarrierNUM      = 1U;
		static constexpr UINT k_singleSetupRenderTargetNUM = 1U;
		static constexpr UINT k_allRECTClear			   = 0U;

		static constexpr UINT k_setViewportNUM       = 1U;
		static constexpr UINT k_setScissorRectNUM    = 1U;

		static constexpr UINT k_setDescriptorHeapNUM = 1U;

		static constexpr UINT k_invalidSizeInBytes		  = 0U;
		static constexpr UINT k_vertexBufferViewStartSlot = 0U;
		static constexpr UINT k_vertexBufferViewCount     = 1U;

		static constexpr UINT k_invalidIndexCount    = 0U;
		static constexpr UINT k_invalidInstanceCount = 0U;

		std::weak_ptr<RootSignature>     m_currentRootSignature = {};
		std::weak_ptr<PipelineStateBase> m_currentPipelineState = {};
	};
}