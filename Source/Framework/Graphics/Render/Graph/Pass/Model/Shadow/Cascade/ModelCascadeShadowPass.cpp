#include "ModelCascadeShadowPass.h"

FWK::Graphics::ModelCascadeShadowPass::ModelCascadeShadowPass()
{
    // SkeletalAnimation計算完了後
    // 通常のModel描画より前にShadowMap作成する
    SetupExecutionLayer(Enum::RenderGraphPassExecutionLayer::Shadow);

    // CascadeShadowMapへDepthを書き込むため
    // Pass実行前にResource全体をDEPTH_WRITEへ遷移する
    WriteShadowMap(Enum::RenderGraphShadowMapType::Cascade, Enum::RenderGraphResourceUsage::DepthWrite);
}
FWK::Graphics::ModelCascadeShadowPass::~ModelCascadeShadowPass() = default;

void FWK::Graphics::ModelCascadeShadowPass::Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer, RenderGraph& a_renderGraph)
{
    const auto& l_dsvDescriptorPool    = a_resourceContext.GetREFDSVDescriptorPool();
    const auto& l_directCommandList    = a_renderer.GetREFDirectCommandList       ();
    const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource    ().lock();

    FWK_ASSERT_RETURN_IF(!l_currentFrameResource, "現在のFrameResourceを取得できないため、ModelCascadeShadowPassを実行できません。");

    // CascadeごとのPassConstantBufferを書き込むUploader
    const auto& l_modelCascadeShadowPassDynamicConstantBufferUploader = l_currentFrameResource->FindPTRDynamicBufferUploader<ModelCascadeShadowPassDynamicConstantBufferUploader>().lock();

    FWK_ASSERT_RETURN_IF(!l_modelCascadeShadowPassDynamicConstantBufferUploader, "ModelCascadeShadowPassDynamicConstantBufferUploaderを取得できないため、ModelCascadeShadowPassを実行できません。");

    // 通常描画で登録されているModelの描画申請を
    // Shadow描画でも再利用する
    const auto& l_staticModelCascadeShadowPerObjectDrawRequest            = a_renderGraph.FindVALDrawRequestPerObject<StaticModelCascadeShadowPerObjectDrawRequest>           ().lock();
    const auto& l_skeletalAnimationModelCascadeShadowPerObjectDrawRequest = a_renderGraph.FindVALDrawRequestPerObject<SkeletalAnimationModelCascadeShadowPerObjectDrawRequest>().lock();

    FWK_ASSERT_RETURN_IF(!l_staticModelCascadeShadowPerObjectDrawRequest,            "StaticModelCascadeShadowPerObjectDrawRequestを取得できないため、ModelCascadeShadowPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_skeletalAnimationModelCascadeShadowPerObjectDrawRequest, "SkeletalAnimationModelCascadeShadowPerObjectDrawRequestを取得できないため、ModelCascadeShadowPassを実行できません。");

    // GPUスキニングで描画する大量のキャラクターの影(設定に無い場合は描画しない)
    const auto& l_gpuSkinnedCascadeShadowPerObjectDrawRequest = a_renderGraph.FindVALDrawRequestPerObject<SkeletalAnimationModelGPUSkinnedCascadeShadowPerObjectDrawRequest>().lock();

    // カスケードごとに、GPUスキニングの描画コマンドを作れたか
    std::array<bool, Constant::k_cascadeShadowMapDefaultMAXCascadeCount> l_isGPUSkinnedCulledList = {};

    auto& l_shadowContext    = a_renderer.GetMutableREFShadowContext        ();
    auto& l_cascadeShadowMap = l_shadowContext.GetMutableREFCascadeShadowMap();

    // 現在のCameraとDirectionalLightから
    // Cascadeごとの行列とAmplificationShader用カリング範囲を更新する
    if (!l_cascadeShadowMap.Update()) { return; }

    const auto& l_renderArea                  = l_cascadeShadowMap.GetREFRenderArea                 ();
    const auto& l_depthStencilTextureSettings = l_cascadeShadowMap.GetREFDepthStencilTextureSettings();

    // StaticModelのGPUドリブン描画のカリング
    // Cascadeごとに、ライトから見える範囲に入っているMeshだけを選んで描画コマンドを作る
    // ※ ComputeShader用のPipelineStateを使うため、Cascadeごとの描画ループより前にまとめて行う
    for (UINT l_cascadeIndex = 0U; l_cascadeIndex < l_depthStencilTextureSettings.m_arraySize; ++l_cascadeIndex)
    {
        // このフレームで描き直さないカスケードはカリングも不要
        if (!l_cascadeShadowMap.IsCascadeRenderRequired(l_cascadeIndex)) { continue; }

        const auto* const l_cbModelCascadeShadowPass = l_cascadeShadowMap.FetchPTRModelCascadeShadowPass(l_cascadeIndex);

        FWK_ASSERT_CONTINUE_IF(!l_cbModelCascadeShadowPass, "CBModelCascadeShadowPassを取得できないため、このCascadeのカリングを行えません。");

        // ライトより手前にある物も影を落とすため、手前の平面では除外しない
        auto l_cullingView = ModelGPUDrivenDrawer::BuildCullingView(l_cbModelCascadeShadowPass->m_viewProjectionMatrix, k_useNearPlaneForCulling);

        // このカスケードのシャドウマップで、1メートルが何テクセルになるかを求める
        // (カスケードの範囲の横幅をシャドウマップの横の解像度で割った値が、1テクセルの大きさになる)
        const float l_orthographicWidth = l_cbModelCascadeShadowPass->m_orthographicMAX.x - l_cbModelCascadeShadowPass->m_orthographicMIN.x;

        if (l_orthographicWidth > k_minOrthographicWidth)
        {
            l_cullingView.m_isOrthographicLOD             = true;
            l_cullingView.m_orthographicTexelPerWorldUnit = l_renderArea.GetREFViewport().Width / l_orthographicWidth;
        }

        l_staticModelCascadeShadowPerObjectDrawRequest->DispatchGPUCulling(a_renderer, l_cascadeIndex, l_cullingView);

        if (l_gpuSkinnedCascadeShadowPerObjectDrawRequest &&
            l_cascadeIndex < l_isGPUSkinnedCulledList.size())
        {
            l_isGPUSkinnedCulledList[l_cascadeIndex] = l_gpuSkinnedCascadeShadowPerObjectDrawRequest->DispatchGPUCulling(a_renderer, l_cascadeIndex, l_cullingView);
        }
    }

    // ShadowMapの解像度に対応する
    // ViewportとScissorRectangleを設定する
    l_directCommandList.SetupRenderArea(l_renderArea);

    // CascadeごとにTexture2DArrayの異なるSliceへ
    // StaticModelとSkeletalAnimationModelのDepthを書き込む
    for (UINT l_cascadeIndex = 0U; l_cascadeIndex < l_depthStencilTextureSettings.m_arraySize; ++l_cascadeIndex)
    {
        // 遠いカスケードは数フレームに1回だけ描き直す
        // 描き直さないフレームは、シャドウマップを消さずに前回の結果をそのまま使う
        if (!l_cascadeShadowMap.IsCascadeRenderRequired(l_cascadeIndex)) { continue; }

        const auto l_dsvDescriptorIndex = l_cascadeShadowMap.FetchVALCascadeDSVDescriptorIndex(l_cascadeIndex);

        FWK_ASSERT_RETURN_IF(l_dsvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "Cascadeに知王するDSVDescriptorIndexが無効なため、ModelCascadeShadowPassを実行できませんでした。");

        // 現在Cascadeに対応するTexture2DArraySliceを、Depth出力先として設定する
        l_directCommandList.SetupDepthStencil(l_dsvDescriptorPool, l_dsvDescriptorIndex);

        // 前FrameのDepthを残さないように、現在のCascadeSliceをClearする
        l_directCommandList.ClearDepthStencil(l_dsvDescriptorPool,
                                              l_depthStencilTextureSettings.m_depthClearValue,
                                              l_dsvDescriptorIndex,
                                              l_depthStencilTextureSettings.m_stencilClearValue);

        // CascadeShadowMapが所有しているConstantBufferデータを、
        // 一時保持せずconstポインタで直接参照する
        const auto* const l_cbModelCascadeShadowPass = l_cascadeShadowMap.FetchPTRModelCascadeShadowPass(l_cascadeIndex);

        FWK_ASSERT_RETURN_IF(!l_cbModelCascadeShadowPass, "CBModelCascadeShadowPassを取得できないため、ModelCascadeShadowPassを実行できません。");

        // Uploader::Write内部でUploadBufferへmemcpyされる
        const auto& l_gpuVirtualAddress = l_modelCascadeShadowPassDynamicConstantBufferUploader->Write(*l_cbModelCascadeShadowPass);

        FWK_ASSERT_RETURN_IF(l_gpuVirtualAddress == Constant::k_invalidDynamicBufferGPUVirtualAddress, "CBModelCascadeShadowPassをUploadBufferへ書き込めないため、ModelCascadeShadowPassを実行できません。");

        const auto& l_staticModelRootSignature = SetupGraphicsRenderPipeline(a_renderer, Enum::PipelineStateType::StaticModelCascadeShadow).lock();

        FWK_ASSERT_RETURN_IF(!l_staticModelRootSignature, "StaticModelCascadeShadow用RootSignatureを取得できないため、ModelCascadeShadowPassを実行できません。");

        // StaticModelShadowの定数バッファを書き込み、このCascade用にカリングした描画コマンドをまとめて実行する
        l_directCommandList.SetupConstantBufferView                                 (l_gpuVirtualAddress, *l_staticModelRootSignature, Enum::RootParameterType::CBModelCascadeShadowPass);
        l_staticModelCascadeShadowPerObjectDrawRequest->SetCurrentDrawCascadeIndex  (l_cascadeIndex);
        l_staticModelCascadeShadowPerObjectDrawRequest->SetupPerObjectConstantBuffer(a_renderer,          *l_staticModelRootSignature, *l_currentFrameResource);

        const auto& l_skeletalAnimationModelRootSignature = SetupGraphicsRenderPipeline(a_renderer, Enum::PipelineStateType::SkeletalAnimationModelCascadeShadow).lock();

        FWK_ASSERT_RETURN_IF(!l_skeletalAnimationModelRootSignature, "SkeletalAnimationModelCascadeShadow用RootSignatureを取得できないため、ModelCascadeShadowPassを実行できません。");

        // StaticとSkeletalのShadowPipelineは、
        // 同じModelCascadeShadowRootSignatureを使う設計。
        FWK_ASSERT_RETURN_IF(l_staticModelRootSignature != l_skeletalAnimationModelRootSignature, "StaticとSkeletalのCascade Shadow Pipeline Stateが異なるRootSignatureを使用しています。");

        // SkeletalAnimationShadowの定数バッファを書き込む
        l_directCommandList.SetupConstantBufferView                                            (l_gpuVirtualAddress, *l_skeletalAnimationModelRootSignature, Enum::RootParameterType::CBModelCascadeShadowPass);
        l_skeletalAnimationModelCascadeShadowPerObjectDrawRequest->SetupPerObjectConstantBuffer(a_renderer,          *l_skeletalAnimationModelRootSignature, *l_currentFrameResource);

        // GPUスキニングで描画する大量のキャラクターの影を、このCascade用にカリングした描画コマンドでまとめて描画する
        // (同じRootSignatureのため、上で設定したCascadeの定数バッファはそのまま使われる)
        if (!l_gpuSkinnedCascadeShadowPerObjectDrawRequest     ||
            l_cascadeIndex >= l_isGPUSkinnedCulledList.size() ||
            !l_isGPUSkinnedCulledList[l_cascadeIndex])
        {
            continue;
        }

        SetupGraphicsRenderPipeline(a_renderer, Enum::PipelineStateType::SkeletalAnimationModelGPUSkinnedCascadeShadow);

        l_gpuSkinnedCascadeShadowPerObjectDrawRequest->SetCurrentDrawCascadeIndex  (l_cascadeIndex);
        l_gpuSkinnedCascadeShadowPerObjectDrawRequest->SetupPerObjectConstantBuffer(a_renderer, *l_skeletalAnimationModelRootSignature, *l_currentFrameResource);
    }

    // LOD適用後に描画命令を出した三角形数(全カスケードの合計)を計測用に記録する
    const auto l_gpuSkinnedShadowTriangleCount = l_gpuSkinnedCascadeShadowPerObjectDrawRequest ? l_gpuSkinnedCascadeShadowPerObjectDrawRequest->GetVALSubmittedTriangleCount() : k_initialTriangleCount;

    const auto l_shadowTriangleCount = l_staticModelCascadeShadowPerObjectDrawRequest->GetVALSubmittedTriangleCount           () +
                                       l_skeletalAnimationModelCascadeShadowPerObjectDrawRequest->GetVALSubmittedTriangleCount() +
                                       l_gpuSkinnedShadowTriangleCount;

    a_renderer.GetMutableREFFrameProfiler().AddCounterSample(k_triangleCounterName, static_cast<double>(l_shadowTriangleCount));
}