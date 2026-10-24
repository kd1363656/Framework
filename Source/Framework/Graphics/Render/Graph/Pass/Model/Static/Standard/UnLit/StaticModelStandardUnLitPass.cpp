#include "StaticModelStandardUnLitPass.h"

FWK::Graphics::StaticModelStandardUnLitPass::StaticModelStandardUnLitPass()
{
    SetupExecutionLayer(Enum::RenderGraphPassExecutionLayer::Model);

    // シーンカラー用レンダーターゲットテクスチャのリソース状態をRENDER_TARGETに遷移してから
    // シーンカラーテクスチャに書き込む
    WriteRenderTarget(Enum::RenderGraphRenderTargetType::SceneColor, Enum::RenderGraphResourceUsage::RenderTarget);

    // シーン深度用デプスステンシルテクスチャのリソース状態をDEPTH_WRITEにしてから
    // シーン深度へDepthを書き込む
    WriteDepthStencil(Enum::RenderGraphDepthStencilType::SceneDepth, Enum::RenderGraphResourceUsage::DepthWrite);
}
FWK::Graphics::StaticModelStandardUnLitPass::~StaticModelStandardUnLitPass() = default;

void FWK::Graphics::StaticModelStandardUnLitPass::Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph)
{
    const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

    const auto& l_cameraPassDrawRequest                   = a_renderGraph.FindVALDrawRequestPass<CameraPassDrawRequest>                            ().lock();
    const auto& l_staticModelStandardPerObjectDrawRequest = a_renderGraph.FindVALDrawRequestPerObject<StaticModelStandardUnLitPerObjectDrawRequest>().lock();

    FWK_ASSERT_RETURN_IF(!l_cameraPassDrawRequest,                   "カメラパスのポインタが無効になっており、StaticModelStandardUnLitPassの実行に失敗しました。");
    FWK_ASSERT_RETURN_IF(!l_staticModelStandardPerObjectDrawRequest, "StaticModelStandardPerObjectDrawRequestが無効のため、StaticModelStandardUnLitPassの実行に失敗しました。");

    // GPUドリブン描画のカリング
    // カメラから見える範囲(視錐台)に入っているMeshだけを選び、LODを決めて描画コマンドを作る
    // ※ ComputeShader用のPipelineStateを使うため、描画用PipelineStateを設定する前に行う
    if (const auto& l_cbCameraPass = l_cameraPassDrawRequest->GetREFSourceConstantBuffer().lock();
        l_cbCameraPass)
    {
        const auto& l_cullingView = ModelGPUDrivenDrawer::BuildCullingView(l_cbCameraPass->m_viewProjectionMatrix, k_useNearPlaneForCulling);

        l_staticModelStandardPerObjectDrawRequest->DispatchGPUCulling(a_renderer, k_cameraViewIndex, l_cullingView);
    }

    // パイプラインステート、ルートシグネチャをセット
    const auto& l_rootSignature        = SetupGraphicsRenderPipeline          (a_renderer, Enum::PipelineStateType::StaticModelUnLit).lock();
    const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock                                                     ();

    FWK_ASSERT_RETURN_IF(!l_rootSignature,        "ルートシグネチャの取得に失敗しており、StaticModelStandardUnLitPassの実行に失敗しました。");
    FWK_ASSERT_RETURN_IF(!l_currentFrameResource, "現在のフレームリソースの取得に失敗しており、StaticModelStandardUnLitPassの実行に失敗しました。");

    FWK_ASSERT_RETURN_IF(!l_cameraPassDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource), "カメラ定数の設定が出来ておらず、StaticModelStandardUnLitPassの実行に失敗しました。");

    // カリングで作った描画コマンドを、ExecuteIndirectでまとめて実行する
    l_staticModelStandardPerObjectDrawRequest->SetCurrentDrawViewIndex     (k_cameraViewIndex);
    l_staticModelStandardPerObjectDrawRequest->SetupPerObjectConstantBuffer(a_renderer, *l_rootSignature, *l_currentFrameResource);
}