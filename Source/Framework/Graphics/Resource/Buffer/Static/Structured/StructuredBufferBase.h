#pragma once

namespace FWK::Graphics
{
	class StructuredBufferBase
	{
	public:

		         StructuredBufferBase() = default;
		virtual ~StructuredBufferBase() = default;

		StructuredBufferBase(const StructuredBufferBase&)  = delete;
		StructuredBufferBase(      StructuredBufferBase&&) noexcept;
		
		StructuredBufferBase& operator=(const StructuredBufferBase&) = delete;
		StructuredBufferBase& operator=(      StructuredBufferBase&&) noexcept;

		const auto& GetREFBufferGPUResource() const { return m_bufferGPUResource; }

		auto GetVALSRVDescriptorIndex() const { return m_srvDescriptorIndex; }

	protected:

		TypeAlias::DescriptorIndex CreateStructuredBufferSRV(const Device&              a_device,
															 const Struct::GPUResource& a_bufferGPUResource,
			                                                 const UINT                 a_elementCount, 
			                                                 const UINT                 a_structuredByteStride);

		bool ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext);

		void SetBufferGPUResource(Struct::GPUResource&& a_set) { m_bufferGPUResource = std::move(a_set); }

		void SetSRVDescriptorIndex(const TypeAlias::DescriptorIndex a_set) { m_srvDescriptorIndex = a_set; }

		static constexpr auto& GetVAlFirsttStructuredBufferElement() { return k_firstStructuredBufferElement; }

	private:

		void MoveFrom(StructuredBufferBase&& a_other) noexcept;

		static constexpr UINT64 k_firstStructuredBufferElement = 0ULL;

		Struct::GPUResource m_bufferGPUResource = {};

		TypeAlias::DescriptorIndex m_srvDescriptorIndex = Constant::k_invalidDescriptorIndex;
	};
}