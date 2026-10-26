#include "GPUParticleDrawPass.h"

FWK::Graphics::GPUParticleDrawPass::GPUParticleDrawPass()
{
    SetupExecutionLayer(Enum::RenderGraphPassExecutionLayer::Particle);

    // モデルを描き終えたSceneColorへ、粒子を半透明で重ねる
    WriteRenderTarget(Enum::RenderGraphRenderTargetType::SceneColor, Enum::RenderGraphResourceUsage::RenderTarget);

    // SceneDepthをDSVとして一緒にセットする
    // RenderGraph上はWriteDepthStencilだが、PipelineStateでDepthWriteMaskをZeroにしているため、
    // 実際には深度を書き込まず、モデルに隠れる粒子を消す深度テストだけを行う
    WriteDepthStencil(Enum::RenderGraphDepthStencilType::SceneDepth, Enum::RenderGraphResourceUsage::DepthWrite);
}
FWK::Graphics::GPUParticleDrawPass::~GPUParticleDrawPass() = default;

void FWK::Graphics::GPUParticleDrawPass::Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph)
{
    const auto& l_gpuParticlePerObjectDrawRequest = a_renderGraph.FindVALDrawRequestPerObject<GPUParticlePerObjectDrawRequest>().lock();

    FWK_ASSERT_RETURN_IF(!l_gpuParticlePerObjectDrawRequest, "GPUParticlePerObjectDrawRequestが無効のため、GPUParticleDrawPassの実行に失敗しました。");

    // まだ一度もSimulateしていなければ、描く物も無いためPipelineStateの設定もしない
    if (!l_gpuParticlePerObjectDrawRequest->IsCreated()) { return; }

    const auto& l_cameraPassDrawRequest = a_renderGraph.FindVALDrawRequestPass<CameraPassDrawRequest>().lock();

    FWK_ASSERT_RETURN_IF(!l_cameraPassDrawRequest, "CameraPassDrawRequestが無効のため、GPUParticleDrawPassの実行に失敗しました。");

    const auto& l_rootSignature        = SetupGraphicsRenderPipeline          (a_renderer, Enum::PipelineStateType::GPUParticleBillboard).lock();
    const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock                                                         ();

    FWK_ASSERT_RETURN_IF(!l_rootSignature,        "GPUParticleBillboard用RootSignatureが無効のため、GPUParticleDrawPassの実行に失敗しました。");
    FWK_ASSERT_RETURN_IF(!l_currentFrameResource, "現在のFrameResourceが無効のため、GPUParticleDrawPassの実行に失敗しました。");

    const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

    FWK_ASSERT_RETURN_IF(!l_cameraPassDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource), "カメラ定数の設定ができないため、GPUParticleDrawPassの実行に失敗しました。");

    l_gpuParticlePerObjectDrawRequest->SetupPerObjectConstantBuffer(a_renderer, *l_rootSignature, *l_currentFrameResource);
}