#include "StaticModelStandardLitPass.h"

FWK::Graphics::StaticModelStandardLitPass::StaticModelStandardLitPass()
{
    SetupExecutionLayer(Enum::RenderGraphPassExecutionLayer::Model);

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

void FWK::Graphics::StaticModelStandardLitPass::Deserialize(const nlohmann::json& a_rootJson)
{
    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Graphics::StaticModelStandardLitPass::Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph)
{
    const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

    const auto& l_cameraPassDrawRequest                   = a_renderGraph.FindVALDrawRequestPass     <CameraPassDrawRequest>                     ().lock();
    const auto& l_lightPassDrawRequest                    = a_renderGraph.FindVALDrawRequestPass     <LightPassDrawRequest>                      ().lock();
    const auto& l_cascadeShadowMapPassDrawRequest         = a_renderGraph.FindVALDrawRequestPass     <CascadeShadowMapPassDrawRequest>           ().lock();
    const auto& l_staticModelStandardPerObjectDrawRequest = a_renderGraph.FindVALDrawRequestPerObject<StaticModelStandardLitPerObjectDrawRequest>().lock();

    FWK_ASSERT_RETURN_IF(!l_cameraPassDrawRequest,                   "カメラパスのポインタが無効になっており、StaticModelStandardLitPassの実行に失敗しました。");
    FWK_ASSERT_RETURN_IF(!l_lightPassDrawRequest,                    "ライトパスのポインタが無効になっており、StaticModelStandardLitPassの実行に失敗しました。");
    FWK_ASSERT_RETURN_IF(!l_cascadeShadowMapPassDrawRequest,         "CascadeShadowMapPassDrawRequestが無効のため、StaticModelStandardLitPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_staticModelStandardPerObjectDrawRequest, "StaticModelStandardPerObjectDrawRequestが無効のため、StaticModelStandardLitPassの実行に失敗しました。");

    // カリングは「カリングに使うカメラ」(通常は描画カメラ、デバッグカメラで描画中はメインカメラ)で行う
    const auto& l_cbCameraPass = l_cameraPassDrawRequest->FetchVALCullingConstantBuffer();

    // Hi-Zは直前のフレームの描画カメラの深度から作るため、描画カメラを差し替えている間(デバッグカメラ、カメラのプレビュー)は
    // 判定が合わず、オクルージョンカリングを行えない
    const bool l_isOcclusionCullingEnabled = m_isOcclusionCullingEnabled &&
                                             l_cameraPassDrawRequest->FetchVALIsOcclusionCullingAllowed();

    // GPUドリブン描画のカリング(1回目)
    // カメラから見える範囲(視錐台)に入っていて、前のフレームのHi-Zで隠れていないMeshだけを選び、
    // LODを決めて描画コマンドを作る(隠れていた物は2回目の候補として残す)
    // オクルージョンカリングを行わない場合は、視錐台の判定とLODの選択だけを行う(Normal)
    // ※ ComputeShader用のPipelineStateを使うため、描画用PipelineStateを設定する前に行う
    if (l_cbCameraPass)
    {
        auto l_cullingView = ModelGPUDrivenDrawer::BuildCullingView(l_cbCameraPass->m_viewProjectionMatrix, k_useNearPlaneForCulling);

        l_cullingView.m_cullingMode             = l_isOcclusionCullingEnabled ? Enum::ModelGPUDrivenCullingMode::Early : Enum::ModelGPUDrivenCullingMode::Normal;
        l_cullingView.m_isCullingDebugVisualize = l_cameraPassDrawRequest->GetVALIsCullingDebugVisualize();

        // 前のフレームのHi-Zは、前のフレームのカメラから見た深度なので、前のフレームの行列で判定する
        if (l_isOcclusionCullingEnabled &&
            m_hasPreviousViewProjectionMatrix)
        {
            l_cullingView.m_hierarchicalDepthTexture      = &m_hierarchicalDepthTexture;
            l_cullingView.m_occlusionViewProjectionMatrix = m_previousViewProjectionMatrix;
        }

        l_staticModelStandardPerObjectDrawRequest->DispatchGPUCulling(a_renderer, k_earlyCullingViewIndex, l_cullingView);
    }

    // パイプラインステート、ルートシグネチャをセット
    const auto& l_rootSignature        = SetupGraphicsRenderPipeline          (a_renderer, Enum::PipelineStateType::StaticModelLit).lock();
    const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock                                                   ();

    FWK_ASSERT_RETURN_IF(!l_rootSignature,        "ロートシグネチャの取得に失敗しており、StaticModelStandardLitPassの実行に失敗しました。");
    FWK_ASSERT_RETURN_IF(!l_currentFrameResource, "現在のフレームリソースの取得に失敗しており、StaticModelStandardLitPassの実行に失敗しました。");

    FWK_ASSERT_RETURN_IF(!l_cameraPassDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource),           "カメラ定数の設定が出来ておらず、StaticModelStandardLitPassの実行に失敗しました。");
    FWK_ASSERT_RETURN_IF(!l_cascadeShadowMapPassDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource), "CascadeShadowMap定数の設定ができないため、StaticModelStandardLitPassを実行できません。");
    FWK_ASSERT_RETURN_IF(!l_lightPassDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource),            "ライト定数の設定が出来ておらず、StaticModelStandardLitPassの実行に失敗しました。");

    // 1回目のカリングで作った描画コマンドを、ExecuteIndirectでまとめて実行する
    l_staticModelStandardPerObjectDrawRequest->SetCurrentDrawViewIndex     (k_earlyCullingViewIndex);
    l_staticModelStandardPerObjectDrawRequest->SetupPerObjectConstantBuffer(a_renderer, *l_rootSignature, *l_currentFrameResource);

    if (l_cbCameraPass &&
        l_isOcclusionCullingEnabled)
    {
        // 1回目で描いた物の深度から、今のフレームのHi-Zを作る
        // (このHi-Zは2回目のカリングと、次のフレームの1回目のカリングで使う)
        const auto& l_renderGraphFrameResource = l_currentFrameResource->GetREFRenderGraphFrameResource();
        const auto& l_sceneDepthPassTexture    = l_renderGraphFrameResource.FindVALDepthStencilPassTexture(Enum::RenderGraphDepthStencilType::SceneDepth).lock();

        if (l_sceneDepthPassTexture &&
            m_hierarchicalDepthTexture.Build(a_renderer, l_sceneDepthPassTexture->GetMutableREFDepthStencilTexture()))
        {
            // GPUドリブン描画のカリング(2回目)
            // 1回目で隠れていた物だけを、今のフレームのHi-Zで判定し直す
            auto l_cullingView = ModelGPUDrivenDrawer::BuildCullingView(l_cbCameraPass->m_viewProjectionMatrix, k_useNearPlaneForCulling);

            l_cullingView.m_cullingMode                   = Enum::ModelGPUDrivenCullingMode::Late;
            l_cullingView.m_hierarchicalDepthTexture      = &m_hierarchicalDepthTexture;
            l_cullingView.m_occlusionViewProjectionMatrix = l_cbCameraPass->m_viewProjectionMatrix;
            l_cullingView.m_candidateSourceViewIndex      = k_earlyCullingViewIndex;

            if (l_staticModelStandardPerObjectDrawRequest->DispatchGPUCulling(a_renderer, k_lateCullingViewIndex, l_cullingView))
            {
                // Hi-Zの作成とカリングでComputeShader用のPipelineStateに変わったため、描画用に設定し直す
                // (描画用のRootSignatureと定数バッファの設定は、ComputeShaderの設定とは別なので残っている)
                SetupGraphicsRenderPipeline(a_renderer, Enum::PipelineStateType::StaticModelLit);

                // 2回目のカリングで見えると分かった物を描画する
                l_staticModelStandardPerObjectDrawRequest->SetCurrentDrawViewIndex     (k_lateCullingViewIndex);
                l_staticModelStandardPerObjectDrawRequest->SetupPerObjectConstantBuffer(a_renderer, *l_rootSignature, *l_currentFrameResource);
            }
        }

        // 次のフレームの1回目のカリングで、今のフレームのHi-Zを正しく使えるように行列を覚えておく
        m_previousViewProjectionMatrix    = l_cbCameraPass->m_viewProjectionMatrix;
        m_hasPreviousViewProjectionMatrix = true;
    }
    else if (!m_isOcclusionCullingEnabled)
    {
        // Hi-Zを作らなかったフレームの次は、古いHi-Zで判定しないようにする
        // (オクルージョンカリングをオンに戻した最初のフレームは、隠れているかの判定をしない)
        // 描画カメラを差し替えている間(デバッグカメラ、カメラのプレビュー)は、メインカメラのHi-Zと行列をそのまま残しておく
        // (Hi-Zと行列は同じフレームの物同士なので、戻ったときに判定に使っても見えている物は消えない)
        m_hasPreviousViewProjectionMatrix = false;
    }
}

nlohmann::json FWK::Graphics::StaticModelStandardLitPass::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::Graphics::StaticModelStandardLitPass::EditInspector()
{
    m_editInspector.EditInspector(*this);
}