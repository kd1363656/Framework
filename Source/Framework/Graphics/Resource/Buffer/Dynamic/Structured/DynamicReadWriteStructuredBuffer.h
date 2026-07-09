#pragma once

namespace FWK::Graphics
{
	class DynamicReadWriteStructuredBuffer final : public StructuredBufferBase
	{
	public:

		 DynamicReadWriteStructuredBuffer();
		~DynamicReadWriteStructuredBuffer() override;

		
		DynamicReadWriteStructuredBuffer(const DynamicReadWriteStructuredBuffer&) = delete;
		DynamicReadWriteStructuredBuffer(      DynamicReadWriteStructuredBuffer&& a_other) noexcept;

		DynamicReadWriteStructuredBuffer& operator=(const DynamicReadWriteStructuredBuffer&) = delete;
		DynamicReadWriteStructuredBuffer& operator=(      DynamicReadWriteStructuredBuffer&& a_other) noexcept;

		template <typename Type>
		bool Create(const Device&                             a_device,
					const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
					const std::size_t&                        a_elementCount,
					      TypeAlias::CBVSRVUAVDescriptorPool& a_cbbSRVUAVDescriptorPool)
		{
			// DynamicReadWriteStructuredBufferは、
			// ComputeShaderなどからUAVとして書き込み、
			// そのあと、他のShaderからSRVとして読み取るためのStructuredBuffer
			FWK_ASSERT_RETURN_VALUE_IF(a_elementCount == k_invalidElementCount,                                "ElementCountが0のため、DynamicReadWriteStructuredBufferの作成に失敗しました。",                  false);
			FWK_ASSERT_RETURN_VALUE_IF(a_elementCount > k_maxStructuredBufferElementCount,                     "DynamicReadWriteStructuredBufferの要素数がUINTの最大値を超えたため、作成に失敗しました。",       false);
			FWK_ASSERT_RETURN_VALUE_IF(sizeof(Type) > std::numeric_limits<UINT>::max(),                        "DynamicReadWriteStructuredBufferの1要素サイズがUINTの最大値を超えたため、作成に失敗しました。",  false);
			FWK_ASSERT_RETURN_VALUE_IF(GetREFBufferGPUResource().m_resource,                                   "DynamicReadWriteStructuredBufferは既にGPUResourceを保持しているため、再作成できません。",        false);
			FWK_ASSERT_RETURN_VALUE_IF(GetVALSRVDescriptorIndex() != DescriptorHeap::k_invalidDescriptorIndex, "DynamicReadWriteStructuredBufferは既にSRVDescriptorIndexを保持しているため、再作成できません。", false);
			FWK_ASSERT_RETURN_VALUE_IF(m_uavDescriptorIndex != DescriptorHeap::k_invalidDescriptorIndex,       "DynamicReadWriteStructuredBufferは既にUAVDescriptorIndexを保持しているため、再作成できません。", false);

			const auto& l_elementCount         = a_elementCount;
			const auto& l_structuredByteStride = sizeof(Type);

			// StructuredBufferは、「1要素のサイズ * 要素数」分のバッファーサイズが必要、
			// TypeがMatrixで64bte、要素数が100個なら、64 * 100 = 6400byteのGPUBufferを作る
			const auto& l_bufferSize = l_structuredByteStride * l_elementCount;

			FWK_ASSERT_RETURN_VALUE_IF(l_bufferSize == UploadBuffer::k_invalidBufferSize, "DynamicReadWriteStructuredBufferの作成サイズが0のため、作成に失敗しました。", false);

			// 失敗したときに、このクラスが中途半端な状態にならないように、
			// まずはローカル変数でGPUResourceを作る
			ResourceReleaseContext::GPUResource l_bufferGPUResource = {};

			// UAVとして書き込むBufferなので、
			// D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESSを指定する
			FWK_ASSERT_RETURN_VALUE_IF(!a_gpuMemoryAllocator.CreateBufferResource(l_bufferSize,
																				  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
																				  D3D12_RESOURCE_STATE_COMMON,
																				  l_bufferGPUResource),
																				  "DynamicReadWriteStructuredBuffer用GPUResourceの作成に失敗しました。。",
																				  false);

			// Shaderから読み取るためのSRVを作成
			const auto l_srvDescriptorIndex = CreateSRV(a_device,
														l_bufferGPUResource,
														l_elementCount,
														l_structuredByteStride,
													    a_cbbSRVUAVDescriptorPool);

			FWK_ASSERT_RETURN_VALUE_IF(l_srvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "DynamicReadWriteStructuredBuffer用SRVの作成に失敗しました。", false);

			// ComputeShaderなどから書き込むためのUAVを作る
			const auto l_uavDescriptorInde;
		}

		bool ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext) override;
		void Release       ()                                                                                    override;

		void ReleaseImmediatelyUAVDescriptorIndex(TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);
		void ReleaseImmediatelyDescriptorIndecies(TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

		void SetCurrentResourceState(const D3D12_RESOURCE_STATES a_set) { m_currentResourceState = a_set; }

		auto GetVALUAVDescriptorIndex() const { return m_uavDescriptorIndex; }

		auto GetVALUAVDescirptorIndex() const { return m_uavDescriptorIndex; }

		auto GetVALCurrentResourceState() const { return m_currentResourceState; }

		auto GetVALElementCount() const { return m_elementCount; }

		auto GetVALStructuredByteStride() const { return m_structureByteStride; }

	private:

		TypeAlias::DescriptorIndex CreateUAV(const Device&                              a_device,
											 const ResourceReleaseContext::GPUResource& a_bufferGPUResource,
											 const UINT								    a_elementCount,
											 const UINT									a_structureByteStride,
											       TypeAlias::CBVSRVUAVDescriptorPool&  a_cbvSRVUAVDescriptorPool) const;

		void MoveFrom(DynamicReadWriteStructuredBuffer&& a_other) noexcept;

		static constexpr UINT64 k_counterOffsetInBytes = 0ULL;

		TypeAlias::DescriptorIndex m_uavDescriptorIndex;

		D3D12_RESOURCE_STATES m_currentResourceState;

		UINT m_elementCount;

		UINT m_structureByteStride;
	};
}