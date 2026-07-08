#pragma once

namespace FWK::Graphics
{
	class ReadWriteStructuredBuffer final
	{
	public:

		 ReadWriteStructuredBuffer();
		~ReadWriteStructuredBuffer();

		ReadWriteStructuredBuffer(const ReadWriteStructuredBuffer&) = delete;
		ReadWriteStructuredBuffer(      ReadWriteStructuredBuffer&& a_other) noexcept;

		ReadWriteStructuredBuffer& operator=(const ReadWriteStructuredBuffer&) = delete;
		ReadWriteStructuredBuffer& operator=(      ReadWriteStructuredBuffer&& a_other) noexcept;

		template <typename Type>
		bool Create(const Device&                             a_device, 
					const GPUMemoryAllocator&                 a_gpuMemoryAllocator, 
			        const std::size_t&                        a_elementCount,
						  TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
		{
			FWK_ASSERT_RETURN_VALUE_IF(a_elementCount == k_invalidElementCount,                    "ElementCountが0のため、ReadWriteStructuredBufferの作成に失敗しました。",                   false);
			FWK_ASSERT_RETURN_VALUE_IF(a_elementCount > std::numeric_limits<UINT>::max(),          "ElementCountがUINTの最大値を超えたため、ReadWriteStructuredBufferの作成に失敗しました。",  false);
			FWK_ASSERT_RETURN_VALUE_IF(sizeof(Type) >   std::numeric_limits<UINT>::max(),          "1要素のサイズがUINTの最大値を超えたため、ReadWriteStructuredBufferの作成に失敗しました。", false);
			FWK_ASSERT_RETURN_VALUE_IF(m_bufferGPUResource.m_resource,                             "ReadWriteStructuredBufferは既にGPUResourceを保持しているため、再作成できません。",         false);
			FWK_ASSERT_RETURN_VALUE_IF(m_srvDescriptorIndex != Constant::k_invalidDescriptorIndex, "ReadWriteStructuredBufferは既にSRVDescriptorIndexを保持しているため、再作成できません。",  false);
			FWK_ASSERT_RETURN_VALUE_IF(m_uavDescriptorIndex != Constant::k_invalidDescriptorIndex, "ReadWriteStructuredBufferは既にUAVDescriptorIndexを保持しているため、再作成できません。",  false);

			const auto& l_structuredByteStride = sizeof(Type);
			const auto& l_bufferSize           = a_elementCount * l_structuredByteStride;

			FWK_ASSERT_RETURN_VALUE_IF(l_bufferSize == Constant::k_invalidBufferSize, "BufferSizeが0のため、ReadWriteStructuredBufferの作成に失敗しました。", false);

			Struct::GPUResource l_bufferGPUResouce = {};

			// UAVとしてアクセスできるバッファーを作成
			FWK_ASSERT_RETURN_VALUE_IF(!a_gpuMemoryAllocator.CreateBufferResource(l_bufferSize, 
																				  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, 
																				  D3D12_RESOURCE_STATE_COMMON, 
																				  l_bufferGPUResource),
																				  "ReadWriteStructuredBuffer用GPUResourceの作成に失敗しました。", 
																				  false);

		

		}

	private:

		static constexpr std::size_t k_invalidElementCount = 0ULL;

		static constexpr UINT k_invalidStructuredByteStride = 0ULL;

		TypeAlias::DescriptorIndex m_srvDescriptorIndex = Constant::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_uavDescriptorIndex = Constant::k_invalidDescriptorIndex;

		UINT m_elementCount = static_cast<UINT>(k_invalidElementCount);

		UINT m_structuredBytesStride = k_invalidStructuredByteStride;
	};
}