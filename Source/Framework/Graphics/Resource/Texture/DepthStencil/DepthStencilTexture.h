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

		bool Create(const Device&					    a_device,
					const GPUMemoryAllocator&		    a_gpuMemoryAllocator,
					const DXGI_FORMAT				    a_format,
					const FLOAT						    a_depthClearValue,
					const UINT						    a_width,
					const UINT						    a_height,
					const UINT8						    a_stencilClearValue,
						  TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool);

		bool Resize(const Device&			            a_device,
					const GPUMemoryAllocator&           a_gpuMemoryAllocator,
					const UINT64&			            a_retiredFenceValue,
					const UINT				            a_width,
					const UINT				            a_height,
						  TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool,
						  ResourceReleaseContext&		a_resourceReleaseContext);

		void SetCurrentResourceState(const D3D12_RESOURCE_STATES a_set) { m_currentResourceState = a_set; }

		const auto& GetREFGPUResource() const { return m_gpuResource; }

		auto GetVALCurrentResourceState() const { return m_currentResourceState; }

		auto GetVALFormat() const { return m_format; }

		auto GetVALWidth () const { return m_width; }
		auto GetVALHeight() const { return m_height; }

		auto GetVALDepthClearValue  () const { return m_depthClearValue; }
		auto GetVALStencilClearValue() const { return m_stencilClearValue; }

		auto GetVALDSVDescriptorIndex() const { return m_dsvDescriptorIndex; }

		static constexpr DXGI_FORMAT k_defaultDepthStencilTextureFormat = DXGI_FORMAT_D32_FLOAT;

	private:

		bool CreateGPUResource(const GPUMemoryAllocator& a_gpuMemoryAllocator, const UINT a_width, const UINT a_height);

		bool CreateDSV(const Device& a_device, TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool);

		bool ReserveReleaseCurrentResource(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext);

		static constexpr D3D12_RESOURCE_STATES k_defaultResourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

		Struct::GPUResource m_gpuResource = {};

		D3D12_RESOURCE_STATES m_currentResourceState = k_defaultResourceState;

		DXGI_FORMAT m_format = k_defaultDepthStencilTextureFormat;

		FLOAT m_depthClearValue = Constant::k_defaultDepthClearValue;

		UINT m_width  = Converter::TextureBinaryConverter::k_emptyTextureWidth;
		UINT m_height = Converter::TextureBinaryConverter::k_emptyTextureHeight;

		UINT8 m_stencilClearValue = Constant::k_defaultStencilClearValue;

		TypeAlias::DescriptorIndex m_dsvDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;
	};
}