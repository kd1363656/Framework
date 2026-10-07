#include "SkeletalAnimationModelStandardLitPass.h"

FWK::Graphics::SkeletalAnimationModelStandardLitPass::SkeletalAnimationModelStandardLitPass()
{
    // ComputePassによるVertexShaderが完了した後に、
    // 通常のModel描画として実行する
    SetupExecutionLayer(Enum::RenderGraphPassExecutionLayer::Model);

    // プレビューなど、メイン以外のビューでもこのパスを実行する
    SetupViewScope(Enum::RenderGraphPassViewScope::AllViews);

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
    const auto& l_directCommandList    = a_renderer.GetREFDirectCommandList   ();
    const auto& l_rootSignature        = SetupGraphicsRenderPipeline          (Enum::PipelineStateType::SkeletalAnimationModelLit, a_renderer).lock();
    const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock                                                              ();

    FWK_ASSERT_RETURN_IF(!l_rootSignature,        "SkeletalAnimationModelLit用RootSignatureを取得できないため、SkeletalAnimationModelStandardLitPassを実行できません。" );
    FWK_ASSERT_RETURN_IF(!l_currentFrameResource, "現在FrameResourceを取得できないため、SkeletalAnimationModelStandardLitPassを実行できません。" );

    const auto& l_cameraPassDrawRequest                                 = a_renderGraph.FindVALDrawRequestPass     <CameraPassDrawRequest>                                ().lock();
    const auto& l_cullingCameraPassDrawRequest                          = a_renderGraph.FindVALDrawRequestPass     <CullingCameraPassDrawRequest>                         ().lock();
    const auto& l_lightPassDrawRequest                                  = a_renderGraph.FindVALDrawRequestPass     <LightPassDrawRequest>                                 ().lock();
    const auto& l_cascadeShadowMapPassDrawRequest                       = a_renderGraph.FindVALDrawRequestPass     <CascadeShadowMapPassDrawRequest>                      ().lock();
    const auto& l_skeletalAnimationModelStandardLitPerObjectDrawRequest = a_renderGraph.FindVALDrawRequestPerObject<SkeletalAnimationModelStandardLitPerObjectDrawRequest>().lock();

    FWK_ASSERT_RETURN_IF(!l_cameraPassDrawRequest,                                                                                                    "CameraPassDrawRequestを取得できないため、SkeletalAnimationModelStandardLitPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_cullingCameraPassDrawRequest,                                                                                             "CullingCameraPassDrawRequestを取得できないため、SkeletalAnimationModelStandardLitPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_lightPassDrawRequest,                                                                                                     "LightPassDrawRequestを取得できないため、SkeletalAnimationModelStandardLitPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_cascadeShadowMapPassDrawRequest,                                                                                          "CascadeShadowMapPassDrawRequestを取得できないため、SkeletalAnimationModelStandardLitPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_skeletalAnimationModelStandardLitPerObjectDrawRequest,                                                                    "SkeletalAnimationModelStandardLitPerObjectDrawRequestを取得できないため、SkeletalAnimationModelStandardLitPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_cameraPassDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource),           "Camera定数を設定できないため、SkeletalAnimationModelStandardLitPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_cullingCameraPassDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource),    "カリング用カメラ定数を設定できないため、SkeletalAnimationModelStandardLitPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_lightPassDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource),            "Light定数を設定できないため、SkeletalAnimationModelStandardLitPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_cascadeShadowMapPassDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource), "CascadeShadowMap定数を設定できないため、SkeletalAnimationModelStandardLitPassを実行できません。");

    // 登録されている各SkeletalAnimationModelについて
    // Model定数を設定してMeshShaderをDispatchする
    l_skeletalAnimationModelStandardLitPerObjectDrawRequest->SetupPerObjectConstantBuffer(a_renderer, *l_rootSignature, *l_currentFrameResource);
}