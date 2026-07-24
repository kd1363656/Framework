#include "DepthStencilTexture.h"

bool FWK::Graphics::DepthStencilTexture::Create(const Device&                              a_device, 
                                                const GPUMemoryAllocator&                  a_gpuMemoryAllocator, 
                                                const Struct::DepthStencilTextureSettings& a_depthStencilTextureSettings, 
                                                const UINT                                 a_width, 
                                                const UINT                                 a_height, 
                                                      TypeAlias::DSVDescriptorPool&        a_dsvDescriptorPool,
                                                      TypeAlias::CBVSRVUAVDescriptorPool&  a_cbvSRVUAVDescriptorPool)
{
    FWK_ASSERT_RETURN_VALUE_IF(!Utility::IsValidTextureSize(a_width, a_height),                             "DepthStencilTextureのSizeが無効のため、作成処理に失敗しました。",           false);
    FWK_ASSERT_RETURN_VALUE_IF(a_depthStencilTextureSettings.m_resourceFormat == DXGI_FORMAT_UNKNOWN,       "DepthStencilTextureのResourceFormatが無効のため、作成処理に失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF(a_depthStencilTextureSettings.m_dsvFormat ==      DXGI_FORMAT_UNKNOWN,       "DepthStencilTextureのDSVFormatが無効のため、作成処理に失敗しました。",      false);
    FWK_ASSERT_RETURN_VALUE_IF(a_depthStencilTextureSettings.m_arraySize <       k_minimumTextureArraySize, "DepthStencilTextureのArraySizeが無効のため、作成処理に失敗しました。",      false);
	FWK_ASSERT_RETURN_VALUE_IF(a_depthStencilTextureSettings.m_mipLevels <       k_minimumMIPLevelCount,    "DepthStencilTextureのMipLevelsが無効のため、作成処理に失敗しました。",      false);
	FWK_ASSERT_RETURN_VALUE_IF(a_depthStencilTextureSettings.m_sampleCount <     k_minimumSampleCount,      "DepthStencilTextureのSampleCountが無効のため、作成処理に失敗しました。",    false);

    // DirectX12のMSAATextureはMipMapを複数持てない。
    // SampleCountが1より大きい場合は、
    // MipLevelsを必ず1にする必要がある。
    FWK_ASSERT_RETURN_VALUE_IF(a_depthStencilTextureSettings.m_sampleCount > k_nonMultisampleCount &&
                               a_depthStencilTextureSettings.m_mipLevels != k_singleMIPLevelCount, 
                               "MSAAを使用するDepthStencilTextureはMipLevelsをOneにする必要があります。", 
                               false);

    // Jsonから読み込んだ設定をTexture自身が保持する
    // Resize時にも同じ設定を再利用するため
    // WidthとHeight以外の引数を再度外部から渡す必要がない
    m_depthStencilTextureSettings = a_depthStencilTextureSettings;
    m_currentResourceState        = k_defaultResourceState;
    
    FWK_ASSERT_RETURN_VALUE_IF(!CreateGPUResource(a_gpuMemoryAllocator, a_width, a_height), "DepthStencilTexture用GPUResourceの作成に失敗しました。", false);

    // DSV用デスクリプタインデックスを作成
    if (!CreateDSVList(a_device, a_dsvDescriptorPool))
    {
        m_gpuResource = {};

		m_width  = Converter::TextureBinaryConverter::k_emptyTextureWidth;
		m_height = Converter::TextureBinaryConverter::k_emptyTextureHeight;

        FWK_ASSERT_RETURN_VALUE("DepthStencilTexture用DSVListの作成に失敗しました。", false);
    }

    // SRVFormatがUNKNOWNの場合は、Shaderから読み取らないDepth専用Texture
    if (m_depthStencilTextureSettings.m_srvFormat != DXGI_FORMAT_UNKNOWN &&
        !CreateSRV(a_device, a_cbvSRVUAVDescriptorPool))
    {
        // SRV作成に失敗した場合は、
		// 先に作成したDSVも即座にDescriptor Poolへ返す
		// まだGPUへCommandを提出していないため、Fence待ちは不要
        ReleaseCreatedDSVDescriptorIndexList(a_dsvDescriptorPool);

        m_gpuResource = {};

        m_width  = Converter::TextureBinaryConverter::k_emptyTextureWidth;
        m_height = Converter::TextureBinaryConverter::k_emptyTextureHeight;

        FWK_ASSERT_RETURN_VALUE("DepthStencilTexture用SRVの作成に失敗しました。", false);
    }

    return true;
}

bool FWK::Graphics::DepthStencilTexture::Resize(const Device&                             a_device,
                                                const GPUMemoryAllocator&                 a_gpuMemoryAllocator, 
                                                const UINT64&                             a_retiredFenceValue, 
                                                const UINT                                a_width, 
                                                const UINT                                a_height, 
                                                      TypeAlias::DSVDescriptorPool&       a_dsvDescriptorPool, 
                                                      TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool,
                                                      ResourceReleaseContext&             a_resourceReleaseContext)
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
                                                                a_dsvDescriptorPool,
                                                                a_cbvSRVUAVDescriptorPool),
                                                                "リサイズ後のDepthStencilTexture作成に失敗しました。",
                                                                false);

    if (!ReserveReleaseCurrentResource(a_retiredFenceValue, a_resourceReleaseContext))
    {
        // 新しく作成したTexutreはまだGPUへ提出されていないため、
        // DescriptorをFence待ちせずに即座にPoolへ返せる
        l_newDepthStencilTexture.ReleaseCreatedDSVDescriptorIndexList(a_dsvDescriptorPool);
        l_newDepthStencilTexture.ReleaseCreatedSRVDescriptorIndex    (a_cbvSRVUAVDescriptorPool);

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

FWK::TypeAlias::DescriptorIndex FWK::Graphics::DepthStencilTexture::FetchVALBaseSubresourceDSVDescriptorIndex() const
{
    return FetchVALDSVDescriptorIndex(k_firstArrayIndex, k_firstMIPSlice);
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

D3D12_DEPTH_STENCIL_VIEW_DESC FWK::Graphics::DepthStencilTexture::CreateDSVDesc(const UINT a_arrayIndex, const UINT a_mipSlice) const
{
    D3D12_DEPTH_STENCIL_VIEW_DESC l_dsvDesc = {};

    l_dsvDesc.Format = m_depthStencilTextureSettings.m_dsvFormat;
    l_dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

    const bool l_isTextureArray = m_depthStencilTextureSettings.m_arraySize   > k_singleTextureArraySize;
    const bool l_isMultisample  = m_depthStencilTextureSettings.m_sampleCount > k_nonMultisampleCount;

    // MSAAを使用しない通常のTexture2D
    if (!l_isTextureArray &&
        !l_isMultisample)
    {
        l_dsvDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
        l_dsvDesc.Texture2D.MipSlice = a_mipSlice;

        return l_dsvDesc;
    }

    // MSAAを使用しないTexture2DArray
    if (l_isTextureArray && 
        !l_isMultisample)
    {
        l_dsvDesc.ViewDimension                  = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
        l_dsvDesc.Texture2DArray.MipSlice        = a_mipSlice;
        l_dsvDesc.Texture2DArray.FirstArraySlice = a_arrayIndex;
        l_dsvDesc.Texture2DArray.ArraySize       = k_singleDSVArraySliceCount;

        return l_dsvDesc;
    }

    // MSAAを使用する配列ではないTexture2D
    if (!l_isTextureArray && l_isMultisample)
    {
        l_dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;

        return l_dsvDesc;
    }

    // MSAAを使用するTexture2DArray
    l_dsvDesc.ViewDimension                    = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
    l_dsvDesc.Texture2DMSArray.FirstArraySlice = a_arrayIndex;
    l_dsvDesc.Texture2DMSArray.ArraySize       = k_singleDSVArraySliceCount;

    return l_dsvDesc;
}
D3D12_SHADER_RESOURCE_VIEW_DESC FWK::Graphics::DepthStencilTexture::CreateSRVDesc() const
{
    D3D12_SHADER_RESOURCE_VIEW_DESC l_srvDesc = {};

    l_srvDesc.Format                  = m_depthStencilTextureSettings.m_srvFormat;
    l_srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    
    const bool l_isTextureArray = m_depthStencilTextureSettings.m_arraySize   > k_singleTextureArraySize;
    const bool l_isMultisample  = m_depthStencilTextureSettings.m_sampleCount > k_nonMultisampleCount;

    // MSAAを使用しない通常のTexture2D
    if (!l_isTextureArray &&
        !l_isMultisample)
    {
        l_srvDesc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D;
        l_srvDesc.Texture2D.MostDetailedMip     = k_mostDetailedMIP;
        l_srvDesc.Texture2D.MipLevels           = m_depthStencilTextureSettings.m_mipLevels;
        l_srvDesc.Texture2D.PlaneSlice          = k_planeSlice;
        l_srvDesc.Texture2D.ResourceMinLODClamp = k_resourceMINLODClamp;

        return l_srvDesc;
    }

    // MSAAを使用しないTexture2DArray
    if (l_isTextureArray &&
        !l_isMultisample) 
    {
        l_srvDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        l_srvDesc.Texture2DArray.MostDetailedMip     = k_mostDetailedMIP;
        l_srvDesc.Texture2DArray.MipLevels           = m_depthStencilTextureSettings.m_mipLevels;
        l_srvDesc.Texture2DArray.FirstArraySlice     = k_firstArraySlice;
        l_srvDesc.Texture2DArray.ArraySize           = m_depthStencilTextureSettings.m_arraySize;
        l_srvDesc.Texture2DArray.PlaneSlice          = k_planeSlice;
        l_srvDesc.Texture2DArray.ResourceMinLODClamp = k_resourceMINLODClamp;

        return l_srvDesc;
    }

    // MSAAを使用する通常のTexture2D
    if (!l_isTextureArray &&
        l_isMultisample)
    {
        l_srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
        
        return l_srvDesc;
    }

    // MSAAを使用するTexture2DArray
    l_srvDesc.ViewDimension                    = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
    l_srvDesc.Texture2DMSArray.FirstArraySlice = k_firstArraySlice;
    l_srvDesc.Texture2DMSArray.ArraySize       = m_depthStencilTextureSettings.m_arraySize;

    return l_srvDesc;
}

bool FWK::Graphics::DepthStencilTexture::CreateDSVList(const Device& a_device, TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool)
{
    const auto& l_device = a_device.GetREFDevice();

    FWK_ASSERT_RETURN_VALUE_IF(!l_device,                 "Deviceが無効のため、DepthStencilTexture用DSVListの作成に失敗しました。",      false);
    FWK_ASSERT_RETURN_VALUE_IF(!m_gpuResource.m_resource, "GPUResourceが無効のため、DepthStencilTexture用DSVListの作成に失敗しました。", false);

    // Array SliceとMip Sliceのすべての組み合わせに対して、
	// 個別のDSVを一つずつ作成する。
	// CSMが4Cascade、MipがOneならDSVは4個になる。
    const auto& l_dsvCount = static_cast<std::size_t>(m_depthStencilTextureSettings.m_arraySize * m_depthStencilTextureSettings.m_mipLevels);

    m_dsvDescriptorIndexList.clear  ();
    m_dsvDescriptorIndexList.reserve(l_dsvCount);

    for (UINT l_arrayIndex = 0ULL; l_arrayIndex < m_depthStencilTextureSettings.m_arraySize; ++l_arrayIndex) 
    {
        for (UINT l_mipSlice = k_firstMIPSlice; l_mipSlice < m_depthStencilTextureSettings.m_mipLevels; ++l_mipSlice)
        {
            const auto l_dsvDescriptorIndex = a_dsvDescriptorPool.Allocate();

            if (l_dsvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex)
            {
                // GPUへCommandを提出する前に作成失敗なので
                // ここまでに確保したDescriptorは即座にPoolへ返せる
                ReleaseCreatedDSVDescriptorIndexList(a_dsvDescriptorPool);
            }

            FWK_ASSERT_RETURN_VALUE_IF(l_dsvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "DSVDescriptorIndexの確保に失敗しており、DepthStencilTexture用DSVListの作成に失敗しました。", false);

            const auto& l_dsvDesc   = CreateDSVDesc                                  (l_arrayIndex, l_mipSlice);
            const auto& l_dsvHandle = a_dsvDescriptorPool.FetchVALCPUDescriptorHandle(l_dsvDescriptorIndex);

            // CreateDepthStencilView(DSVを作成するGPUResource,
            //                        DSVの設定,
            //                        DSVを書き込むCPUDescriptorHandle);
            // 指定したArraySliceとMiupSliceだけを参照するDSVを作る
            l_device->CreateDepthStencilView(m_gpuResource.m_resource.Get(), &l_dsvDesc, l_dsvHandle);

            m_dsvDescriptorIndexList.emplace_back(l_dsvDescriptorIndex);
        }
    }

    m_currentResourceState = k_defaultResourceState;

    return true;
}
bool FWK::Graphics::DepthStencilTexture::CreateSRV(const Device& a_device, TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
    const auto& l_device = a_device.GetREFDevice();

    FWK_ASSERT_RETURN_VALUE_IF(!l_device,                                                        "Deviceが無効のため、DepthStencilTexture用SRVの作成に失敗しました。",      false);
	FWK_ASSERT_RETURN_VALUE_IF(!m_gpuResource.m_resource,                                        "GPUResourceが無効のため、DepthStencilTexture用SRVの作成に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(m_depthStencilTextureSettings.m_srvFormat == DXGI_FORMAT_UNKNOWN, "DepthStencilTextureのSRVFormatが無効のため、SRVの作成に失敗しました。",   false);

    const auto l_srvDescriptorIndex = a_cbvSRVUAVDescriptorPool.Allocate();

    FWK_ASSERT_RETURN_VALUE_IF(l_srvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "DepthStencilTexture用SRVDescriptorIndexの確保に失敗しまた。", false);

    const auto& l_srvDesc      = CreateSRVDesc                                        ();
    const auto& l_srvCPUHandle = a_cbvSRVUAVDescriptorPool.FetchVALCPUDescriptorHandle(l_srvDescriptorIndex);

    // CPU専用DescriptorHeapへSRVを作成する
    l_device->CreateShaderResourceView(m_gpuResource.m_resource.Get(), &l_srvDesc, l_srvCPUHandle);

    // ShaderからResourceDescriptorHeapで参照できるようにする
    // 同じかIndexのShaderVisibleDescriptorHeapへコピーする
    if (!a_cbvSRVUAVDescriptorPool.CopyCPUDescriptorToShaderVisibleDescriptor(a_device, l_srvDescriptorIndex))
    {
        a_cbvSRVUAVDescriptorPool.Release(l_srvDescriptorIndex);

        FWK_ASSERT_RETURN_VALUE("DepthStencilTexture用SRVのShaderVisibleDescriptorHeapへのコピーに失敗しました。", false);
    }

    m_srvDescriptorIndex = l_srvDescriptorIndex;

    return true;
}

bool FWK::Graphics::DepthStencilTexture::ReserveReleaseCurrentResource(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext)
{
    FWK_ASSERT_RETURN_VALUE_IF(!m_gpuResource.m_resource,                        "DepthStencilTextureのGPUResourceが無効のため、遅延解放登録に失敗しました。",          false);
    FWK_ASSERT_RETURN_VALUE_IF(m_dsvDescriptorIndexList.empty(),                 "DepthStencilTextureのDSVDescriptorIndexListが空のため、遅延解放登録に失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF(a_retiredFenceValue == Fence::k_unusedFenceValue, "FenceValueが無効のため、遅延解放登録に失敗しました。",                                false);
 
    const bool l_isSRVRequired = m_depthStencilTextureSettings.m_srvFormat != DXGI_FORMAT_UNKNOWN;

    FWK_ASSERT_RETURN_VALUE_IF(l_isSRVRequired &&
                               m_srvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, 
                               "SRVを使用するDepthStencilTextureのSRVDescriptorIndexが無効です。",
                               false);

    ResourceReleaseContext::GPUResourceReleaseRecord l_gpuResourceReleaseRecord = {};

    l_gpuResourceReleaseRecord.m_gpuResource       = std::move(m_gpuResource);
    l_gpuResourceReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

    FWK_ASSERT_RETURN_VALUE_IF(!a_resourceReleaseContext.ReserveDeferredReleaseGPUResourceRecord(std::move(l_gpuResourceReleaseRecord)), "DepthStencilTextureのGPUResource遅延解放登録に失敗しました。", false);

    // TextureArryaでは複数のDSVを持つ
    // GPUが古いTextureを使用し終わった同じFenceValueで全てのDSVDescriptorIndexも遅延解放する
    for (const auto l_dsvDescriptorIndex : m_dsvDescriptorIndexList) 
    {
        ResourceReleaseContext::DescriptorIndexReleaseRecord l_dsvDescriptorIndexReleaseRecord = {};

        l_dsvDescriptorIndexReleaseRecord.m_descriptorIndex   = l_dsvDescriptorIndex;
        l_dsvDescriptorIndexReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

        FWK_ASSERT_RETURN_VALUE_IF(!a_resourceReleaseContext.ReserveDeferredReleaseDSVDescriptorIndex(std::move(l_dsvDescriptorIndexReleaseRecord)), "DepthStencilTextureのDSVDescriptorIndex遅延解放登録に失敗しました。", false);
    }

    // SRVFormatが有効な場合だけ
    // SRVDescriptorIndexも同じFenceValueで遅延解放する
    if (l_isSRVRequired)
    {
        ResourceReleaseContext::DescriptorIndexReleaseRecord l_srvDescriptorIndexReleaseRecord = {};

        l_srvDescriptorIndexReleaseRecord.m_descriptorIndex   = m_srvDescriptorIndex;
        l_srvDescriptorIndexReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

        FWK_ASSERT_RETURN_VALUE_IF(!a_resourceReleaseContext.ReserveDeferredReleaseCBVSRVUAVDescriptorIndex(std::move(l_srvDescriptorIndexReleaseRecord)), "DepthStencilTextureのSRVDescriptorIndex遅延解放登録に失敗しました。", false);
    }

    // 二重解放を防ぐため
    // 現在のTextureからDescriptorIndexの所有情報を外す
    m_dsvDescriptorIndexList.clear();

    m_srvDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;

    m_width  = Converter::TextureBinaryConverter::k_emptyTextureWidth;
    m_height = Converter::TextureBinaryConverter::k_emptyTextureHeight;
    
    m_currentResourceState = k_defaultResourceState;

    return true;
}

void FWK::Graphics::DepthStencilTexture::ReleaseCreatedDSVDescriptorIndexList(TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool)
{
    // Create()途中で失敗した場合は、
	// まだGPUがDescriptorを参照していない。
	// そのためFenceを待たず、
	// DescriptorPoolへ即座に返却できる。
    for (const auto l_dsvDescriptorIndex : m_dsvDescriptorIndexList)
    {
        if (l_dsvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex) { continue; }

        a_dsvDescriptorPool.Release(l_dsvDescriptorIndex);
    }

    m_dsvDescriptorIndexList.clear();
}
void FWK::Graphics::DepthStencilTexture::ReleaseCreatedSRVDescriptorIndex(TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
    if (m_srvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex) { return; }

    // この関数を使用するのは、まだGPUへCommandを提出していない作成失敗時
	// そのためFenceを待たず、
	// CBV/SRV/UAV Descriptor Poolへ即座に返却できる
    a_cbvSRVUAVDescriptorPool.Release(m_srvDescriptorIndex);

    m_srvDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;
}