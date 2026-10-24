#include "DynamicUploadStructuredBuffer.h"

FWK::Graphics::DynamicUploadStructuredBuffer::DynamicUploadStructuredBuffer() :
    m_uploadBuffer(),

    m_elementCount(k_invalidElementCount),

    m_structureByteStride(k_invalidStructuredByteStride)
{}
FWK::Graphics::DynamicUploadStructuredBuffer::~DynamicUploadStructuredBuffer()
{
    Release();
}

FWK::Graphics::DynamicUploadStructuredBuffer::DynamicUploadStructuredBuffer(DynamicUploadStructuredBuffer&& a_other) noexcept :
    StructuredBufferBase(std::move(a_other)),

    m_uploadBuffer(),

    m_elementCount(k_invalidElementCount),

    m_structureByteStride(k_invalidStructuredByteStride)
{
    MoveFrom(std::move(a_other));
}
FWK::Graphics::DynamicUploadStructuredBuffer& FWK::Graphics::DynamicUploadStructuredBuffer::operator=(DynamicUploadStructuredBuffer&& a_other) noexcept
{
    if (this == &a_other) { return *this; }

    Release();

    StructuredBufferBase::operator=(std::move(a_other));

    MoveFrom(std::move(a_other));

    return *this;
}

bool FWK::Graphics::DynamicUploadStructuredBuffer::ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext)
{
    // GPUが使い終わるまでリソースとSRVの解放を待つように登録する
    FWK_ASSERT_RETURN_VALUE_IF(!ReserveReleaseCurrentResource(a_retiredFenceValue, a_resourceReleaseContext), "DynamicUploadStructuredBufferの遅延解放登録に失敗しました。", false);

    // 自分が持つUploadBufferはUnmapして手放す
    // リソース本体は遅延解放の登録側が参照を持っているため、GPUが使い終わるまで残る
    m_uploadBuffer = UploadBuffer();

    m_elementCount        = k_invalidElementCount;
    m_structureByteStride = k_invalidStructuredByteStride;

    return true;
}
void FWK::Graphics::DynamicUploadStructuredBuffer::Release()
{
    if (!GetREFBufferGPUResource().m_resource &&
        GetVALSRVDescriptorIndex() == DescriptorHeap::k_invalidDescriptorIndex)
    {
        return;
    }

    auto& l_graphicsManager = FWK::Graphics::GraphicsManager::GetInstance();

    const auto& l_renderer           = l_graphicsManager.GetREFRenderer   ();
    const auto& l_directCommandQueue = l_renderer.GetREFDirectCommandQueue();

    auto& l_resourceContext        = l_graphicsManager.GetMutableREFResourceContext       ();
    auto& l_resourceReleaseContext = l_resourceContext.GetMutableREFResourceReleaseContext();

    const auto& l_retiredFenceValue = l_directCommandQueue.FetchREFLastSignaledFenceValue();

    FWK_ASSERT_RETURN_IF(!ReserveRelease(l_retiredFenceValue, l_resourceReleaseContext), "DynamicUploadStructuredBufferの自動遅延解放登録に失敗しました。");
}

void FWK::Graphics::DynamicUploadStructuredBuffer::MoveFrom(DynamicUploadStructuredBuffer&& a_other) noexcept
{
    m_uploadBuffer        = std::move(a_other.m_uploadBuffer);
    m_elementCount        = a_other.m_elementCount;
    m_structureByteStride = a_other.m_structureByteStride;

    a_other.m_elementCount        = k_invalidElementCount;
    a_other.m_structureByteStride = k_invalidStructuredByteStride;
}