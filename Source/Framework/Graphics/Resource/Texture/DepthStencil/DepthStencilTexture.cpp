#include "DepthStencilTexture.h"

bool FWK::Graphics::DepthStencilTexture::Create(const Device&                              a_device, 
                                                const GPUMemoryAllocator&                  a_gpuMemoryAllocator, 
                                                const Struct::DepthStencilTextureSettings& a_depthStencilTextureSettings, 
                                                const UINT                                 a_width, 
                                                const UINT                                 a_height, 
                                                      TypeAlias::DSVDescriptorPool&        a_dsvDescriptorPool)
{
    FWK_ASSERT_RETURN_VALUE_IF(!Utility::IsValidTextureSize(a_width, a_height),                             "DepthStencilTextureのSizeが無効のため、作成処理に失敗しました。",           false);
    FWK_ASSERT_RETURN_VALUE_IF(m_depthStencilTextureSettings.m_resourceFormat == DXGI_FORMAT_UNKNOWN,       "DepthStencilTextureのResourceFormatが無効のため、作成処理に失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF(m_depthStencilTextureSettings.m_dsvFormat ==      DXGI_FORMAT_UNKNOWN,       "DepthStencilTextureのDSVFormatが無効のため、作成処理に失敗しました。",      false);
    FWK_ASSERT_RETURN_VALUE_IF(a_depthStencilTextureSettings.m_arraySize <       k_minimumTextureArraySize, "DepthStencilTextureのArraySizeが無効のため、作成処理に失敗しました。",      false);
	FWK_ASSERT_RETURN_VALUE_IF(a_depthStencilTextureSettings.m_mipLevels <       k_minimumMipLevelCount,    "DepthStencilTextureのMipLevelsが無効のため、作成処理に失敗しました。",      false);
	FWK_ASSERT_RETURN_VALUE_IF(a_depthStencilTextureSettings.m_sampleCount <     k_minimumSampleCount,      "DepthStencilTextureのSampleCountが無効のため、作成処理に失敗しました。",    false);

    const bool l_isMultiSample = a_depthStencilTextureSettings.m_sampleCount > k_nonMultiSampleCount;

    // DirectX12のMSAATextureはMipMapを複数持てない。
    // SampleCountが1より大きい場合は、
    // MipLevelsを必ず1にする必要がある。
    FWK_ASSERT_RETURN_VALUE_IF(l_isMultiSample && 
                               a_depthStencilTextureSettings.m_mipLevels != k_singleMipLevelCount, 
                               "MSAAを使用するDepthStencilTextureはMipLevelsをOneにする必要があります。", 
                               false);

    // Jsonから読み込んだ設定をTexture自身が保持する
    // Resize時にも同じ設定を再利用するため
    // WidthとHeight以外の引数を再度外部から渡す必要がない
    m_depthStencilTextureSettings = a_depthStencilTextureSettings;
    m_currentResourceState        = k_defaultResourceState;
    
    FWK_ASSERT_RETURN_VALUE_IF(!CreateGPUResource(a_gpuMemoryAllocator, a_width, a_height), "DepthStencilTexture用GPUResourceの作成に失敗しました。", false);

    if (!CreateDSVList(a_device, a_dsvDescriptorPool))
    {
        m_gpuResource = {};

		m_width  = Converter::TextureBinaryConverter::k_emptyTextureWidth;
		m_height = Converter::TextureBinaryConverter::k_emptyTextureHeight;

        FWK_ASSERT_RETURN_VALUE("DepthStencilTexture用DSVListの作成に失敗しました。", false);
    }
    
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
                                                                m_depthStencilTextureSettings,
                                                                a_width, 
                                                                a_height,
                                                                a_dsvDescriptorPool),
                                                                "リサイズ後のDepthStencilTexture作成に失敗しました。",
                                                                false);

    if (!ReserveReleaseCurrentResource(a_retiredFenceValue, a_resourceReleaseContext))
    {
        l_newDepthStencilTexture.ReleaseCreateDSVDescriptorInndexList(a_dsvDescriptorPool);

        FWK_ASSERT_RETURN_VALUE("古いDepthStencilTextureの遅延解放登録に失敗しました。", false);
    }

    *this = std::move(l_newDepthStencilTexture);

    return true;
}

FWK::TypeAlias::DescriptorIndex FWK::Graphics::DepthStencilTexture::FetchVALDSVDescriptorIndex(const UINT a_arrayIndex, const UINT a_mipSlice) const
{
    // 存在しないArryaSliceまたはMipSliceを指定された場合は、無効なDescriptorIndexを返して使用側で検出できるようにする
    if (a_arrayIndex >= m_depthStencilTextureSettings.m_arraySize ||
        a_mipSlice >= m_depthStencilTextureSettings.m_mipLevels) 
    {
        return DescriptorHeap::k_invalidDescriptorIndex;
    }

    // DSVの格納順は次のとおり。
	// Array Zero / Mip Zero
	// Array Zero / Mip One
	// Array One  / Mip Zero
	// Array One  / Mip One
	// そのためArray IndexにMip数を掛け、
	// その後へMip Sliceを加えることで一次元配列の位置を求める。
    const auto& l_descriptorIndex = a_arrayIndex * m_depthStencilTextureSettings.m_mipLevels + a_mipSlice;

    if (l_descriptorIndex >= m_dsvDescriptorIndexList.size()) { return DescriptorHeap::k_invalidDescriptorIndex; }

    return m_dsvDescriptorIndexList[l_descriptorIndex];
}

