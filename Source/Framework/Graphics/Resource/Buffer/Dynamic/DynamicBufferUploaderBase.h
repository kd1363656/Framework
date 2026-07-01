#pragma once

namespace FWK::Graphics
{
	class DynamicBufferUploaderBase
	{
	public:

		explicit DynamicBufferUploaderBase(const UINT64& a_typeSize);
		virtual ~DynamicBufferUploaderBase();

		void		 Deserialize(const nlohmann::json& a_rootJson);
		virtual bool Create	    (const Device&		   a_device) = 0;

		void BeginFrame();
		
		nlohmann::json Serialize() const;
		
		void SetCreateCount(const UINT64& a_set) { m_createCount = a_set; }

		static constexpr auto& GetREFInvalidCreateCount() { return k_invalidCreateCount; }
		
		const auto& GetREFTypeSize() const { return k_typeSize; }

		const auto& GetREFCreateCount() const { return m_createCount; }

	protected:

		bool CreateUploadBuffer(const Device& a_device, const UINT64& a_alignment);

		UINT64 AllocateElementRange(const UINT64& a_elementCount);

		template <typename Type>
		D3D12_GPU_VIRTUAL_ADDRESS WriteElementAndAdvance(const Type& a_data)
		{
			const auto& l_startElementIndex = AllocateElementRange(k_singleElementCount);

			return WriteElementAt(a_data, l_startElementIndex);
		}

		template <typename Type>
		D3D12_GPU_VIRTUAL_ADDRESS WriteElementAt(const Type& a_data, const UINT64& a_elementIndex)
		{
			auto* const l_mappedData = m_uploadBuffer.FetchPTRMappedData();

			FWK_ASSERT_RETURN_VALUE_IF(!l_mappedData, "Mapデータが無効のため、書き込み処理に失敗しました。", Constant::k_invalidGPUVirtualAddress);

			const auto& l_byteOffset = a_elementIndex * m_elementStrideSize;

			std::memcpy(l_mappedData + l_byteOffset, &a_data, sizeof(Type));

			return m_uploadBuffer.FetchVALGPUVirtualAddress() + l_byteOffset;
		}

		template <typename Type>
		D3D12_GPU_VIRTUAL_ADDRESS WriteContiguousElementListAndAdvance(const std::vector<Type>& a_dataList)
		{
			FWK_ASSERT_RETURN_VALUE_IF(a_dataList.empty(),                  "DataListが空のため、連続ElementList書き込みに失敗しました。",                        Constant::k_invalidGPUVirtualAddress);
			FWK_ASSERT_RETURN_VALUE_IF(m_elementStrideSize != sizeof(Type), "ElementSizeとTypeSizeが一致していないため、連続ElementList書き込みに失敗しました。", Constant::k_invalidGPUVirtualAddress);

			const auto& l_startElementIndex = AllocateElementRange(a_dataList.size());

			auto* const l_mappedData = m_uploadBuffer.FetchPTRMappedData();

			FWK_ASSERT_RETURN_VALUE_IF(!l_mappedData, "Mapデータが無効のため、書き込み処理に失敗しました。", Constant::k_invalidGPUVirtualAddress);

			const auto l_byteOffset = l_startElementIndex * m_elementStrideSize;
			const auto l_writeSize  = sizeof(Type)        * a_dataList.size();

			std::memcpy(l_mappedData + l_byteOffset, a_dataList.data(), l_writeSize);

			return m_uploadBuffer.FetchVALGPUVirtualAddress() + l_byteOffset;
		}

	private:

		static constexpr UINT64 k_invalidCreateCount        = 0ULL;
		static constexpr UINT64 k_initialElementBufferIndex = 0ULL;
		static constexpr UINT64 k_initialElementStrideSize  = 0ULL;
		static constexpr UINT64 k_invalidElementBufferIndex = std::numeric_limits<UINT64>::max();
		static constexpr UINT64 k_singleElementCount		= 1ULL;

		const UINT64 k_typeSize;

		UploadBuffer m_uploadBuffer;

		Converter::DynamicBufferUploaderBaseJsonConverter m_jsonConverter = {};

		UINT64 m_createCount;

		UINT64 m_currentElementIndex;
		UINT64 m_elementStrideSize;

		FWK_DEFINE_TYPE_INFO_ROOT(DynamicBufferUploaderBase)
	};
}