#include "RenderViewCapture.h"

void FWK::Graphics::RenderViewCapture::BeginFrame()
{
    m_isCapturedThisFrame = false;
}
void FWK::Graphics::RenderViewCapture::Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer)
{
    // 依頼は1回描いたら消す(毎フレーム依頼し直さない限り、余計な描画をしない)
    const auto& l_cameraConstantBuffer = m_requestCameraConstantBuffer.lock();

    m_requestCameraConstantBuffer = {};

    if (!l_cameraConstantBuffer) { return; }

          auto& l_renderGraph           = a_renderer.GetMutableREFRenderGraph                         ();
    const auto& l_cameraPassDrawRequest = l_renderGraph.FindVALDrawRequestPass<CameraPassDrawRequest>().lock();
    const auto& l_currentFrameResource  = a_renderer.GetREFCurrentFrameResource                       ().lock();

    FWK_ASSERT_RETURN_IF(!l_cameraPassDrawRequest, "CameraPassDrawRequestが無効のため、追加のカメラの描画に失敗しました。");
    FWK_ASSERT_RETURN_IF(!l_currentFrameResource,  "現在のFrameResourceが無効のため、追加のカメラの描画に失敗しました。");

    const auto& l_renderGraphFrameResource = l_currentFrameResource->GetREFRenderGraphFrameResource                         ();
    const auto& l_finalColorPassTexture    = l_renderGraphFrameResource.FindVALRenderTargetPassTexture(Enum::RenderGraphRenderTargetType::FinalColor).lock();

    FWK_ASSERT_RETURN_IF(!l_finalColorPassTexture, "FinalColorのPassTextureが無効のため、追加のカメラの描画に失敗しました。");

    // コピー先のTextureを、FinalColorと同じ大きさ、同じFormatで用意する
    if (!PrepareCaptureTexture(a_renderer, *l_finalColorPassTexture)) { return; }

    // 今の描画カメラ、カリングカメラの設定を覚えておき、追加のカメラへ差し替える
    // (カリングも追加のカメラで行うので、そのカメラから見える物がそのまま描かれる)
    const auto  l_previousOverrideSource     = l_cameraPassDrawRequest->GetREFOverrideSourceConstantBuffer();
    const auto  l_previousCullingSource      = l_cameraPassDrawRequest->GetREFCullingSourceConstantBuffer ();
    const bool  l_previousIsCullingVisualize = l_cameraPassDrawRequest->GetVALIsCullingDebugVisualize     ();

    l_cameraPassDrawRequest->SetOverrideSourceConstantBuffer(l_cameraConstantBuffer);
    l_cameraPassDrawRequest->SetCullingSourceConstantBuffer (l_cameraConstantBuffer);
    l_cameraPassDrawRequest->SetIsCullingDebugVisualize     (false);

    l_renderGraph.ExecuteAdditionalView(a_resourceContext, a_renderer);

    // 描き上がったFinalColorを、このクラスのTextureへ写す
    const auto& l_directCommandList   = a_renderer.GetREFDirectCommandList                      ();
          auto& l_finalColorTexture   = l_finalColorPassTexture->GetMutableREFRenderTargetTexture();

    CopyFinalColor(l_directCommandList, l_finalColorTexture);

    // 描画カメラ、カリングカメラを元に戻す
    l_cameraPassDrawRequest->SetOverrideSourceConstantBuffer(l_previousOverrideSource);
    l_cameraPassDrawRequest->SetCullingSourceConstantBuffer (l_previousCullingSource);
    l_cameraPassDrawRequest->SetIsCullingDebugVisualize     (l_previousIsCullingVisualize);

    // メインの描画が真っ新な状態から始まるよう、SceneColor、FinalColor、SceneDepthをクリアし直す
    l_renderGraph.ClearCurrentFramePassTextureList(a_resourceContext, a_renderer);

    m_isCapturedThisFrame = true;
}

