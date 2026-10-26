#include "GPUParticleSimulatePass.h"
#include "../../../../../../../Application/Application.h"

FWK::Graphics::GPUParticleSimulatePass::GPUParticleSimulatePass()
{
    // 粒子の描画より前に、発生と移動を終わらせる
    SetupExecutionLayer(Enum::RenderGraphPassExecutionLayer::ParticleSimulate);
}
FWK::Graphics::GPUParticleSimulatePass::~GPUParticleSimulatePass() = default;

void FWK::Graphics::GPUParticleSimulatePass::Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph)
{
    const auto& l_gpuParticlePerObjectDrawRequest = a_renderGraph.FindVALDrawRequestPerObject<GPUParticlePerObjectDrawRequest>().lock();

    FWK_ASSERT_RETURN_IF(!l_gpuParticlePerObjectDrawRequest, "GPUParticlePerObjectDrawRequestが無効のため、GPUParticleSimulatePassの実行に失敗しました。");

    const auto& l_application   = Application::GetInstance             ();
    const auto& l_fpsController = l_application.GetREFFPSController    ();
    const float l_deltaTime     = l_fpsController.GetVALScaledDeltaTime();

    FWK_ASSERT_RETURN_IF(!l_gpuParticlePerObjectDrawRequest->Simulate(a_renderer, l_deltaTime), "GPUパーティクルのSimulateに失敗しました。");
}