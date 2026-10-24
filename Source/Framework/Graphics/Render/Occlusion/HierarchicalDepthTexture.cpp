#include "HierarchicalDepthTexture.h"

FWK::Graphics::HierarchicalDepthTexture::~HierarchicalDepthTexture()
{
    Release();
}

bool FWK::Graphics::HierarchicalDepthTexture::Build(Renderer& a_renderer, DepthStencilTexture& a_depthStencilTexture)
{
    const auto& l_depthResource = a_depthStencilTexture.GetREFGPUResource().m_resource;

    FWK_ASSERT_RETURN_VALUE_IF(!l_depthResource, "深度バッファが作成されておらず、Hi-Zの作成に失敗しました。", false);

    const auto l_depthSRVDescriptorIndex = a_depthStencilTexture.GetVALSRVDescriptorIndex();

    FWK_ASSERT_RETURN_VALUE_IF(l_depthSRVDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "深度バッファにSRVが無いため、Hi-Zを作成できません。(SRVFormatを設定してください)", false);

    // 深度バッファの大きさは、リソースの設定から取得する
    const auto l_depthDesc   = l_depthResource->GetDesc();
    const auto l_depthWidth  = static_cast<UINT>(l_depthDesc.Width);
    const auto l_depthHeight = l_depthDesc.Height;

    // 初回、又は画面サイズが変わって深度バッファの大きさが変わった場合はHi-Zを作り直す
    if (!m_gpuResource.m_resource     ||
        l_depthWidth  != m_sourceWidth ||
        l_depthHeight != m_sourceHeight)
    {
        Release();

        FWK_ASSERT_RETURN_VALUE_IF(!Create(a_renderer, l_depthWidth, l_depthHeight), "Hi-Zの作成に失敗しました。", false);
    }

    const auto& l_pipelineState = a_renderer.FindVALPipelineState<ComputePipelineState>(Enum::PipelineStateType::HierarchicalDepthBuild).lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_pipelineState, "Hi-Z作成用ComputePipelineStateが見つかりません。", false);

    const auto& l_rootSignature = l_pipelineState->GetREFUseRootSignature().lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_rootSignature, "Hi-Z作成用RootSignatureが見つかりません。", false);

    const auto l_frameIndex = a_renderer.GetREFCurrentFrameResourceIndex();

    FWK_ASSERT_RETURN_VALUE_IF(l_frameIndex >= m_buildPassConstantBufferList.size(), "フレームリソースの番号が範囲外のため、Hi-Zの作成に失敗しました。", false);

          auto& l_directCommandList     = a_renderer.GetMutableREFDirectCommandList ();
          auto& l_constantBuffer        = m_buildPassConstantBufferList[l_frameIndex];
    auto* const l_constantBufferData    = l_constantBuffer.FetchPTRMappedData       ();
    const auto  l_constantBufferAddress = l_constantBuffer.FetchVALGPUVirtualAddress();

    FWK_ASSERT_RETURN_VALUE_IF(!l_constantBufferData, "Hi-Z作成用定数バッファがMapされていません。", false);

    // 1. 深度バッファをComputeShaderから読める状態へ、Hi-Zを書き込める状態へ遷移する
    const auto l_depthBeforeState = a_depthStencilTexture.GetVALCurrentResourceState();

    l_directCommandList.TransitionResourceBarrier(l_depthResource, l_depthBeforeState, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    a_depthStencilTexture.SetCurrentResourceState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    TransitionResource(l_directCommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    l_directCommandList.SetupComputePipeline(l_pipelineState);

    // 2. ミップを1段ずつ作る
    //    ミップ0 : 深度バッファから作る
    //    ミップ1以降 : 1つ前のミップから作る(1つ前の書き込みが終わるまで待ってから読む)
    for (UINT l_mipLevel = k_firstMIPLevel; l_mipLevel < m_mipCount; ++l_mipLevel)
    {
        const bool l_isFirstMIP = l_mipLevel == k_firstMIPLevel;

        Struct::CBHierarchicalDepthBuildPass l_buildPass = {};

        l_buildPass.m_sourceDescriptorIndex = l_isFirstMIP ? l_depthSRVDescriptorIndex : m_uavDescriptorIndexList[l_mipLevel - k_minSize];
        l_buildPass.m_isSourceDepthTexture  = l_isFirstMIP ? k_trueValue               : k_falseValue;
        l_buildPass.m_sourceWidth           = l_isFirstMIP ? l_depthWidth              : std::max(m_width  >> (l_mipLevel - k_minSize), k_minSize);
        l_buildPass.m_sourceHeight          = l_isFirstMIP ? l_depthHeight             : std::max(m_height >> (l_mipLevel - k_minSize), k_minSize);

        l_buildPass.m_destinationUAVDescriptorIndex = m_uavDescriptorIndexList[l_mipLevel];
        l_buildPass.m_destinationWidth              = std::max(m_width  >> l_mipLevel, k_minSize);
        l_buildPass.m_destinationHeight             = std::max(m_height >> l_mipLevel, k_minSize);

        // 定数バッファはミップ1段につき256byte(定数バッファの配置単位)ずつずらして書き込む
        const auto l_constantBufferOffset = static_cast<UINT64>(l_mipLevel) * D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;

        std::memcpy(l_constantBufferData + l_constantBufferOffset, &l_buildPass, sizeof(Struct::CBHierarchicalDepthBuildPass));

        l_directCommandList.SetupComputeConstantBufferView(l_constantBufferAddress + l_constantBufferOffset, *l_rootSignature, Enum::RootParameterType::CBHierarchicalDepthBuildPass);

        // 書き込み先1ピクセルにつき1Thread(1Groupは8 × 8Thread、切り上げ)
        const auto l_threadGroupCountX = (l_buildPass.m_destinationWidth  + k_hierarchicalDepthBuildThreadCount - k_minSize) / k_hierarchicalDepthBuildThreadCount;
        const auto l_threadGroupCountY = (l_buildPass.m_destinationHeight + k_hierarchicalDepthBuildThreadCount - k_minSize) / k_hierarchicalDepthBuildThreadCount;

        l_directCommandList.Dispatch(l_threadGroupCountX, l_threadGroupCountY, k_singleThreadGroupCount);

        // 次のミップがこのミップを読むため、書き込みが終わるまで待たせる
        l_directCommandList.UAVResourceBarrier(m_gpuResource.m_resource);
    }

    // 3. Hi-Zをカリング用ComputeShaderから読める状態へ、深度バッファを元の状態へ戻す
    TransitionResource(l_directCommandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    l_directCommandList.TransitionResourceBarrier(l_depthResource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, l_depthBeforeState);
    a_depthStencilTexture.SetCurrentResourceState(l_depthBeforeState);

    m_isBuilt = true;

    return true;
}

bool FWK::Graphics::HierarchicalDepthTexture::Create(const Renderer& a_renderer, const UINT a_sourceWidth, const UINT a_sourceHeight)
{
    auto& l_graphicsManager         = GraphicsManager::GetInstance                          ();
    auto& l_resourceContext         = l_graphicsManager.GetMutableREFResourceContext        ();
    auto& l_cbvSRVUAVDescriptorPool = l_resourceContext.GetMutableREFCBVSRVUAVDescriptorPool();

    const auto& l_device             = l_graphicsManager.GetREFDevice            ();
    const auto& l_gpuMemoryAllocator = l_resourceContext.GetREFGPUMemoryAllocator();
    const auto& l_d3dDevice          = l_device.GetREFDevice                     ();

    FWK_ASSERT_RETURN_VALUE_IF(!l_d3dDevice, "デバイスが作成されておらず、Hi-Zの作成に失敗しました。", false);

    // 大きさは深度バッファ以下の2の累乗にする
    m_sourceWidth  = a_sourceWidth;
    m_sourceHeight = a_sourceHeight;
    m_width        = FloorPowerOfTwo(a_sourceWidth);
    m_height       = FloorPowerOfTwo(a_sourceHeight);

    // ミップの数 = 長い方の辺が1になるまで半分にできる回数 + 1
    m_mipCount = k_minSize;

    while ((std::max(m_width, m_height) >> m_mipCount) >= k_minSize &&
           m_mipCount < k_maxMIPCount)
    {
        ++m_mipCount;
    }

    // Hi-Z本体のテクスチャを作成する(ComputeShaderから書き込めるようにALLOW_UNORDERED_ACCESSを付ける)
    const auto l_resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(k_hierarchicalDepthFormat,
                                                             m_width,
                                                             m_height,
                                                             k_singleTextureArraySize,
                                                             static_cast<UINT16>(m_mipCount),
                                                             k_nonMultisampleCount,
                                                             k_nonMultisampleQuality,
                                                             D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    FWK_ASSERT_RETURN_VALUE_IF(!l_gpuMemoryAllocator.CreateTextureResource(l_resourceDesc,
                                                                           nullptr,
                                                                           k_initialResourceState,
                                                                           m_gpuResource),
                                                                           "Hi-Z用テクスチャの作成に失敗しました。",
                                                                           false);

    m_currentResourceState = k_initialResourceState;

    // 全ミップを読むためのSRVを作成する
    D3D12_SHADER_RESOURCE_VIEW_DESC l_srvDesc = {};

    l_srvDesc.Format                    = k_hierarchicalDepthFormat;
    l_srvDesc.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
    l_srvDesc.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    l_srvDesc.Texture2D.MostDetailedMip = k_firstMIPLevel;
    l_srvDesc.Texture2D.MipLevels       = m_mipCount;

    m_srvDescriptorIndex = l_cbvSRVUAVDescriptorPool.Allocate();

    l_d3dDevice->CreateShaderResourceView(m_gpuResource.m_resource.Get(), &l_srvDesc, l_cbvSRVUAVDescriptorPool.FetchVALCPUDescriptorHandle(m_srvDescriptorIndex));

    FWK_ASSERT_RETURN_VALUE_IF(!l_cbvSRVUAVDescriptorPool.CopyCPUDescriptorToShaderVisibleDescriptor(l_device, m_srvDescriptorIndex), "Hi-Z用SRVの作成に失敗しました。", false);

    // ミップ1段ごとに書き込み用のUAVを作成する
    m_uavDescriptorIndexList.clear();

    for (UINT l_mipLevel = k_firstMIPLevel; l_mipLevel < m_mipCount; ++l_mipLevel)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC l_uavDesc = {};

        l_uavDesc.Format             = k_hierarchicalDepthFormat;
        l_uavDesc.ViewDimension      = D3D12_UAV_DIMENSION_TEXTURE2D;
        l_uavDesc.Texture2D.MipSlice = l_mipLevel;

        const auto l_uavDescriptorIndex = l_cbvSRVUAVDescriptorPool.Allocate();

        m_uavDescriptorIndexList.emplace_back(l_uavDescriptorIndex);

        l_d3dDevice->CreateUnorderedAccessView(m_gpuResource.m_resource.Get(),
                                               nullptr,
                                               &l_uavDesc,
                                               l_cbvSRVUAVDescriptorPool.FetchVALCPUDescriptorHandle(l_uavDescriptorIndex));

        FWK_ASSERT_RETURN_VALUE_IF(!l_cbvSRVUAVDescriptorPool.CopyCPUDescriptorToShaderVisibleDescriptor(l_device, l_uavDescriptorIndex), "Hi-Z用UAVの作成に失敗しました。", false);
    }

    // Hi-Z作成用の定数バッファをフレームリソースの数だけ作成する
    if (m_buildPassConstantBufferList.empty())
    {
        m_buildPassConstantBufferList.resize(a_renderer.GetREFFrameResourceList().size());

        for (auto& l_constantBuffer : m_buildPassConstantBufferList)
        {
            FWK_ASSERT_RETURN_VALUE_IF(!l_constantBuffer.Create(l_device, k_maxMIPCount * D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), "Hi-Z作成用定数バッファの作成に失敗しました。", false);
        }
    }

    // 作り直した直後のHi-Zには中身が無いため、作成が終わるまで使えないようにする
    m_isBuilt = false;

    return true;
}

void FWK::Graphics::HierarchicalDepthTexture::Release()
{
    if (!m_gpuResource.m_resource) { return; }

    auto& l_graphicsManager = GraphicsManager::GetInstance();

    const auto& l_renderer           = l_graphicsManager.GetREFRenderer   ();
    const auto& l_directCommandQueue = l_renderer.GetREFDirectCommandQueue();

    auto& l_resourceContext        = l_graphicsManager.GetMutableREFResourceContext       ();
    auto& l_resourceReleaseContext = l_resourceContext.GetMutableREFResourceReleaseContext();

    // GPUがまだHi-Zを使っている可能性があるため、GPUが使い終わるまで解放を待つように登録する
    const auto& l_retiredFenceValue = l_directCommandQueue.FetchREFLastSignaledFenceValue();

    ResourceReleaseContext::GPUResourceReleaseRecord l_gpuResourceReleaseRecord = {};

    l_gpuResourceReleaseRecord.m_gpuResource       = std::move(m_gpuResource);
    l_gpuResourceReleaseRecord.m_retiredFenceValue = l_retiredFenceValue;

    l_resourceReleaseContext.ReserveDeferredReleaseGPUResourceRecord(std::move(l_gpuResourceReleaseRecord));

    // SRVとUAVの番号も同じように、GPUが使い終わってから解放する
    auto l_releaseDescriptorIndex = [&l_resourceReleaseContext, &l_retiredFenceValue](const TypeAlias::DescriptorIndex a_descriptorIndex)
    {
        if (a_descriptorIndex == DescriptorHeap::k_invalidDescriptorIndex) { return; }

        ResourceReleaseContext::DescriptorIndexReleaseRecord l_descriptorIndexReleaseRecord = {};

        l_descriptorIndexReleaseRecord.m_descriptorIndex   = a_descriptorIndex;
        l_descriptorIndexReleaseRecord.m_retiredFenceValue = l_retiredFenceValue;

        l_resourceReleaseContext.ReserveDeferredReleaseCBVSRVUAVDescriptorIndex(std::move(l_descriptorIndexReleaseRecord));
    };

    l_releaseDescriptorIndex(m_srvDescriptorIndex);

    for (const auto& l_uavDescriptorIndex : m_uavDescriptorIndexList)
    {
        l_releaseDescriptorIndex(l_uavDescriptorIndex);
    }

    m_gpuResource        = {};
    m_srvDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;

    m_uavDescriptorIndexList.clear();

    m_isBuilt = false;
}

void FWK::Graphics::HierarchicalDepthTexture::TransitionResource(const DirectCommandList& a_directCommandList, const D3D12_RESOURCE_STATES a_afterState)
{
    if (m_currentResourceState == a_afterState) { return; }

    a_directCommandList.TransitionResourceBarrier(m_gpuResource.m_resource, m_currentResourceState, a_afterState);

    m_currentResourceState = a_afterState;
}

UINT FWK::Graphics::HierarchicalDepthTexture::FloorPowerOfTwo(const UINT a_value)
{
    // a_value以下で最大の2の累乗を求める(例 : 1280 → 1024、720 → 512)
    // std::bit_floorは「一番上の1のビットだけを残す」ことで2の累乗へ切り捨てる
    return std::max(std::bit_floor(a_value), k_minSize);
}