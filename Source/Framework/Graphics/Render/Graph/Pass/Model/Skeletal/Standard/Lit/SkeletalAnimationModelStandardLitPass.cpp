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

    const auto& l_modelRenderSystem         = a_renderer.GetREFModelRenderSystem        ();
    const auto& l_currentFrameResourceIndex = a_renderer.GetREFCurrentFrameResourceIndex();

    // SkeletalAnimationModelで、StandardLitのマテリアルを使うメッシュの描画項目の一覧を描く
    // 描くマテリアルは、マテリアルのGPUデータの型で指定する(このパスのPSが読む型)
    l_modelRenderSystem.RecordDraw<ModelStandardLitMaterialGPUData>(*l_rootSignature,
                                                                    l_directCommandList,
                                                                    l_currentFrameResourceIndex,
                                                                    Enum::ModelMeshType::Skeletal);
}