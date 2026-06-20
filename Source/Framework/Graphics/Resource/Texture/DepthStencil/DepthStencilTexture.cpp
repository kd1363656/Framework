#include "DepthStencilTexture.h"

bool FWK::Graphics::DepthStencilTexture::Create(const Device&                       a_device, 
                                                const GPUMemoryAllocator&           a_gpuMemoryAllocator, 
                                                const DXGI_FORMAT                   a_format, 
                                                const UINT                          a_width,
                                                const UINT                          a_height,
                                                const FLOAT                         a_depthClearValue,
                                                const UINT8                         a_stencilClearValue,
                                                      TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool)
{
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!Utility::IsValidTextureSize(a_height, a_height), "DepthStencilTextureのSizeが無効のため、作成処理に失敗しました。",   false);
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_format == DXGI_FORMAT_UNKNOWN,                  "DepthStencilTextureのFormatが無効のため、作成処理に失敗しました。", false);

    m_format               = a_format;
    m_depthClearValue      = a_depthClearValue;
    m_stencilClearValue    = a_stencilClearValue;
    m_currentResourceState = k_initialResourceState;

    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateGPUResource(a_gpuMemoryAllocator, a_width, a_height), "DepthStencilTexture用GPUResourceの作成に失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateDSV(a_device, a_dsvDescriptorPool),                   "DepthStencilTexture用DSVの作成に失敗しました。",         false);

    return true;
}

bool FWK::Graphics::DepthStencilTexture::Resize(const Device&                       a_device, 
                                                const GPUMemoryAllocator&           a_gpuMemoryAllocator, 
                                                const UINT64&                       a_retiredFenceValue, 
                                                const UINT                          a_width, 
                                                const UINT                          a_height, 
                                                      TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool, 
                                                      ResourceReleaseContext&       a_resourceReleaseContext)
{
    // 同じサイズなら作り直す必要がない
    if (Utility::IsSameSize(m_width,
                            m_height,
                            a_width,
                            a_height))
    {
        return true;
    }

    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!Utility::IsValidTextureSize(a_width, a_height), "DepthStencilTextureのリサイズ後サイズが無効のため、リサイズ処理に失敗しました。", false);

    DepthStencilTexture l_newDepthStencilTexture = {};

    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_newDepthStencilTexture.Create(a_device,
                                                                       a_gpuMemoryAllocator,    
                                                                       m_format,
                                                                       a_width, 
                                                                       a_height,
                                                                       m_depthClearValue,
                                                                       m_stencilClearValue,
                                                                       a_dsvDescriptorPool),
                                                                       "リサイズ後のDepthStencilTexture作成に失敗しました。",
                                                                       false);

    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!ReserveReleaseCurrentResource(a_retiredFenceValue, a_resourceReleaseContext), "古いDepthStencilTextureの遅延解放登録に失敗しました。", false);

    *this = std::move(l_newDepthStencilTexture);

    return true;
}

bool FWK::Graphics::DepthStencilTexture::CreateGPUResource(const GPUMemoryAllocator& a_gpuMemoryAllocator, const UINT a_width, const UINT a_height)
{
    return false;
}