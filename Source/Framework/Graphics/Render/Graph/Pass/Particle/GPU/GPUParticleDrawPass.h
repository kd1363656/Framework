#pragma once

namespace FWK::Graphics
{
    // GPUパーティクルをビルボード(常にカメラの方を向く板)として描くPass
    // 半透明の粒子をモデルの上に重ねるため、モデルの後に実行し、深度は比べるだけで書き込まない
    class GPUParticleDrawPass final : public RenderGraphPassBase
    {
    public:

         GPUParticleDrawPass();
        ~GPUParticleDrawPass() override;

        void Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph) override;

    private:

        FWK_DEFINE_TYPE_INFO(GPUParticleDrawPass, RenderGraphPassBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::GPUParticleDrawPass)