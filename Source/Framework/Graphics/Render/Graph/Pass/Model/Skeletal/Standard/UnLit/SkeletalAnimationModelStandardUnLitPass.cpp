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

    const auto& l_modelRenderSystem         = a_renderer.GetREFModelRenderSystem        ();
    const auto& l_currentFrameResourceIndex = a_renderer.GetREFCurrentFrameResourceIndex();

    // SkeletalAnimationModelで、StandardUnLitのマテリアルを使うメッシュの描画項目の一覧を描く
    // 描くマテリアルは、マテリアルのGPUデータの型で指定する(このパスのPSが読む型)
    l_modelRenderSystem.RecordDraw<ModelStandardUnLitMaterialGPUData>(*l_rootSignature,
                                                                      l_directCommandList,
                                                                      l_currentFrameResourceIndex,
                                                                      Enum::ModelMeshType::Skeletal);
}