bool FWK::Graphics::RenderViewCapture::PrepareCaptureTexture(const Renderer& a_renderer, const RenderTargetPassTexture& a_sourcePassTexture)
{
    const auto& l_sourceTexture = a_sourcePassTexture.GetREFRenderTargetTexture();

    const auto l_width  = l_sourceTexture.GetVALWidth ();
    const auto l_height = l_sourceTexture.GetVALHeight();

    auto& l_graphicsManager         = GraphicsManager::GetInstance                          ();
    auto& l_resourceContext         = l_graphicsManager.GetMutableREFResourceContext        ();
    auto& l_rtvDescriptorPool       = l_resourceContext.GetMutableREFRTVDescriptorPool      ();
    auto& l_cbvSRVUAVDescriptorPool = l_resourceContext.GetMutableREFCBVSRVUAVDescriptorPool();

    const auto& l_device             = l_graphicsManager.GetREFDevice           ();
    const auto& l_gpuMemoryAllocator = l_resourceContext.GetREFGPUMemoryAllocator();

    // まだ作っていなければ作る
    if (!m_captureTexture.IsCreated())
    {
        FWK_ASSERT_RETURN_VALUE_IF(!m_captureTexture.Create(l_device,
                                                            l_gpuMemoryAllocator,
                                                            a_sourcePassTexture.GetREFClearColor(),
                                                            l_sourceTexture.GetVALFormat(),
                                                            l_width,
                                                            l_height,
                                                            l_rtvDescriptorPool,
                                                            l_cbvSRVUAVDescriptorPool),
                                                            "追加のカメラの描画結果をコピーするTextureの作成に失敗しました。",
                                                            false);

        // 使う側(エディター)がSRVを取り直せるように、作り直すたびに番号を進める
        m_captureTextureVersion += k_captureTextureVersionStep;

        return true;
    }

    // 大きさが同じなら作り直さない(FinalColorはウィンドウのリサイズでのみ大きさが変わる)
    if (m_captureTexture.GetVALWidth () == l_width &&
        m_captureTexture.GetVALHeight() == l_height)
    {
        return true;
    }

    // 古いTextureは、GPUが前のフレームまでの命令で使い終わってから解放する(遅延解放)
    const auto& l_directCommandQueue = a_renderer.GetREFDirectCommandQueue                 ();
    const auto& l_retiredFenceValue  = l_directCommandQueue.FetchREFLastSignaledFenceValue   ();
          auto& l_releaseContext     = l_resourceContext.GetMutableREFResourceReleaseContext();

    FWK_ASSERT_RETURN_VALUE_IF(!m_captureTexture.Resize(l_device,
                                                        l_gpuMemoryAllocator,
                                                        l_retiredFenceValue,
                                                        l_width,
                                                        l_height,
                                                        l_rtvDescriptorPool,
                                                        l_cbvSRVUAVDescriptorPool,
                                                        l_releaseContext),
                                                        "追加のカメラの描画結果をコピーするTextureのリサイズに失敗しました。",
                                                        false);

    m_captureTextureVersion += k_captureTextureVersionStep;

    return true;
}

void FWK::Graphics::RenderViewCapture::CopyFinalColor(const DirectCommandList& a_directCommandList, RenderTargetTexture& a_sourceTexture)
{
    const auto l_sourceBeforeState = a_sourceTexture.GetVALCurrentResourceState();

    // コピー元はCOPY_SOURCE、コピー先はCOPY_DESTの状態にしておく決まりがある
    TransitionTexture(a_directCommandList, a_sourceTexture,  D3D12_RESOURCE_STATE_COPY_SOURCE);
    TransitionTexture(a_directCommandList, m_captureTexture, D3D12_RESOURCE_STATE_COPY_DEST);

    const auto& l_sourceResource      = a_sourceTexture.GetREFGPUResource ();
    const auto& l_destinationResource = m_captureTexture.GetREFGPUResource();

    a_directCommandList.CopyResource(*l_destinationResource.m_resource.Get(), *l_sourceResource.m_resource.Get());

    // コピー元は元の状態へ戻し、コピー先はImGuiが読めるようPixelShaderResourceにする
    TransitionTexture(a_directCommandList, a_sourceTexture,  l_sourceBeforeState);
    TransitionTexture(a_directCommandList, m_captureTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void FWK::Graphics::RenderViewCapture::TransitionTexture(const DirectCommandList& a_directCommandList, RenderTargetTexture& a_texture, const D3D12_RESOURCE_STATES a_afterState) const
{
    const auto l_beforeState = a_texture.GetVALCurrentResourceState();

    if (l_beforeState == a_afterState) { return; }

    const auto& l_gpuResource = a_texture.GetREFGPUResource();

    a_directCommandList.TransitionResourceBarrier(l_gpuResource.m_resource, l_beforeState, a_afterState);

    a_texture.SetCurrentResourceState(a_afterState);
}
