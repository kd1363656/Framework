#include "StaticModelStandardLitPass.h"

FWK::Graphics::StaticModelStandardLitPass::StaticModelStandardLitPass()
{
    SetupExecutionLayer(Enum::RenderGraphPassExecutionLayer::Model);

    // プレビューなど、メイン以外のビューでもこのパスを実行する
    SetupViewScope(Enum::RenderGraphPassViewScope::AllViews);

    // シーンカラー用レンダーターゲットテクスチャのリソース状態をRENDER_TARGETに遷移してから
    // シーンカラーテクスチャに書き込む
    WriteRenderTarget(Enum::RenderGraphRenderTargetType::SceneColor, Enum::RenderGraphResourceUsage::RenderTarget);

    // シーン深度用デプスステンシルテクスチャのリソース状態をDEPTH_WRITEにしてから
    // シーン深度へDepthを書き込む
    WriteDepthStencil(Enum::RenderGraphDepthStencilType::SceneDepth, Enum::RenderGraphResourceUsage::DepthWrite);

    // ShadowPassがDepthを書き込んだTexture2DArrayを、
    // LitPixelShaderから読み取れる状態へ遷移する
    ReadShadowMap(Enum::RenderGraphShadowMapType::Cascade, Enum::RenderGraphResourceUsage::PixelShaderResource);
}
FWK::Graphics::StaticModelStandardLitPass::~StaticModelStandardLitPass() = default;

void FWK::Graphics::StaticModelStandardLitPass::Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph)
{
    const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

    // パイプラインステート、ルートシグネチャをセット
    const auto& l_rootSignature        = SetupGraphicsRenderPipeline          (Enum::PipelineStateType::StaticModelLit, a_renderer).lock();
    const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock                                                   ();

    const auto& l_modelRenderSystem         = a_renderer.GetREFModelRenderSystem        ();
    const auto& l_currentFrameResourceIndex = a_renderer.GetREFCurrentFrameResourceIndex();

    // StaticModelで、StandardLitのマテリアルを使うメッシュの描画項目の一覧を描く
    // 描くマテリアルは、マテリアルのGPUデータの型で指定する(このパスのPSが読む型)
    l_modelRenderSystem.RecordDraw<ModelStandardLitMaterialGPUData>(*l_rootSignature,
                                                                    l_directCommandList,
                                                                    l_currentFrameResourceIndex,
                                                                    Enum::ModelMeshType::Static);
}