#include "SkeletalAnimationModelRitchToonPass.h"

FWK::Graphics::SkeletalAnimationModelRitchToonPass::SkeletalAnimationModelRitchToonPass()
{
    // ComputePassによるスキニングが完了した後に、通常のModel描画として実行する
    SetupExecutionLayer(Enum::RenderGraphPassExecutionLayer::Model);

    // シーンカラーとシーン深度へ書き込む
    WriteRenderTarget(Enum::RenderGraphRenderTargetType::SceneColor,   Enum::RenderGraphResourceUsage::RenderTarget);
    WriteDepthStencil(Enum::RenderGraphDepthStencilType::SceneDepth, Enum::RenderGraphResourceUsage::DepthWrite);

    // ShadowPassがDepthを書き込んだTexture2DArrayを、PixelShaderから読み取れる状態へ遷移する
    ReadShadowMap(Enum::RenderGraphShadowMapType::Cascade, Enum::RenderGraphResourceUsage::PixelShaderResource);
}
FWK::Graphics::SkeletalAnimationModelRitchToonPass::~SkeletalAnimationModelRitchToonPass() = default;

void FWK::Graphics::SkeletalAnimationModelRitchToonPass::Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph)
{
    const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

    const auto& l_cameraPassDrawRequest           = a_renderGraph.FindVALDrawRequestPass     <CameraPassDrawRequest>                                        ().lock();
    const auto& l_lightPassDrawRequest            = a_renderGraph.FindVALDrawRequestPass     <LightPassDrawRequest>                                         ().lock();
    const auto& l_cascadeShadowMapPassDrawRequest = a_renderGraph.FindVALDrawRequestPass     <CascadeShadowMapPassDrawRequest>                              ().lock();
    const auto& l_ritchToonPerObjectDrawRequest   = a_renderGraph.FindVALDrawRequestPerObject<SkeletalAnimationModelRitchToonPerObjectDrawRequest>          ().lock();
    const auto& l_gpuSkinnedRitchToonDrawRequest  = a_renderGraph.FindVALDrawRequestPerObject<SkeletalAnimationModelGPUSkinnedRitchToonPerObjectDrawRequest>().lock();

    FWK_ASSERT_RETURN_IF(!l_cameraPassDrawRequest,           "CameraPassDrawRequestを取得できないため、SkeletalAnimationModelRitchToonPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_lightPassDrawRequest,            "LightPassDrawRequestを取得できないため、SkeletalAnimationModelRitchToonPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_cascadeShadowMapPassDrawRequest, "CascadeShadowMapPassDrawRequestを取得できないため、SkeletalAnimationModelRitchToonPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_ritchToonPerObjectDrawRequest,   "SkeletalAnimationModelRitchToonPerObjectDrawRequestを取得できないため、SkeletalAnimationModelRitchToonPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_gpuSkinnedRitchToonDrawRequest,  "SkeletalAnimationModelGPUSkinnedRitchToonPerObjectDrawRequestを取得できないため、SkeletalAnimationModelRitchToonPassを実行できません。");

    // GPUスキニングで描画するキャラクターのカリング
    // 表面用と輪郭線用で、同じ判定の描画コマンドを2つ作る(描画コマンドは1回実行すると使い終わった扱いになるため)
    // ※ ComputeShader用のPipelineStateを使うため、描画用PipelineStateを設定する前に行う
    bool l_isGPUSkinnedCulled = false;

    if (const auto& l_cbCameraPass = l_cameraPassDrawRequest->FetchVALCullingConstantBuffer();
        l_cbCameraPass)
    {
        auto l_cullingView = ModelGPUDrivenDrawer::BuildCullingView(l_cbCameraPass->m_viewProjectionMatrix, k_useNearPlaneForCulling);

        l_cullingView.m_isCullingDebugVisualize = l_cameraPassDrawRequest->GetVALIsCullingDebugVisualize();

        l_isGPUSkinnedCulled = l_gpuSkinnedRitchToonDrawRequest->DispatchGPUCulling(a_renderer, k_surfaceViewIndex, l_cullingView) &&
                               l_gpuSkinnedRitchToonDrawRequest->DispatchGPUCulling(a_renderer, k_outlineViewIndex, l_cullingView);
    }

    // 1. 従来の方式の表面
    const auto& l_rootSignature        = SetupGraphicsRenderPipeline          (a_renderer, Enum::PipelineStateType::SkeletalAnimationModelRitchToon).lock();
    const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock                                                                    ();

    FWK_ASSERT_RETURN_IF(!l_rootSignature,        "SkeletalAnimationModelRitchToon用RootSignatureを取得できないため、SkeletalAnimationModelRitchToonPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_currentFrameResource, "現在のFrameResourceを取得できないため、SkeletalAnimationModelRitchToonPassを実行できません。");

    FWK_ASSERT_RETURN_IF(!l_cameraPassDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource),           "Camera定数を設定できないため、SkeletalAnimationModelRitchToonPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_lightPassDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource),            "Light定数を設定できないため、SkeletalAnimationModelRitchToonPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_cascadeShadowMapPassDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource), "CascadeShadowMap定数を設定できないため、SkeletalAnimationModelRitchToonPassを実行できません。");

    l_ritchToonPerObjectDrawRequest->SetupPerObjectConstantBuffer(a_renderer, *l_rootSignature, *l_currentFrameResource);

    // 2. 従来の方式の輪郭線
    // RootSignatureは表面と同じため、上で設定したカメラ、ライト、影の定数バッファはそのまま使われる
    SetupGraphicsRenderPipeline(a_renderer, Enum::PipelineStateType::SkeletalAnimationModelRitchToonOutline);

    l_ritchToonPerObjectDrawRequest->SetupPerObjectConstantBuffer(a_renderer, *l_rootSignature, *l_currentFrameResource);

    if (!l_isGPUSkinnedCulled) { return; }

    // 3. GPUスキニングの表面
    SetupGraphicsRenderPipeline(a_renderer, Enum::PipelineStateType::SkeletalAnimationModelGPUSkinnedRitchToon);

    l_gpuSkinnedRitchToonDrawRequest->SetCurrentDrawViewIndex     (k_surfaceViewIndex);
    l_gpuSkinnedRitchToonDrawRequest->SetupPerObjectConstantBuffer(a_renderer, *l_rootSignature, *l_currentFrameResource);

    // 4. GPUスキニングの輪郭線
    SetupGraphicsRenderPipeline(a_renderer, Enum::PipelineStateType::SkeletalAnimationModelGPUSkinnedRitchToonOutline);

    l_gpuSkinnedRitchToonDrawRequest->SetCurrentDrawViewIndex     (k_outlineViewIndex);
    l_gpuSkinnedRitchToonDrawRequest->SetupPerObjectConstantBuffer(a_renderer, *l_rootSignature, *l_currentFrameResource);
}