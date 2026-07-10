#pragma once

namespace FWK::Graphics
{
	class StructuredBufferBase
	{
	public:

		         StructuredBufferBase() = default;
		virtual ~StructuredBufferBase() = default;

		StructuredBufferBase(const StructuredBufferBase&) = delete;
		StructuredBufferBase(      StructuredBufferBase&& a_other) noexcept;

		StructuredBufferBase& operator=(const StructuredBufferBase&) = delete;
		StructuredBufferBase& operator=(      StructuredBufferBase&& a_other) noexcept;

		virtual bool ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext) = 0;

		virtual void Release() = 0;

		void ReleaseImmediatelySRVDescriptorIndex(TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

		const auto& GetREFBufferGPUResource() const { return m_bufferGPUResource; }

		auto GetVALSRVDescriptorIndex() const { return m_srvDescriptorIndex; }

	protected:

		TypeAlias::DescriptorIndex CreateSRV(const Device&                              a_device,
											 const Struct::GPUResource&                 a_bufferGPUResource,
											 const UINT                                 a_elementCount,
			                                 const UINT				                    a_structureByteStride,
			                                       TypeAlias::CBVSRVUAVDescriptorPool&  a_cbvSRVUAVDescriptorPool) const;

		bool ReserveReleaseCurrentResource(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext);

		void SetBufferGPUResource(Struct::GPUResource&& a_set) { m_bufferGPUResource = std::move(a_set); }

		void SetSRVDescriptorIndex(const TypeAlias::DescriptorIndex a_set) { m_srvDescriptorIndex = a_set; }

		static constexpr UINT64 k_maxStructuredBufferElementCount = std::numeric_limits<UINT>::max();

		static constexpr UINT64 k_firstStructuredBufferElement = 0ULL;

		static constexpr UINT k_invalidElementCount         = 0U;
		static constexpr UINT k_invalidStructuredByteStride = 0U;

	private:

		void MoveFrom(StructuredBufferBase&& a_other) noexcept;

		Struct::GPUResource m_bufferGPUResource = {};

		TypeAlias::DescriptorIndex m_srvDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;
	};
}