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
						  TypeAlias::DSVDescriptorPool&        a_dsvDescriptorPool);

		bool Resize(const Device&			            a_device,
					const GPUMemoryAllocator&           a_gpuMemoryAllocator,
					const UINT64&			            a_retiredFenceValue,
					const UINT				            a_width,
					const UINT				            a_height,
						  TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool,
						  ResourceReleaseContext&		a_resourceReleaseContext);

		void SetCurrentResourceState(const D3D12_RESOURCE_STATES a_set) { m_currentResourceState = a_set; }

		TypeAlias::DescriptorIndex FetchVALDSVDescriptorIndex(const UINT a_arrayIndex, const UINT a_mipSlice) const;

		const auto& GetREFGPUResource() const { return m_gpuResource; }

		auto GetVALCurrentResourceState() const { return m_currentResourceState; }

		static constexpr DXGI_FORMAT k_defaultDepthStencilTextureFormat = DXGI_FORMAT_D32_FLOAT;

	private:

		bool CreateGPUResource(const GPUMemoryAllocator& a_gpuMemoryAllocator, const UINT a_width, const UINT a_height);

		bool CreateDSVList(const Device& a_device, TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool);

		bool ReserveReleaseCurrentResource(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext);

		void ReleaseCreateDSVDescriptorInndexList(TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool);

		D3D12_DEPTH_STENCIL_VIEW_DESC FetchVALDesc(const UINT a_arrayIndex, const UINT a_mipSlice) const;

		static constexpr D3D12_RESOURCE_STATES k_defaultResourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

		static constexpr UINT k_firstArrayIndex = 0U;
		static constexpr UINT k_firstMipSlice   = 0U;

		static constexpr UINT k_minimumSampleCount = 1U;

		static constexpr UINT k_singleTextureArraySize   = 1U;
		static constexpr UINT k_singleDSVArraySliceCount = 1U;
		static constexpr UINT k_singleMipLevelCount      = 1U;
		static constexpr UINT k_nonMultiSampleCount      = 1U;
		
		static constexpr UINT16 k_minimumTextureArraySize = 1U;
		static constexpr UINT16 k_minimumMipLevelCount    = 1U;

		Struct::GPUResource m_gpuResource = {};

		Struct::DepthStencilTextureSettings m_depthStencilTextureSettings = {};

		std::vector<TypeAlias::DescriptorIndex> m_dsvDescriptorIndexList = {};

		D3D12_RESOURCE_STATES m_currentResourceState = k_defaultResourceState;

		UINT m_width  = Converter::TextureBinaryConverter::k_emptyTextureWidth;
		UINT m_height = Converter::TextureBinaryConverter::k_emptyTextureHeight;
	};
}