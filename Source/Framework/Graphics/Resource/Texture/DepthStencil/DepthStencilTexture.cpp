#include "DepthStencilTexture.h"

bool FWK::Graphics::DepthStencilTexture::Create(const Device&                       a_device, 
                                                const GPUMemoryAllocator&           a_gpuMemoryAllocator, 
                                                const DXGI_FORMAT                   a_format, 
                                                const FLOAT                         a_depthClearValue,
                                                const UINT                          a_width,
                                                const UINT                          a_height,
                                                const UINT8                         a_stencilClearValue,
                                                      TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool)
{
    FWK_ASSERT_RETURN_VALUE_IF(!Utility::IsValidTextureSize(a_width, a_height), "DepthStencilTextureのSizeが無効のため、作成処理に失敗しました。",   false);
    FWK_ASSERT_RETURN_VALUE_IF(a_format == DXGI_FORMAT_UNKNOWN,                 "DepthStencilTextureのFormatが無効のため、作成処理に失敗しました。", false);

    m_format               = a_format;
    m_depthClearValue      = a_depthClearValue;
    m_stencilClearValue    = a_stencilClearValue;
    m_currentResourceState = k_defaultResourceState;

    FWK_ASSERT_RETURN_VALUE_IF(!CreateGPUResource(a_gpuMemoryAllocator, a_width, a_height), "DepthStencilTexture用GPUResourceの作成に失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF(!CreateDSV(a_device, a_dsvDescriptorPool),                   "DepthStencilTexture用DSVの作成に失敗しました。",         false);

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

    FWK_ASSERT_RETURN_VALUE_IF(!Utility::IsValidTextureSize(a_width, a_height), "DepthStencilTextureのリサイズ後サイズが無効のため、リサイズ処理に失敗しました。", false);

    DepthStencilTexture l_newDepthStencilTexture = {};

    FWK_ASSERT_RETURN_VALUE_IF(!l_newDepthStencilTexture.Create(a_device,
                                                                a_gpuMemoryAllocator,    
                                                                m_format,
                                                                m_depthClearValue,
                                                                a_width, 
                                                                a_height,
                                                                m_stencilClearValue,
                                                                a_dsvDescriptorPool),
                                                                "リサイズ後のDepthStencilTexture作成に失敗しました。",
                                                                false);

    FWK_ASSERT_RETURN_VALUE_IF(!ReserveReleaseCurrentResource(a_retiredFenceValue, a_resourceReleaseContext), "古いDepthStencilTextureの遅延解放登録に失敗しました。", false);

    *this = std::move(l_newDepthStencilTexture);

    return true;
}

bool FWK::Graphics::DepthStencilTexture::CreateGPUResource(const GPUMemoryAllocator& a_gpuMemoryAllocator, const UINT a_width, const UINT a_height)
{
    // D3D12_CLEAR_VALUEについて
	// Format			    : クリア対象リソースのフォーマット
	// DepthStencil.Depth   : 深度バッファをクリアする値
	// DepthStencil.Stencil : ステンシルバッファをクリアする値
	D3D12_CLEAR_VALUE l_clearValue = {};
	
	l_clearValue.Format				  = m_format;
	l_clearValue.DepthStencil.Depth   = m_depthClearValue;
	l_clearValue.DepthStencil.Stencil = m_stencilClearValue;

    // 書き込み用深度テクスチャの作成
	// D3D12_RESOURCE_DESCについて
	// Tex2D(フォーマット、
	//		 幅、
	//		 高さ、
	//		 配列数、
	//		 Mip数、
	//		 サンプル数、
	//		 サンプル品質、
	//		 リソースフラグ);
	const auto l_resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_format,
															 a_width,
															 a_height,
															 Constant::k_defaultTexture2DArraySize,
															 Constant::k_defaultTexture2DMipLevels,
															 Constant::k_defaultSampleCount,
															 Constant::k_defaultSampleQuality,
															 D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	FWK_ASSERT_RETURN_VALUE_IF(!a_gpuMemoryAllocator.CreateTextureResource(l_resourceDesc,
																		   &l_clearValue,
																		   k_defaultResourceState,
																		   m_gpuResource),
																		   "DepthStencilTexture用TextureResourceの作成に失敗しました。",
																		   false);

    m_width  = a_width;
    m_height = a_height;

    return true;
}

bool FWK::Graphics::DepthStencilTexture::CreateDSV(const Device& a_device, TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool)
{
    const auto& l_device = a_device.GetREFDevice();

    FWK_ASSERT_RETURN_VALUE_IF(!l_device,                 "Deviceが無効のため、DepthStencilTexture用DSVの作成に失敗しました。",      false);
    FWK_ASSERT_RETURN_VALUE_IF(!m_gpuResource.m_resource, "GPUResourceが無効のため、DepthStencilTexture用DSVの作成に失敗しました。", false);

    const auto l_dsvDescriptorIndex = a_dsvDescriptorPool.Allocate();

    FWK_ASSERT_RETURN_VALUE_IF(l_dsvDescriptorIndex == Constant::k_invalidDescriptorIndex, "DSVDescriptorIndexの確保に失敗しており、DepthStencilTexture用DSVの作成に失敗しました。", false);

    // D3D12_DEPTH_STENCIL_VIEW_DESCについて
    // Format	     : DSVとしてみるときのフォーマット
    // ViewDimension : 2DTextureとしてDSVを作成する
    // Flags		 : 通常のDepthStencilViewとして使用する
    D3D12_DEPTH_STENCIL_VIEW_DESC l_dsvDesc = {};

	l_dsvDesc.Format		= m_format;
	l_dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	l_dsvDesc.Flags			= D3D12_DSV_FLAG_NONE;

    const auto l_dsvHandle = a_dsvDescriptorPool.FetchVALCPUDescriptorHandle(l_dsvDescriptorIndex);

	// CreateDepthStencilView(DSVを作りたい対象リソース、
	//						  DSV設定、
	//						  DSVを書き込むCPUディスクリプタハンドル);
	l_device->CreateDepthStencilView(m_gpuResource.m_resource.Get(), &l_dsvDesc, l_dsvHandle);

	m_currentResourceState = k_defaultResourceState;

    m_dsvDescriptorIndex = l_dsvDescriptorIndex;
	
	return true;
}

bool FWK::Graphics::DepthStencilTexture::ReserveReleaseCurrentResource(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext)
{
    FWK_ASSERT_RETURN_VALUE_IF(!m_gpuResource.m_resource,                                  "DepthStencilTextureのGPUResourceが無効のため、遅延解放登録に失敗しました。",         false);
    FWK_ASSERT_RETURN_VALUE_IF(m_dsvDescriptorIndex == Constant::k_invalidDescriptorIndex, "DepthStencilTextureのDSVDesccriptorIndexが無効のため、遅延解放登録に失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF(a_retiredFenceValue  == Constant::k_unusedFenceValue,       "FenceValueが無効のため、遅延解放登録に失敗しました。",                               false);

    Struct::GPUResourceReleaseRecord l_gpuResourceReleaseRecord = {};

    l_gpuResourceReleaseRecord.m_gpuResource       = std::move(m_gpuResource);
    l_gpuResourceReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

    Struct::DescriptorIndexReleaseRecord l_dsvDescriptorIndexReleaseRecord = {};

    l_dsvDescriptorIndexReleaseRecord.m_descriptorIndex   = m_dsvDescriptorIndex;
    l_dsvDescriptorIndexReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

    FWK_ASSERT_RETURN_VALUE_IF(!a_resourceReleaseContext.ReserveDeferredReleaseGPUResourceRecord (std::move(l_gpuResourceReleaseRecord)),        "DepthStencilTextureのGPUResource遅延解放登録に失敗しました。",        false);
    FWK_ASSERT_RETURN_VALUE_IF(!a_resourceReleaseContext.ReserveDeferredReleaseDSVDescriptorIndex(std::move(l_dsvDescriptorIndexReleaseRecord)), "DepthStencilTextureのDSVDescriptorIndex遅延解放登録に失敗しました。", false);

    // 二重開放を防ぐため、DescriptorIndexを無効化する
    m_dsvDescriptorIndex = Constant::k_invalidDescriptorIndex;

    m_width  = Constant::k_emptyTextureWidth;
    m_height = Constant::k_emptyTextureHeight;

    m_currentResourceState = k_defaultResourceState;

    return true;
}