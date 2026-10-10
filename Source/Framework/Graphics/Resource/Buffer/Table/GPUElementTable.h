#pragma once

namespace FWK::Graphics
{
    class GPUElementTable final : public StructuredBufferBase
    {
    public:

         GPUElementTable();
        ~GPUElementTable() override;

        GPUElementTable(const GPUElementTable&)           = delete;
        GPUElementTable(      GPUElementTable&&) noexcept = delete;

        GPUElementTable& operator=(const GPUElementTable&)           = delete;
        GPUElementTable& operator=(      GPUElementTable&&) noexcept = delete;

        bool Create(const Device&                             a_device,
                    const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
                    const std::size_t&                        a_frameCount,
                    const UINT                                a_capacity,
                    const UINT                                a_elementByteStride,
                          TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

        std::uint32_t AllocateElementIndex();

        void ReleaseElementIndex(const std::uint32_t a_elementIndex);

        template <typename ElementType>
        void WriteElement(const ElementType& a_element, const std::uint32_t a_elementIndex)
        {
            // GPUへはバイト列としてそのままコピーするため、memcpyできる単純なデータ型だけを許可する
            // (std::stringやstd::vectorのように、中身が別の場所にある型はコピーしても意味がない)
            static_assert(std::is_trivially_copyable_v<ElementType>, "GPUElementTableへ書き込む型は、triviallyCopyableである必要があります。");

            FWK_ASSERT_RETURN_IF(a_elementIndex >= m_capacity,               "書き込む番号がテーブルの容量を超えているため、テーブルへの書き込みに失敗しました。");
            FWK_ASSERT_RETURN_IF(sizeof(ElementType) != m_elementByteStride, "書き込む型の大きさがテーブルの1要素の大きさと違うため、テーブルへの書き込みに失敗しました。");

            const auto& l_byteOffset = static_cast<std::size_t>(a_elementIndex) * m_elementByteStride;

            // ここではCPU側の写しに書くだけで、GPUへはまだ送らない
            // 番号を書き換え待ちに入れておき、次のフレームのRecordUploadでまとめてGPUへコピーする
            // 同じフレームで同じ番号を何度書き換えても、コピーは1回で済む(setなので重複しない)
            // 例 : 1要素136バイトで5番を書くなら、写しの680バイト目から136バイトを書き換える
            std::memcpy(m_cpuElementDataList.data() + l_byteOffset, &a_element, sizeof(ElementType));

            m_dirtyElementIndexSet.emplace(a_elementIndex);
        }

        void RecordUpload(const DirectCommandList& a_directCommandList, const std::size_t& a_frameIndex);

        bool ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext) override;
        void Release       ()                                                                                    override;

        static constexpr std::uint32_t k_invalidElementIndex = std::numeric_limits<std::uint32_t>::max();

    private:

        static constexpr std::size_t k_emptyFrameCount = 0ULL;

        static constexpr UINT k_initialCapacity          = 0U;
        static constexpr UINT k_initialElementByteStride = 0U;

        static constexpr std::uint32_t k_initialNextElementIndex = 0U;

        std::vector<UploadBuffer> m_uploadBufferList;

        std::vector<std::uint8_t> m_cpuElementDataList;

        std::vector<std::uint32_t> m_freeElementIndexList;

        std::unordered_set<std::uint32_t> m_dirtyElementIndexSet;

        UINT m_capacity;
        UINT m_elementByteStride;

        std::uint32_t m_nextElementIndex;
    };
}