bool FWK::Graphics::DepthStencilTexture::CreateGPUResource(const GPUMemoryAllocator& a_gpuMemoryAllocator, const UINT a_width, const UINT a_height)
{
    // D3D12_CLEAR_VALUEについて
	// Format			    : クリア対象リソースのフォーマット
	// DepthStencil.Depth   : 深度バッファをクリアする値
	// DepthStencil.Stencil : ステンシルバッファをクリアする値
	D3D12_CLEAR_VALUE l_clearValue = {};
	
	l_clearValue.Format				  =m_depthStencilTextureSettings.m_dsvFormat;
	l_clearValue.DepthStencil.Depth   =m_depthStencilTextureSettings.m_depthClearValue;
	l_clearValue.DepthStencil.Stencil =m_depthStencilTextureSettings.m_stencilClearValue;

    D3D12_RESOURCE_FLAGS l_resourceFlags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    // SRVFormatがUNKNOWNなら、
	// このTextureはShaderから読み取らないDepth専用Resourceになる。
	// D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCEを付けることで、
	// ShaderResourceとして使用しないことをDirectX12へ明示する。
    if (m_depthStencilTextureSettings.m_srvFormat == DXGI_FORMAT_UNKNOWN)
    {
        l_resourceFlags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
    }

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
	const auto l_resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_depthStencilTextureSettings.m_resourceFormat,
															 a_width,
															 a_height,
                                                             m_depthStencilTextureSettings.m_arraySize,
                                                             m_depthStencilTextureSettings.m_mipLevels,
                                                             m_depthStencilTextureSettings.m_sampleCount,
                                                             m_depthStencilTextureSettings.m_sampleQuality,
                                                             l_resourceFlags);

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

bool FWK::Graphics::DepthStencilTexture::CreateDSVList(const Device& a_device, TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool)
{
    const auto& l_device = a_device.GetREFDevice();

    return false;
}

bool FWK::Graphics::DepthStencilTexture::ReserveReleaseCurrentResource(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext)
{
    FWK_ASSERT_RETURN_VALUE_IF(!m_gpuResource.m_resource,                                        "DepthStencilTextureのGPUResourceが無効のため、遅延解放登録に失敗しました。",         false);
    FWK_ASSERT_RETURN_VALUE_IF(m_dsvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "DepthStencilTextureのDSVDesccriptorIndexが無効のため、遅延解放登録に失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF(a_retiredFenceValue  == Fence::k_unusedFenceValue,                "FenceValueが無効のため、遅延解放登録に失敗しました。",                               false);

    ResourceReleaseContext::GPUResourceReleaseRecord l_gpuResourceReleaseRecord = {};

    l_gpuResourceReleaseRecord.m_gpuResource       = std::move(m_gpuResource);
    l_gpuResourceReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

    ResourceReleaseContext::DescriptorIndexReleaseRecord l_dsvDescriptorIndexReleaseRecord = {};

    l_dsvDescriptorIndexReleaseRecord.m_descriptorIndex   = m_dsvDescriptorIndex;
    l_dsvDescriptorIndexReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

    FWK_ASSERT_RETURN_VALUE_IF(!a_resourceReleaseContext.ReserveDeferredReleaseGPUResourceRecord (std::move(l_gpuResourceReleaseRecord)),        "DepthStencilTextureのGPUResource遅延解放登録に失敗しました。",        false);
    FWK_ASSERT_RETURN_VALUE_IF(!a_resourceReleaseContext.ReserveDeferredReleaseDSVDescriptorIndex(std::move(l_dsvDescriptorIndexReleaseRecord)), "DepthStencilTextureのDSVDescriptorIndex遅延解放登録に失敗しました。", false);

    // 二重開放を防ぐため、DescriptorIndexを無効化する
    m_dsvDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;

    m_width  = Converter::TextureBinaryConverter::k_emptyTextureWidth;
    m_height = Converter::TextureBinaryConverter::k_emptyTextureHeight;

    m_currentResourceState = k_defaultResourceState;

    return true;
}

void FWK::Graphics::DepthStencilTexture::ReleaseCreateDSVDescriptorInndexList(TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool)
{}

D3D12_DEPTH_STENCIL_VIEW_DESC FWK::Graphics::DepthStencilTexture::FetchVALDesc(const UINT a_arrayIndex, const UINT a_mipSlice) const
{
    return D3D12_DEPTH_STENCIL_VIEW_DESC();
}
