#include "SkeletalAnimationModelStandardUnLitPass.h"

FWK::Graphics::SkeletalAnimationModelStandardUnLitPass::SkeletalAnimationModelStandardUnLitPass()
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
}
FWK::Graphics::SkeletalAnimationModelStandardUnLitPass::~SkeletalAnimationModelStandardUnLitPass() = default;

void FWK::Graphics::SkeletalAnimationModelStandardUnLitPass::Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph)
{
    const auto& l_directCommandList    = a_renderer.GetREFDirectCommandList   ();
    const auto& l_rootSignature        = SetupGraphicsRenderPipeline          (Enum::PipelineStateType::SkeletalAnimationModelUnLit, a_renderer).lock();
    const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock                                                                ();

    FWK_ASSERT_RETURN_IF(!l_rootSignature,        "SkeletalAnimationModelUnLit用RootSignatureを取得できないため、SkeletalAnimationModelStandardUnLitPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_currentFrameResource, "現在FrameResourceを取得できないため、SkeletalAnimationModelStandardUnLitPassを実行できません。");

    const auto& l_cameraPassDrawRequest                                   = a_renderGraph.FindVALDrawRequestPass<CameraPassDrawRequest>                                       ().lock();
    const auto& l_skeletalAnimationModelStandardUnLitPerObjectDrawRequest = a_renderGraph.FindVALDrawRequestPerObject<SkeletalAnimationModelStandardUnLitPerObjectDrawRequest>().lock();

    FWK_ASSERT_RETURN_IF(!l_cameraPassDrawRequest,                                                                                          "CameraPassDrawRequestを取得できないため、SkeletalAnimationModelStandardUnLitPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_skeletalAnimationModelStandardUnLitPerObjectDrawRequest,                                                        "SkeletalAnimationModelStandardUnLitPerObjectDrawRequestを取得できないため、SkeletalAnimationModelStandardUnLitPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_cameraPassDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource), "Camera定数を設定できないため、SkeletalAnimationModelStandardUnLitPassを実行できません。");

    // 登録されている各SkeletalAnimationModelについて
    // Model定数を設定してMeshShaderをDispatchする
    l_skeletalAnimationModelStandardUnLitPerObjectDrawRequest->SetupPerObjectConstantBuffer(a_renderer, *l_rootSignature, *l_currentFrameResource);
}