#pragma once

namespace FWK::Graphics
{
    class DynamicBufferUploaderBase
    {
    public:

        explicit DynamicBufferUploaderBase(const UINT64& a_typeSize, const bool a_shouldAdvanceWritePosition);
        virtual ~DynamicBufferUploaderBase();

        DynamicBufferUploaderBase(const DynamicBufferUploaderBase&) = delete;
        DynamicBufferUploaderBase(      DynamicBufferUploaderBase&& a_other) noexcept;

        DynamicBufferUploaderBase& operator=(const DynamicBufferUploaderBase&)           = delete;
        DynamicBufferUploaderBase& operator=(      DynamicBufferUploaderBase&&) noexcept = delete;

        void         Deserialize(const nlohmann::json& a_rootJson);
        virtual bool Create     (const Device&         a_device) = 0;

        void BeginFrame();

        nlohmann::json Serialize() const;

        void SetCreateCount(const UINT64& a_set) { m_createCount = a_set; }

        const auto& GetREFUploadBuffer() const { return m_uploadBuffer; }

        const auto& GetREFTypeSize() const { return k_typeSize; }

        const auto& GetREFCreateCount() const { return m_createCount; }

    protected:

        bool CreateUploadBuffer(const Device& a_device, const UINT64& a_alignment);

        template <typename Type>
        D3D12_GPU_VIRTUAL_ADDRESS WriteElement(const Type& a_element)
        {
            return WriteElementRange(&a_element, k_singleElementCount);
        }

        template <typename Type>
        D3D12_GPU_VIRTUAL_ADDRESS WriteElementList(const std::vector<Type>& a_elementList)
        {
            return WriteElementRange(a_elementList.data(), a_elementList.size());
        }

        static constexpr UINT64 k_singleElementCount = 1ULL;

        static constexpr bool k_advanceWritePosition = true;
        static constexpr bool k_keepWritePosition    = false;

    private:

        // WriteElement / WriteElementListの共通処理
        // 先頭アドレスと要素数で受け取り、連続した範囲をUploadBufferへまとめてコピーする
        template <typename Type>
        D3D12_GPU_VIRTUAL_ADDRESS WriteElementRange(const Type* const a_elementData, const UINT64& a_elementCount)
        {
            FWK_ASSERT_RETURN_VALUE_IF(!a_elementData,                       "ElementDataが無効のため、書き込み処理に失敗しました。", Constant::k_invalidDynamicBufferGPUVirtualAddress);
            FWK_ASSERT_RETURN_VALUE_IF(a_elementCount == k_emptyElementCount, "ElementRangeが空のため、書き込み処理に失敗しました。",   Constant::k_invalidDynamicBufferGPUVirtualAddress);

            const auto& l_remainingElementCount = m_createCount - m_currentElementIndex;

            FWK_ASSERT_RETURN_VALUE_IF(a_elementCount > l_remainingElementCount, "ElementRangeが書き込み可能な残りElement数を超えているため、書き込み処理に失敗しました。", Constant::k_invalidDynamicBufferGPUVirtualAddress);
            FWK_ASSERT_RETURN_VALUE_IF(sizeof(Type)   > m_elementStrideSize,     "書き込みTypeのサイズがElementStrideSizeを超えているため、書き込み処理に失敗しました。",   Constant::k_invalidDynamicBufferGPUVirtualAddress);

            FWK_ASSERT_RETURN_VALUE_IF(a_elementCount       > k_singleElementCount &&
                                       m_elementStrideSize != sizeof(Type),
                                       "複数Elementを連続書き込みできないElementStrideSizeのため、書き込み処理に失敗しました。",
                                       Constant::k_invalidDynamicBufferGPUVirtualAddress);

            const auto& l_startElementIndex = m_currentElementIndex;

            auto* const l_mappedData = m_uploadBuffer.FetchPTRMappedData();

            FWK_ASSERT_RETURN_VALUE_IF(!l_mappedData, "Mapデータが無効のため、書き込み処理に失敗しました。", Constant::k_invalidDynamicBufferGPUVirtualAddress);

            const auto& l_byteOffset = l_startElementIndex * m_elementStrideSize;
            const auto& l_writeSize  = sizeof(Type) * a_elementCount;

            std::memcpy(l_mappedData + l_byteOffset, a_elementData, l_writeSize);

            // 書き込みが正常に完了した後でのみ位置を進める。
            // 固定位置Uploaderでは常に先頭Elementを上書きする。
            if (k_shouldAdvanceWritePosition)
            {
                m_currentElementIndex += a_elementCount;
            }

            return m_uploadBuffer.FetchVALGPUVirtualAddress() + l_byteOffset;
        }

        static constexpr UINT64 k_emptyElementCount         = 0ULL;
        static constexpr UINT64 k_initialElementBufferIndex = 0ULL;
        static constexpr UINT64 k_initialElementStrideSize  = 0ULL;

        const UINT64 k_typeSize;

        const bool k_shouldAdvanceWritePosition;

        UploadBuffer m_uploadBuffer;

        Converter::DynamicBufferUploaderBaseJsonConverter m_jsonConverter;

        UINT64 m_createCount;

        UINT64 m_currentElementIndex;
        UINT64 m_elementStrideSize;

        FWK_DEFINE_TYPE_INFO_ROOT(DynamicBufferUploaderBase)
    };
}