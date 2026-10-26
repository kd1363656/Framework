#include "ModelGPUDrivenDrawer.h"

void FWK::Graphics::ModelGPUDrivenDrawer::BeginFrame()
{
    // 新しいフレームでは、最初のカリング時にインスタンスなどの表をGPUへ送り直す
    m_isTableUploadRequired = true;

    for (auto& l_viewData : m_viewDataList)
    {
        l_viewData.m_isCulled = false;
    }
}

void FWK::Graphics::ModelGPUDrivenDrawer::UploadFrameTable(const Renderer& a_renderer, const Struct::ModelGPUDrivenFrameTable& a_frameTable)
{
    m_isTableUploadRequired = false;

    // GPU上のBufferは、初めて使うときに作成する
    if (!m_isCreated)
    {
        FWK_ASSERT_RETURN_IF(!Create(a_renderer), "GPUドリブン描画用Bufferの作成に失敗しました。");
    }

    m_currentFrameIndex    = a_renderer.GetREFCurrentFrameResourceIndex();
    m_cullingDispatchCount = k_initialSize;

    auto& l_frameData = m_frameDataList[m_currentFrameIndex];

    // カリングのDispatchに必要な情報を覚えておく
    m_modelBatchList = a_frameTable.m_batchList;
    m_totalPairCount = a_frameTable.m_totalPairCount;
    m_instanceCount  = a_frameTable.m_instanceList.size();

    // インスタンス、Mesh、LODの表を、GPUが読めるBufferへ書き込む
    // このフレームリソースへ前回書き込んだときから変わっていない表は書き込まない
    // (動かない物しか無ければ、毎フレームの書き込みは0になる)
    if (l_frameData.m_uploadedMeshTableVersion != a_frameTable.m_meshTableVersion)
    {
        FWK_ASSERT_RETURN_IF(!l_frameData.m_meshInfoBuffer.Write(a_frameTable.m_meshInfoList), "Mesh情報の書き込みに失敗しました。");
        FWK_ASSERT_RETURN_IF(!l_frameData.m_lodBuffer.Write     (a_frameTable.m_lodList),      "LOD情報の書き込みに失敗しました。");

        l_frameData.m_uploadedMeshTableVersion = a_frameTable.m_meshTableVersion;
    }

    if (l_frameData.m_uploadedInstanceTableVersion != a_frameTable.m_instanceTableVersion)
    {
        FWK_ASSERT_RETURN_IF(!l_frameData.m_instanceBuffer.Write(a_frameTable.m_instanceList), "インスタンス情報の書き込みに失敗しました。");

        l_frameData.m_uploadedInstanceTableVersion = a_frameTable.m_instanceTableVersion;
    }

    // インスタンスごとに上書きするモーフのウェイトを書き込む(上書きするインスタンスが無ければ何も書かない)
    if (l_frameData.m_uploadedMorphWeightTableVersion != a_frameTable.m_morphWeightTableVersion)
    {
        FWK_ASSERT_RETURN_IF(!l_frameData.m_morphWeightBuffer.Write(a_frameTable.m_morphWeightList), "モーフのウェイトの書き込みに失敗しました。");

        l_frameData.m_uploadedMorphWeightTableVersion = a_frameTable.m_morphWeightTableVersion;
    }
}

