#include "SkeletalAnimationModelStandardLitPass.h"

FWK::Graphics::SkeletalAnimationModelStandardLitPass::SkeletalAnimationModelStandardLitPass()
{
    // ComputePassによるVertexShaderが完了した後に、
    // 通常のModel描画として実行する
    SetupExecutionLayer(Enum::RenderGraphPassExecutionLayer::Model);

    // SkeletalAnimationModelの描画結果を
    // SceneColorRenderTargetへ書き込む
    WriteRenderTarget(Enum::RenderGraphRenderTargetType::SceneColor, Enum::RenderGraphResourceUsage::RenderTarget);

    // 描画したModelのDepthをSceneDepthへ書き込む
    WriteDepthStencil(Enum::RenderGraphDepthStencilType::SceneDepth, Enum::RenderGraphResourceUsage::DepthWrite);

    // ShadowPassがDepthを書き込んだTexture2DArrayを、
    // LitPixelShaderから読み取れる状態へ遷移する
    ReadShadowMap(Enum::RenderGraphShadowMapType::Cascade, Enum::RenderGraphResourceUsage::PixelShaderResource);
}
FWK::Graphics::SkeletalAnimationModelStandardLitPass::~SkeletalAnimationModelStandardLitPass() = default;

void FWK::Graphics::SkeletalAnimationModelStandardLitPass::Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph)
{
    // GPUスキニングで描画する大量のキャラクターのカリング
    // カメラから見える範囲に入っているMeshだけを選び、LODを決めて描画コマンドを作る
    // ※ ComputeShader用のPipelineStateを使うため、描画用PipelineStateを設定する前に行う
    const auto& l_gpuSkinnedLitPerObjectDrawRequest = a_renderGraph.FindVALDrawRequestPerObject<SkeletalAnimationModelGPUSkinnedLitPerObjectDrawRequest>().lock();

    bool l_isGPUSkinnedCulled = false;

    if (const auto& l_cameraPassDrawRequest = a_renderGraph.FindVALDrawRequestPass<CameraPassDrawRequest>().lock();
        l_cameraPassDrawRequest &&
        l_gpuSkinnedLitPerObjectDrawRequest)
    {
        if (const auto& l_cbCameraPass = l_cameraPassDrawRequest->GetREFSourceConstantBuffer().lock();
            l_cbCameraPass)
        {
            const auto& l_cullingView = ModelGPUDrivenDrawer::BuildCullingView(l_cbCameraPass->m_viewProjectionMatrix, k_useNearPlaneForCulling);

            l_isGPUSkinnedCulled = l_gpuSkinnedLitPerObjectDrawRequest->DispatchGPUCulling(a_renderer, k_cameraViewIndex, l_cullingView);
        }
    }

    const auto& l_directCommandList    = a_renderer.GetREFDirectCommandList   ();
    const auto& l_rootSignature        = SetupGraphicsRenderPipeline          (a_renderer, Enum::PipelineStateType::SkeletalAnimationModelLit).lock();
    const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock                                                              ();

    FWK_ASSERT_RETURN_IF(!l_rootSignature,        "SkeletalAnimationModelLit用RootSignatureを取得できないため、SkeletalAnimationModelStandardLitPassを実行できません。" );
    FWK_ASSERT_RETURN_IF(!l_currentFrameResource, "現在FrameResourceを取得できないため、SkeletalAnimationModelStandardLitPassを実行できません。" );

    const auto& l_cameraPassDrawRequest                                 = a_renderGraph.FindVALDrawRequestPass     <CameraPassDrawRequest>                                ().lock();
    const auto& l_lightPassDrawRequest                                  = a_renderGraph.FindVALDrawRequestPass     <LightPassDrawRequest>                                 ().lock();
    const auto& l_cascadeShadowMapPassDrawRequest                       = a_renderGraph.FindVALDrawRequestPass     <CascadeShadowMapPassDrawRequest>                      ().lock();
    const auto& l_skeletalAnimationModelStandardLitPerObjectDrawRequest = a_renderGraph.FindVALDrawRequestPerObject<SkeletalAnimationModelStandardLitPerObjectDrawRequest>().lock();

    FWK_ASSERT_RETURN_IF(!l_cameraPassDrawRequest,                                                                                                    "CameraPassDrawRequestを取得できないため、SkeletalAnimationModelStandardLitPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_lightPassDrawRequest,                                                                                                     "LightPassDrawRequestを取得できないため、SkeletalAnimationModelStandardLitPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_cascadeShadowMapPassDrawRequest,                                                                                          "CascadeShadowMapPassDrawRequestを取得できないため、SkeletalAnimationModelStandardLitPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_skeletalAnimationModelStandardLitPerObjectDrawRequest,                                                                    "SkeletalAnimationModelStandardLitPerObjectDrawRequestを取得できないため、SkeletalAnimationModelStandardLitPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_cameraPassDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource),           "Camera定数を設定できないため、SkeletalAnimationModelStandardLitPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_lightPassDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource),            "Light定数を設定できないため、SkeletalAnimationModelStandardLitPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_cascadeShadowMapPassDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource), "CascadeShadowMap定数を設定できないため、SkeletalAnimationModelStandardLitPassを実行できません。");

    // 登録されている各SkeletalAnimationModelについて
    // Model定数を設定してMeshShaderをDispatchする
    l_skeletalAnimationModelStandardLitPerObjectDrawRequest->SetupPerObjectConstantBuffer(a_renderer, *l_rootSignature, *l_currentFrameResource);

    // LOD適用後に描画命令を出した三角形数を計測用に記録する
    a_renderer.GetMutableREFFrameProfiler().AddCounterSample(k_triangleCounterName, static_cast<double>(l_skeletalAnimationModelStandardLitPerObjectDrawRequest->GetVALSubmittedTriangleCount()));

    if (!l_gpuSkinnedLitPerObjectDrawRequest) { return; }

    if (l_isGPUSkinnedCulled)
    {
        // GPUスキニング用のPipelineState(メッシュシェーダーの中でスキニングする)へ切り替えて、まとめて描画する
        // RootSignatureは同じため、上で設定したカメラ、ライト、影の定数バッファはそのまま使われる
        SetupGraphicsRenderPipeline(a_renderer, Enum::PipelineStateType::SkeletalAnimationModelGPUSkinnedLit);

        l_gpuSkinnedLitPerObjectDrawRequest->SetCurrentDrawViewIndex     (k_cameraViewIndex);
        l_gpuSkinnedLitPerObjectDrawRequest->SetupPerObjectConstantBuffer(a_renderer, *l_rootSignature, *l_currentFrameResource);
    }

    a_renderer.GetMutableREFFrameProfiler().AddCounterSample(k_gpuSkinnedTriangleCounterName, static_cast<double>(l_gpuSkinnedLitPerObjectDrawRequest->GetVALSubmittedTriangleCount()));
}