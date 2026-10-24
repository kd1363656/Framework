#pragma once

namespace FWK::Graphics
{
    class StaticModelStandardUnLitPass final : public RenderGraphPassBase
    {
    public:

         StaticModelStandardUnLitPass();
        ~StaticModelStandardUnLitPass() override;

        void Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph) override;

    private:

        // GPUドリブン描画のView番号(カメラは1つだけなので0番)
        static constexpr std::size_t k_cameraViewIndex = 0ULL;

        // カメラのカリングでは、カメラより手前の物も除外する
        static constexpr bool k_useNearPlaneForCulling = true;

        FWK_DEFINE_TYPE_INFO(StaticModelStandardUnLitPass, RenderGraphPassBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::StaticModelStandardUnLitPass)