bool FWK::Graphics::ModelGPUDrivenDrawer::DispatchCulling(Renderer& a_renderer, const std::size_t a_viewIndex, const Struct::ModelGPUDrivenCullingView& a_cullingView)
{
    FWK_ASSERT_RETURN_VALUE_IF(a_viewIndex >= k_maxViewCount, "ViewIndexが最大数を超えているため、GPUドリブン描画のカリングに失敗しました。", false);

    // 表がまだ送られていない、又は描画するモデルが1体も無ければ何もしない
    if (!m_isCreated                        ||
        m_isTableUploadRequired             ||
        m_instanceCount == k_initialSize)
    {
        return false;
    }

    // Viewが足りなければ追加で作成する(影のカスケード数などに合わせて増える)
    while (m_viewDataList.size() <= a_viewIndex)
    {
        auto& l_viewData = m_viewDataList.emplace_back();

        FWK_ASSERT_RETURN_VALUE_IF(!CreateViewData(l_viewData), "GPUドリブン描画用ViewDataの作成に失敗しました。", false);
    }

    const auto& l_pipelineState = a_renderer.FindVALPipelineState<ComputePipelineState>(Enum::PipelineStateType::ModelGPUDrivenCulling).lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_pipelineState, "カリング用ComputePipelineStateが見つからないため、GPUドリブン描画のカリングに失敗しました。", false);

    const auto& l_rootSignature = l_pipelineState->GetREFUseRootSignature().lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_rootSignature, "カリング用RootSignatureが見つからないため、GPUドリブン描画のカリングに失敗しました。", false);

    // LODの選び方はCPU版(ModelLODSelector)と同じ値をGPUへ渡して計算する
    ModelLODSelector l_modelLODSelector = {};

    const auto& l_renderGraph           = a_renderer.GetREFRenderGraph                                ();
    const auto& l_cameraPassDrawRequest = l_renderGraph.FindVALDrawRequestPass<CameraPassDrawRequest>().lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_cameraPassDrawRequest, "CameraPassDrawRequestが見つからないため、GPUドリブン描画のカリングに失敗しました。", false);

    // LODは「カリングに使うカメラ」からの距離で選ぶ
    // (デバッグカメラで描画している間も、メインカメラから見たLODの選ばれ方をそのまま見られるようにするため)
    const auto& l_cbCameraPass = l_cameraPassDrawRequest->FetchVALCullingConstantBuffer();

    // カメラが無ければLODを選べないため描画しない
    if (!l_cbCameraPass) { return false; }

    const auto& l_screenRenderArea = a_renderer.GetREFScreenRenderArea();
    const auto& l_viewport         = l_screenRenderArea.GetREFViewport();

    l_modelLODSelector.Setup(*l_cbCameraPass, l_viewport.Height, Constant::k_defaultMaxScreenSpaceErrorPixel);

    const bool l_isEarlyCulling = a_cullingView.m_cullingMode == Enum::ModelGPUDrivenCullingMode::Early;
    const bool l_isLateCulling  = a_cullingView.m_cullingMode == Enum::ModelGPUDrivenCullingMode::Late;

    // 2回目(Late)は、1回目(Early)を行ったViewが記録した候補を読むため、そのViewが有効か確認する
    if (l_isLateCulling &&
        (a_cullingView.m_candidateSourceViewIndex >= m_viewDataList.size() ||
         !m_viewDataList[a_cullingView.m_candidateSourceViewIndex].m_candidateBuffer.GetREFBufferGPUResource().m_resource))
    {
        return false;
    }

          auto& l_directCommandList = a_renderer.GetMutableREFDirectCommandList();
          auto& l_frameData         = m_frameDataList[m_currentFrameIndex];
          auto& l_viewData          = m_viewDataList [a_viewIndex];
    const auto& l_zeroCounterBuffer = m_zeroCounterUploadBuffer.GetREFUploadBuffer();

    // 1回目(Early)を行うViewは、隠れていた組み合わせを記録するBufferを用意する
    if (l_isEarlyCulling &&
        !l_viewData.m_candidateBuffer.GetREFBufferGPUResource().m_resource)
    {
        auto& l_graphicsManager         = GraphicsManager::GetInstance                          ();
        auto& l_resourceContext         = l_graphicsManager.GetMutableREFResourceContext        ();
        auto& l_cbvSRVUAVDescriptorPool = l_resourceContext.GetMutableREFCBVSRVUAVDescriptorPool();

        FWK_ASSERT_RETURN_VALUE_IF(!l_viewData.m_candidateBuffer.Create<Struct::ModelGPUDrivenOcclusionCandidate>(l_graphicsManager.GetREFDevice(),
                                                                                                                   l_resourceContext.GetREFGPUMemoryAllocator(),
                                                                                                                   k_maxDrawCountPerView,
                                                                                                                   l_cbvSRVUAVDescriptorPool),
                                                                                                                   "オクルージョンカリングの候補用Bufferの作成に失敗しました。",
                                                                                                                   false);
    }

    // 候補を書き込む(Early)、又は読む(Late)Viewのデータ
    auto& l_candidateViewData = l_isLateCulling ? m_viewDataList[a_cullingView.m_candidateSourceViewIndex] : l_viewData;

    // 1. 描画数、三角形数、候補数のカウンターを0に戻す(0が入ったBufferからコピーする)
    Utility::TransitionDynamicRWStructuredBuffer(l_directCommandList, l_viewData.m_counterBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

    l_directCommandList.CopyBufferRegion(k_bufferStartOffset,
                                         k_bufferStartOffset,
                                         k_counterByteSize,
                                         *l_viewData.m_counterBuffer.GetREFBufferGPUResource().m_resource.Get(),
                                         *l_zeroCounterBuffer.Get());

    // 2. ComputeShaderが書き込めるように、書き込み先のBufferをUNORDERED_ACCESS状態へ遷移する
    Utility::TransitionDynamicRWStructuredBuffer(l_directCommandList, l_viewData.m_counterBuffer,         D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Utility::TransitionDynamicRWStructuredBuffer(l_directCommandList, l_viewData.m_drawRecordBuffer,      D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Utility::TransitionDynamicRWStructuredBuffer(l_directCommandList, l_viewData.m_indirectCommandBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    if (l_isEarlyCulling ||
        l_isLateCulling)
    {
        Utility::TransitionDynamicRWStructuredBuffer(l_directCommandList, l_candidateViewData.m_counterBuffer,   D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Utility::TransitionDynamicRWStructuredBuffer(l_directCommandList, l_candidateViewData.m_candidateBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    }

    // 3. カリング用ComputeShaderを実行する
    l_directCommandList.SetupComputePipeline(l_pipelineState);

    if (l_isLateCulling)
    {
        // 2回目 : 1回目で記録した候補だけを判定し直す(候補の数は最大で組み合わせの総数)
        const auto l_cullingPass = CreateCullingPass({},
                                                     a_cullingView,
                                                     l_modelLODSelector,
                                                     l_frameData,
                                                     l_viewData,
                                                     l_candidateViewData);

        WriteCullingPassAndDispatch(l_directCommandList,
                                    *l_rootSignature,
                                    l_cullingPass,
                                    static_cast<UINT>(m_totalPairCount));
    }
    else
    {
        // 通常 / 1回目 : モデルの種類ごとに「インスタンス数 × Mesh数」個のThreadで判定する
        for (const auto& l_modelBatch : m_modelBatchList)
        {
            const auto l_cullingPass = CreateCullingPass(l_modelBatch,
                                                         a_cullingView,
                                                         l_modelLODSelector,
                                                         l_frameData,
                                                         l_viewData,
                                                         l_candidateViewData);

            WriteCullingPassAndDispatch(l_directCommandList,
                                        *l_rootSignature,
                                        l_cullingPass,
                                        l_modelBatch.m_instanceCount * l_modelBatch.m_meshCount);
        }
    }

    // 4. 書き込みが終わったBufferを、それぞれの使い方に合わせた状態へ遷移する
    //    DrawRecord      : AS / MS / PSが定数バッファとして読む
    //    IndirectCommand : ExecuteIndirectが描画コマンドとして読む
    //    Counter         : ExecuteIndirectが実行するコマンド数として読む
    Utility::TransitionDynamicRWStructuredBuffer(l_directCommandList, l_viewData.m_drawRecordBuffer,      D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    Utility::TransitionDynamicRWStructuredBuffer(l_directCommandList, l_viewData.m_indirectCommandBuffer, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
    Utility::TransitionDynamicRWStructuredBuffer(l_directCommandList, l_viewData.m_counterBuffer,         D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

    l_viewData.m_isCulled = true;

    return true;
}

void FWK::Graphics::ModelGPUDrivenDrawer::ExecuteIndirect(const Renderer& a_renderer, const RootSignature& a_rootSignature, const std::size_t a_viewIndex)
{
    // このフレームでカリングしていないViewは、描画コマンドが用意されていないので描画しない
    if (a_viewIndex >= m_viewDataList.size()) { return; }

    auto& l_viewData = m_viewDataList[a_viewIndex];

    if (!l_viewData.m_isCulled) { return; }

    // 使用中のRootSignature用のCommandSignatureを探し、無ければ作成する
    auto l_commandSignatureItr = std::ranges::find_if(m_commandSignatureList, [&a_rootSignature](const ModelIndirectDispatchMeshCommandSignature& a_commandSignature)
    {
        return a_commandSignature.IsCreatedFor(a_rootSignature);
    });

    if (l_commandSignatureItr == m_commandSignatureList.end())
    {
              auto& l_graphicsManager = GraphicsManager::GetInstance  ();
        const auto& l_device          = l_graphicsManager.GetREFDevice();

        ModelIndirectDispatchMeshCommandSignature l_commandSignature = {};

        FWK_ASSERT_RETURN_IF(!l_commandSignature.Create(l_device, a_rootSignature), "GPUドリブン描画用CommandSignatureの作成に失敗しました。");

        m_commandSignatureList.emplace_back(l_commandSignature);

        l_commandSignatureItr = std::prev(m_commandSignatureList.end());
    }

    const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

    // GPUが書いた描画コマンドを1回の命令でまとめて実行する
    l_directCommandList.ExecuteIndirect(l_commandSignatureItr->GetREFCommandSignature(),
                                        static_cast<UINT>(k_maxDrawCountPerView),
                                        l_viewData.m_indirectCommandBuffer.GetREFBufferGPUResource().m_resource,
                                        l_viewData.m_counterBuffer.GetREFBufferGPUResource().m_resource);

    // 同じフレームで2回描画しないように、使い終わったことを記録する
    l_viewData.m_isCulled = false;
}

FWK::Struct::ModelGPUDrivenCullingView FWK::Graphics::ModelGPUDrivenDrawer::BuildCullingView(const TypeAlias::Math::Matrix& a_viewProjectionMatrix, const bool a_useNearPlane)
{
    // ワールド座標の点pを画面上の座標へ変換すると clip = (p, 1) × ビュープロジェクション行列 になる
    // 点が画面内にあるのは -clip.w <= clip.x <= clip.w (上下も同様)、0 <= clip.z <= clip.w のとき
    // この条件を式変形すると、行列の列同士の足し引きで6枚の平面が求まる(Gribb-Hartmann法)
    const auto& l_matrix = a_viewProjectionMatrix;

    const TypeAlias::Math::Vector4 l_column1 = { l_matrix._11, l_matrix._21, l_matrix._31, l_matrix._41 };
    const TypeAlias::Math::Vector4 l_column2 = { l_matrix._12, l_matrix._22, l_matrix._32, l_matrix._42 };
    const TypeAlias::Math::Vector4 l_column3 = { l_matrix._13, l_matrix._23, l_matrix._33, l_matrix._43 };
    const TypeAlias::Math::Vector4 l_column4 = { l_matrix._14, l_matrix._24, l_matrix._34, l_matrix._44 };

    Struct::ModelGPUDrivenCullingView l_cullingView = {};

    auto& l_planeList = l_cullingView.m_frustumPlaneList;

    l_planeList[k_frustumLeftPlaneIndex]   = l_column4 + l_column1;
    l_planeList[k_frustumRightPlaneIndex]  = l_column4 - l_column1;
    l_planeList[k_frustumBottomPlaneIndex] = l_column4 + l_column2;
    l_planeList[k_frustumTopPlaneIndex]    = l_column4 - l_column2;
    l_planeList[k_frustumFarPlaneIndex]    = l_column4 - l_column3;
    l_planeList[k_frustumNearPlaneIndex]   = l_column3;

    // 平面の法線(xyz)の長さを1にする
    // 長さが1でないと「中心から平面までの距離」が正しく求まらず、球の半径と比べられないため
    for (auto& l_plane : l_planeList)
    {
        const float l_normalLength = TypeAlias::Math::Vector3(l_plane.x, l_plane.y, l_plane.z).Length();

        if (l_normalLength > k_minPlaneNormalLength) { l_plane /= l_normalLength; }
    }

    // 手前の平面は配列の最後に置いているため、数を5にすれば手前の判定だけを省ける
    l_cullingView.m_frustumPlaneCount = a_useNearPlane ? k_frustumPlaneCountWithNear : k_frustumPlaneCountWithoutNear;

    return l_cullingView;
}

bool FWK::Graphics::ModelGPUDrivenDrawer::Create(const Renderer& a_renderer)
{
    auto& l_graphicsManager         = GraphicsManager::GetInstance                          ();
    auto& l_resourceContext         = l_graphicsManager.GetMutableREFResourceContext        ();
    auto& l_cbvSRVUAVDescriptorPool = l_resourceContext.GetMutableREFCBVSRVUAVDescriptorPool();

    const auto& l_device             = l_graphicsManager.GetREFDevice    ();
    const auto  l_frameResourceCount = a_renderer.GetREFFrameResourceList().size();

    FWK_ASSERT_RETURN_VALUE_IF(l_frameResourceCount == k_initialSize, "フレームリソースが無いため、GPUドリブン描画用Bufferの作成に失敗しました。", false);

    m_frameDataList.clear ();
    m_frameDataList.resize(l_frameResourceCount);

    for (auto& l_frameData : m_frameDataList)
    {
        FWK_ASSERT_RETURN_VALUE_IF(!l_frameData.m_instanceBuffer.Create<Struct::SBModelGPUDrivenInstance>(l_device, Constant::k_modelGPUDrivenMaxInstanceCount, l_cbvSRVUAVDescriptorPool), "インスタンス用Bufferの作成に失敗しました。", false);
        FWK_ASSERT_RETURN_VALUE_IF(!l_frameData.m_meshInfoBuffer.Create<Struct::SBModelGPUDrivenMeshInfo>(l_device, Constant::k_modelGPUDrivenMaxMeshInfoCount, l_cbvSRVUAVDescriptorPool), "Mesh情報用Bufferの作成に失敗しました。",       false);
        FWK_ASSERT_RETURN_VALUE_IF(!l_frameData.m_lodBuffer.Create<Struct::ModelMeshLOD>             (l_device, Constant::k_modelGPUDrivenMaxLODCount,      l_cbvSRVUAVDescriptorPool), "LOD用Bufferの作成に失敗しました。",            false);
        FWK_ASSERT_RETURN_VALUE_IF(!l_frameData.m_morphWeightBuffer.Create<float>(l_device, Constant::k_modelGPUDrivenMaxMorphWeightCount, l_cbvSRVUAVDescriptorPool), "モーフのウェイト用Bufferの作成に失敗しました。", false);

        FWK_ASSERT_RETURN_VALUE_IF(!l_frameData.m_cullingPassConstantBuffer.Create(l_device, k_maxCullingDispatchCountPerFrame * k_cullingPassConstantBufferStride), "カリング用定数バッファの作成に失敗しました。", false);
    }

    // カウンターを0に戻すためのコピー元を作成し、中身を0にしておく
    FWK_ASSERT_RETURN_VALUE_IF(!m_zeroCounterUploadBuffer.Create(l_device, k_counterByteSize), "カウンター初期化用Bufferの作成に失敗しました。", false);

    auto* const l_zeroCounterMappedData = m_zeroCounterUploadBuffer.FetchPTRMappedData();

    FWK_ASSERT_RETURN_VALUE_IF(!l_zeroCounterMappedData, "カウンター初期化用BufferがMapされていません。", false);

    std::memset(l_zeroCounterMappedData, k_zeroFillByte, k_counterByteSize);

    m_isCreated = true;

    return true;
}

bool FWK::Graphics::ModelGPUDrivenDrawer::CreateViewData(ViewData& a_viewData) const
{
    auto& l_graphicsManager         = GraphicsManager::GetInstance                          ();
    auto& l_resourceContext         = l_graphicsManager.GetMutableREFResourceContext        ();
    auto& l_cbvSRVUAVDescriptorPool = l_resourceContext.GetMutableREFCBVSRVUAVDescriptorPool();

    const auto& l_device             = l_graphicsManager.GetREFDevice            ();
    const auto& l_gpuMemoryAllocator = l_resourceContext.GetREFGPUMemoryAllocator();

    FWK_ASSERT_RETURN_VALUE_IF(!a_viewData.m_drawRecordBuffer.Create<Struct::ModelGPUDrivenDrawRecord>(l_device,
                                                                                                       l_gpuMemoryAllocator,
                                                                                                       k_maxDrawCountPerView,
                                                                                                       l_cbvSRVUAVDescriptorPool),
                                                                                                       "DrawRecord用Bufferの作成に失敗しました。",
                                                                                                       false);

    FWK_ASSERT_RETURN_VALUE_IF(!a_viewData.m_indirectCommandBuffer.Create<Struct::ModelGPUDrivenIndirectCommand>(l_device,
                                                                                                                 l_gpuMemoryAllocator,
                                                                                                                 k_maxDrawCountPerView,
                                                                                                                 l_cbvSRVUAVDescriptorPool),
                                                                                                                 "描画コマンド用Bufferの作成に失敗しました。",
                                                                                                                 false);

    FWK_ASSERT_RETURN_VALUE_IF(!a_viewData.m_counterBuffer.Create<std::uint32_t>(l_device,
                                                                                 l_gpuMemoryAllocator,
                                                                                 Constant::k_modelGPUDrivenCounterElementCount,
                                                                                 l_cbvSRVUAVDescriptorPool),
                                                                                 "カウンター用Bufferの作成に失敗しました。",
                                                                                 false);

    return true;
}

FWK::Struct::CBModelGPUDrivenCullingPass FWK::Graphics::ModelGPUDrivenDrawer::CreateCullingPass(const Struct::ModelGPUDrivenBatch&       a_modelBatch,
                                                                                                         const Struct::ModelGPUDrivenCullingView& a_cullingView,
                                                                                                         const ModelLODSelector&                  a_modelLODSelector,
                                                                                                         const FrameData&                         a_frameData,
                                                                                                         const ViewData&                          a_viewData,
                                                                                                         const ViewData&                          a_candidateViewData) const
{
    Struct::CBModelGPUDrivenCullingPass l_cullingPass = {};

    l_cullingPass.m_frustumPlaneList  = a_cullingView.m_frustumPlaneList;
    l_cullingPass.m_frustumPlaneCount = a_cullingView.m_frustumPlaneCount;

    l_cullingPass.m_cameraWorldPosition         = a_modelLODSelector.GetREFCameraWorldPosition     ();
    l_cullingPass.m_lodProjectionScale          = a_modelLODSelector.GetVALProjectionScale         ();
    l_cullingPass.m_lodMaxScreenSpaceErrorPixel = a_modelLODSelector.GetVALMaxScreenSpaceErrorPixel();
    l_cullingPass.m_lodMinDistance              = a_modelLODSelector.FetchVALMinDistance           ();

    // 影(平行投影)では、カメラからの距離ではなく「シャドウマップの1テクセルの大きさ」を基準にLODを選ぶ
    // 誤差が1テクセル以下なら影の形は変わらないため、遠くのカスケードほど粗いLODを使える
    if (a_cullingView.m_isOrthographicLOD)
    {
        l_cullingPass.m_isOrthographicLOD           = k_trueValue;
        l_cullingPass.m_lodProjectionScale          = a_cullingView.m_orthographicTexelPerWorldUnit;
        l_cullingPass.m_lodMaxScreenSpaceErrorPixel = k_shadowMaxErrorTexel;
        l_cullingPass.m_minProjectedSizeTexel       = k_shadowMinCasterSizeTexel;
    }

    l_cullingPass.m_isCullingDebugVisualize = a_cullingView.m_isCullingDebugVisualize ? k_trueValue : Constant::k_initialGPUDrivenCullingPassUIntValue;

    l_cullingPass.m_maxDrawCount   = static_cast<std::uint32_t>(k_maxDrawCountPerView);
    l_cullingPass.m_instanceOffset = a_modelBatch.m_instanceOffset;
    l_cullingPass.m_instanceCount  = a_modelBatch.m_instanceCount;
    l_cullingPass.m_meshInfoOffset = a_modelBatch.m_meshInfoOffset;
    l_cullingPass.m_meshCount      = a_modelBatch.m_meshCount;

    l_cullingPass.m_instanceBufferSRVDescriptorIndex        = a_frameData.m_instanceBuffer.GetVALSRVDescriptorIndex      ();
    l_cullingPass.m_meshInfoBufferSRVDescriptorIndex        = a_frameData.m_meshInfoBuffer.GetVALSRVDescriptorIndex      ();
    l_cullingPass.m_lodBufferSRVDescriptorIndex             = a_frameData.m_lodBuffer.GetVALSRVDescriptorIndex           ();

    l_cullingPass.m_morphOverrideWeightBufferSRVDescriptorIndex = a_frameData.m_morphWeightBuffer.GetVALSRVDescriptorIndex();

    l_cullingPass.m_drawRecordBufferUAVDescriptorIndex      = a_viewData.m_drawRecordBuffer.GetVALUAVDescriptorIndex     ();
    l_cullingPass.m_indirectCommandBufferUAVDescriptorIndex = a_viewData.m_indirectCommandBuffer.GetVALUAVDescriptorIndex();
    l_cullingPass.m_counterBufferUAVDescriptorIndex         = a_viewData.m_counterBuffer.GetVALUAVDescriptorIndex        ();

    // DrawRecordBufferの先頭のGPUアドレス(64bit)を、HLSLで扱えるように32bitずつに分ける
    const auto l_drawRecordBufferAddress = a_viewData.m_drawRecordBuffer.GetREFBufferGPUResource().m_resource->GetGPUVirtualAddress();

    l_cullingPass.m_drawRecordBufferAddressLow  = static_cast<std::uint32_t>(l_drawRecordBufferAddress & k_addressLowMask);
    l_cullingPass.m_drawRecordBufferAddressHigh = static_cast<std::uint32_t>(l_drawRecordBufferAddress >> k_addressHighShiftBit);

    // オクルージョンカリング(1回目、2回目)の設定
    l_cullingPass.m_cullingMode       = a_cullingView.m_cullingMode;
    l_cullingPass.m_maxCandidateCount = static_cast<std::uint32_t>(k_maxDrawCountPerView);

    if (a_cullingView.m_cullingMode == Enum::ModelGPUDrivenCullingMode::Normal) { return l_cullingPass; }

    l_cullingPass.m_candidateBufferUAVDescriptorIndex        = a_candidateViewData.m_candidateBuffer.GetVALUAVDescriptorIndex();
    l_cullingPass.m_candidateCounterBufferUAVDescriptorIndex = a_candidateViewData.m_counterBuffer.GetVALUAVDescriptorIndex  ();

    // Hi-Zがまだ作られていない(最初のフレームなど)場合は、隠れているかの判定をしない
    const auto* const l_hierarchicalDepthTexture = a_cullingView.m_hierarchicalDepthTexture;

    if (!l_hierarchicalDepthTexture ||
        !l_hierarchicalDepthTexture->IsValid())
    {
        return l_cullingPass;
    }

    l_cullingPass.m_isOcclusionEnabled                         = k_trueValue;
    l_cullingPass.m_occlusionViewProjectionMatrix              = a_cullingView.m_occlusionViewProjectionMatrix;
    l_cullingPass.m_hierarchicalDepthTextureSRVDescriptorIndex = l_hierarchicalDepthTexture->GetVALSRVDescriptorIndex();
    l_cullingPass.m_hierarchicalDepthMIPCount                  = l_hierarchicalDepthTexture->GetVALMIPCount          ();
    l_cullingPass.m_hierarchicalDepthSize                      = TypeAlias::Math::Vector2(static_cast<float>(l_hierarchicalDepthTexture->GetVALWidth ()), static_cast<float>(l_hierarchicalDepthTexture->GetVALHeight()));

    return l_cullingPass;
}
void FWK::Graphics::ModelGPUDrivenDrawer::WriteCullingPassAndDispatch(const DirectCommandList&                   a_directCommandList,
                                                                            const RootSignature&                       a_rootSignature,
                                                                            const Struct::CBModelGPUDrivenCullingPass& a_cullingPass,
                                                                            const UINT                                 a_threadCount)
{
    if (m_cullingDispatchCount >= k_maxCullingDispatchCountPerFrame) { return; }

    auto& l_frameData = m_frameDataList[m_currentFrameIndex];

    auto* const l_cullingPassMappedData  = l_frameData.m_cullingPassConstantBuffer.FetchPTRMappedData       ();
    const auto  l_cullingPassBaseAddress = l_frameData.m_cullingPassConstantBuffer.FetchVALGPUVirtualAddress();

    FWK_ASSERT_RETURN_IF(!l_cullingPassMappedData, "カリング用定数バッファがMapされていないため、GPUドリブン描画のカリングに失敗しました。");

    // 定数バッファは1回のDispatchにつき512byte(定数バッファの配置単位256byteの倍数)ずつずらして書き込む
    const auto l_constantBufferOffset = m_cullingDispatchCount * k_cullingPassConstantBufferStride;

    std::memcpy(l_cullingPassMappedData + l_constantBufferOffset, &a_cullingPass, sizeof(Struct::CBModelGPUDrivenCullingPass));

    a_directCommandList.SetupComputeConstantBufferView(l_cullingPassBaseAddress + l_constantBufferOffset, a_rootSignature, Enum::RootParameterType::CBModelGPUDrivenCullingPass);

    // 1Groupあたり64Threadで、必要なThread数を切り上げでGroup数に変換する
    const auto l_threadGroupCount = Utility::DivideRoundUp(a_threadCount, Constant::k_modelGPUDrivenCullingThreadCount);

    if (l_threadGroupCount != k_initialThreadGroupCount)
    {
        a_directCommandList.Dispatch(l_threadGroupCount, k_singleThreadGroupCount, k_singleThreadGroupCount);
    }

    ++m_cullingDispatchCount;
}