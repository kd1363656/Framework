#pragma once

namespace FWK::Graphics
{
	class RenderTargetTexture final
	{
	public:

		 RenderTargetTexture() = default;
		~RenderTargetTexture() = default;

		RenderTargetTexture(const RenderTargetTexture&)			  = delete;
		RenderTargetTexture(	  RenderTargetTexture&&) noexcept = default;
		
		RenderTargetTexture& operator=(const RenderTargetTexture&)			 = delete;
		RenderTargetTexture& operator=(		 RenderTargetTexture&&) noexcept = default;

		bool Create(const Device&					          a_device,
				    const GPUMemoryAllocator&		          a_gpuMemoryAllocator,
					const TypeAlias::Math::Color&		      a_clearColor,
					const DXGI_FORMAT					      a_format,
					const UINT							      a_width,
					const UINT							      a_height,
						  TypeAlias::RTVDescriptorPool&       a_rtvDescriptorPool,
						  TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

		bool Resize(const Device&						      a_device,
					const GPUMemoryAllocator&			      a_gpuMemoryAllocator,
					const UINT64&						      a_retiredFenceValue,
					const UINT							      a_width,
					const UINT							      a_height,
						  TypeAlias::RTVDescriptorPool&       a_rtvDescriptorPool,
						  TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool,
						  ResourceReleaseContext&	          a_resourceReleaseContext);

		void SetCurrentResourceState(const D3D12_RESOURCE_STATES a_set) { m_currentResourceState = a_set; }

		const auto& GetREFGPUResource() const { return m_gpuResource; }
		
		auto GetVALCurrentResourceState() const { return m_currentResourceState; }

		auto GetVALFormat() const { return m_format; }

		auto GetVALWidth () const { return m_width; }
		auto GetVALHeight() const { return m_height; }

		auto GetVALRTVDescriptorIndex() const { return m_rtvDescriptorIndex; }
		auto GetVALSRVDescriptorIndex() const { return m_srvDescriptorIndex; }

	private:

		bool CreateGPUResource(const GPUMemoryAllocator& a_gpuMemoryAllocator, const UINT						         a_width, const UINT a_height);
		bool CreateRTV        (const Device&			 a_device,					 TypeAlias::RTVDescriptorPool&       a_rtvDescriptorPool);
		bool CreateSRV        (const Device&			 a_device,					 TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

		bool ReserveReleaseCurrentResource(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext);

		static constexpr D3D12_RESOURCE_STATES k_defaultResourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;

		static constexpr FLOAT k_resourceMINLODClamp = 0.0F;

		static constexpr UINT k_clearColorIndexR = 0U;
		static constexpr UINT k_clearColorIndexG = 1U;
		static constexpr UINT k_clearColorIndexB = 2U;
		static constexpr UINT k_clearColorIndexA = 3U;

		static constexpr UINT  k_mostDetailedMIP = 0U;
		static constexpr UINT  k_planeSlice      = 0U;

		TypeAlias::Math::Color m_clearColor = Constant::k_defaultBackBufferClearColor;

		ResourceReleaseContext::GPUResource m_gpuResource = {};

		D3D12_RESOURCE_STATES m_currentResourceState = k_defaultResourceState;

		DXGI_FORMAT m_format = Constant::k_defaultRenderTargetTextureFormat;

		UINT m_width  = Constant::k_emptyTextureWidth;
		UINT m_height = Constant::k_emptyTextureHeight;

		TypeAlias::DescriptorIndex m_rtvDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_srvDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;
	};
}