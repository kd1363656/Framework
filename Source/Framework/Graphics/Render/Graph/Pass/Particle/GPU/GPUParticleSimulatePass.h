#pragma once

namespace FWK::Graphics
{
    // GPUパーティクルの発生と移動をGPUで行うPass(描画はGPUParticleDrawPassが行う)
    // 別のカメラからの追加の描画では実行されないため、粒子が1フレームに2回動くことは無い
    class GPUParticleSimulatePass final : public RenderGraphPassBase
    {
    public:

         GPUParticleSimulatePass();
        ~GPUParticleSimulatePass() override;

        void Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph) override;

    private:

        FWK_DEFINE_TYPE_INFO(GPUParticleSimulatePass, RenderGraphPassBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::GPUParticleSimulatePass)