#pragma once

namespace FWK::Graphics
{
	class DirectCommandList final : public DirectAndComputeCommandListBase<D3D12_COMMAND_LIST_TYPE_DIRECT>
	{
	public:

		 DirectCommandList()          = default;
		~DirectCommandList() override = default;

		void Reset(const TypeAlias::DirectCommandAllocator& a_directCommandAllocator);
		
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

		void SetupRenderPipeline(const std::weak_ptr<GraphicsPipelineStateBase>& a_pipelineState);

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

	protected:

		void SetupRootSignature(ID3D12GraphicsCommandList6& a_commandList, ID3D12RootSignature& a_rootSignature) override;

	private:

		static constexpr UINT k_singleSetupRenderTargetNUM = 1U;
		static constexpr UINT k_allRECTClear			   = 0U;

		static constexpr UINT k_setViewportNUM       = 1U;
		static constexpr UINT k_setScissorRectNUM    = 1U;

		static constexpr UINT k_invalidSizeInBytes		  = 0U;
		static constexpr UINT k_vertexBufferViewStartSlot = 0U;
		static constexpr UINT k_vertexBufferViewCount     = 1U;

		static constexpr UINT k_invalidIndexCount    = 0U;
		static constexpr UINT k_invalidInstanceCount = 0U;
	};
}