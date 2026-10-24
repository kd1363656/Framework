#pragma once

namespace FWK::Graphics
{
    // CPUが毎フレーム書き込み、シェーダーからStructuredBufferとして読むためのバッファ
    //
    // UPLOADヒープ(CPUから直接書き込めるメモリ)にバッファを作り、そこへSRVを作成する。
    // GPU用メモリへのコピーを挟まないため、毎フレーム中身が変わる小さなデータ(インスタンス情報など)に向いている。
    // ※ GPUが読んでいる最中に書き換えないよう、フレームリソースの数だけ作って使い分けること
    class DynamicUploadStructuredBuffer final : public StructuredBufferBase
    {
    public:

         DynamicUploadStructuredBuffer();
        ~DynamicUploadStructuredBuffer() override;

        DynamicUploadStructuredBuffer(const DynamicUploadStructuredBuffer&) = delete;
        DynamicUploadStructuredBuffer(      DynamicUploadStructuredBuffer&& a_other) noexcept;

        DynamicUploadStructuredBuffer& operator=(const DynamicUploadStructuredBuffer&) = delete;
        DynamicUploadStructuredBuffer& operator=(      DynamicUploadStructuredBuffer&& a_other) noexcept;

        template <typename Type>
        bool Create(const Device& a_device, const std::size_t& a_elementCount, TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
        {
            FWK_ASSERT_RETURN_VALUE_IF(a_elementCount == k_invalidElementCount,            "ElementCountが0のため、DynamicUploadStructuredBufferの作成に失敗しました。",            false);
            FWK_ASSERT_RETURN_VALUE_IF(a_elementCount > k_maxStructuredBufferElementCount, "DynamicUploadStructuredBufferの要素数がUINTの最大値を超えたため、作成に失敗しました。", false);
            FWK_ASSERT_RETURN_VALUE_IF(GetREFBufferGPUResource().m_resource,               "DynamicUploadStructuredBufferは既に作成されているため、再作成できません。",              false);

            const auto l_structureByteStride = sizeof(Type);
            const auto l_bufferSize          = l_structureByteStride * a_elementCount;

            // CPUから書き込めるUPLOADヒープのバッファを作成する(作成時にMapされ、書き込み先のアドレスを保持する)
            FWK_ASSERT_RETURN_VALUE_IF(!m_uploadBuffer.Create(a_device, l_bufferSize), "DynamicUploadStructuredBuffer用UploadBufferの作成に失敗しました。", false);

            // SRVの作成と遅延解放の仕組みは基底クラスがGPUResourceとして扱うため、同じリソースを渡しておく
            Struct::GPUResource l_bufferGPUResource = {};

            l_bufferGPUResource.m_resource = m_uploadBuffer.GetREFUploadBuffer();

            const auto l_srvDescriptorIndex = CreateSRV(a_device,
                                                        l_bufferGPUResource,
                                                        static_cast<UINT>(a_elementCount),
                                                        static_cast<UINT>(l_structureByteStride),
                                                        a_cbvSRVUAVDescriptorPool);

            FWK_ASSERT_RETURN_VALUE_IF(l_srvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "DynamicUploadStructuredBuffer用SRVの作成に失敗しました。", false);

            SetBufferGPUResource (std::move(l_bufferGPUResource));
            SetSRVDescriptorIndex(l_srvDescriptorIndex);

            m_elementCount        = static_cast<UINT>(a_elementCount);
            m_structureByteStride = static_cast<UINT>(l_structureByteStride);

            return true;
        }

        // CPUのデータを先頭から書き込む
        // 用意した要素数を超える分は書き込まない
        template <typename Type>
        bool Write(const std::vector<Type>& a_elementList)
        {
            FWK_ASSERT_RETURN_VALUE_IF(sizeof(Type) != m_structureByteStride, "書き込む型のサイズがDynamicUploadStructuredBufferの1要素サイズと一致しません。", false);
            FWK_ASSERT_RETURN_VALUE_IF(a_elementList.size() > m_elementCount, "書き込む要素数がDynamicUploadStructuredBufferの要素数を超えています。",          false);

            if (a_elementList.empty()) { return true; }

            auto* const l_mappedData = m_uploadBuffer.FetchPTRMappedData();

            FWK_ASSERT_RETURN_VALUE_IF(!l_mappedData, "UploadBufferがMapされていないため、DynamicUploadStructuredBufferへの書き込みに失敗しました。", false);

            std::memcpy(l_mappedData, a_elementList.data(), sizeof(Type) * a_elementList.size());

            return true;
        }

        bool ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext) override;
        void Release       ()                                                                                    override;

        auto GetVALElementCount() const { return m_elementCount; }

    private:

        void MoveFrom(DynamicUploadStructuredBuffer&& a_other) noexcept;

        UploadBuffer m_uploadBuffer;

        UINT m_elementCount;

        UINT m_structureByteStride;
    };
}