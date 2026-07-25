#pragma once

namespace FWK::Graphics
{
	class DepthStencilTexture final
	{
	public:
	
		 DepthStencilTexture() = default;
		~DepthStencilTexture() = default;
	
		DepthStencilTexture(const DepthStencilTexture&)			  = delete;
		DepthStencilTexture(	  DepthStencilTexture&&) noexcept = default;
		
		DepthStencilTexture& operator=(const DepthStencilTexture&)			 = delete;
		DepthStencilTexture& operator=(		 DepthStencilTexture&&) noexcept = default;

		bool Create(const Device&					           a_device,
					const GPUMemoryAllocator&		           a_gpuMemoryAllocator,
					const Struct::DepthStencilTextureSettings& a_depthStencilTextureSettings,
					const UINT						           a_width,
					const UINT						           a_height,
						  TypeAlias::DSVDescriptorPool&        a_dsvDescriptorPool,
						  TypeAlias::CBVSRVUAVDescriptorPool&  a_cbvSRVUAVDescriptorPool);

		bool Resize(const Device&			                  a_device,
					const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
					const UINT64&			                  a_retiredFenceValue,
					const UINT				                  a_width,
					const UINT				                  a_height,
						  TypeAlias::DSVDescriptorPool&       a_dsvDescriptorPool,
						  TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool,
						  ResourceReleaseContext&		      a_resourceReleaseContext);

		void SetCurrentResourceState(const D3D12_RESOURCE_STATES a_set) { m_currentResourceState = a_set; }

		TypeAlias::DescriptorIndex FetchVALDSVDescriptorIndex               (const UINT a_arrayIndex, const UINT a_mipSlice) const;
		TypeAlias::DescriptorIndex FetchVALBaseSubresourceDSVDescriptorIndex() const;

		const auto& GetREFGPUResource() const { return m_gpuResource; }

		auto GetVALCurrentResourceState() const { return m_currentResourceState; }

		auto GetVALSRVDescriptorIndex() const { return m_srvDescriptorIndex; }

	private:

		bool CreateGPUResource(const GPUMemoryAllocator& a_gpuMemoryAllocator, const UINT a_width, const UINT a_height);

		D3D12_DEPTH_STENCIL_VIEW_DESC   CreateDSVDesc(const UINT a_arrayIndex, const UINT a_mipSlice) const;
		D3D12_SHADER_RESOURCE_VIEW_DESC CreateSRVDesc()                                               const;

		bool CreateDSVList(const Device& a_device, TypeAlias::DSVDescriptorPool&       a_dsvDescriptorPool);
		bool CreateSRV    (const Device& a_device, TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

		bool ReserveReleaseCurrentResource(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext);

		void ReleaseCreatedDSVDescriptorIndexList(TypeAlias::DSVDescriptorPool&       a_dsvDescriptorPool);
		void ReleaseCreatedSRVDescriptorIndex    (TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

		static constexpr D3D12_RESOURCE_STATES k_defaultResourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

		static constexpr FLOAT k_resourceMINLODClamp = 0.0F;

		static constexpr UINT k_firstArrayIndex = 0U;
		static constexpr UINT k_firstArraySlice = 0U;
		static constexpr UINT k_firstMIPSlice   = 0U;
		static constexpr UINT k_mostDetailedMIP = 0U;
		static constexpr UINT k_planeSlice      = 0U;

		static constexpr UINT k_minimumSampleCount = 1U;

		static constexpr UINT k_singleTextureArraySize   = 1U;
		static constexpr UINT k_singleDSVArraySliceCount = 1U;
		static constexpr UINT k_singleMIPLevelCount      = 1U;
		static constexpr UINT k_nonMultisampleCount      = 1U;
		
		static constexpr UINT16 k_minimumTextureArraySize = 1U;
		static constexpr UINT16 k_minimumMIPLevelCount    = 1U;

		Struct::GPUResource m_gpuResource = {};

		Struct::DepthStencilTextureSettings m_depthStencilTextureSettings = {};

		std::vector<TypeAlias::DescriptorIndex> m_dsvDescriptorIndexList = {};

		D3D12_RESOURCE_STATES m_currentResourceState = k_defaultResourceState;

		UINT m_width  = Constant::k_invalidTextureWidth;
		UINT m_height = Constant::k_invalidTextureHeight;

		TypeAlias::DescriptorIndex m_srvDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;
	};
}