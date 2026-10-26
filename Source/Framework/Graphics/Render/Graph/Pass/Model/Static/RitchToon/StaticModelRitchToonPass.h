#pragma once

namespace FWK::Graphics
{
    // RitchToonMaterialを使うStaticModelを描画するPass
    // 1. 表面をトゥーン調で描く
    // 2. 同じMeshを、頂点を外側へ押し出して裏面だけ描き、輪郭線にする
    // ※ オクルージョンカリング(Hi-Z)は行わず、視錐台の判定とLODの選択だけを行う
    //   (Hi-ZはStaticModelStandardLitPassが自身の描画結果から作る物で、このPassの描画結果は入らないため)
    class StaticModelRitchToonPass final : public RenderGraphPassBase
    {
    public:

         StaticModelRitchToonPass();
        ~StaticModelRitchToonPass() override;

        void Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph) override;

    private:

        // GPUドリブン描画のView番号
        // 描画コマンドは1回実行すると使い終わった扱いになるため、表面と輪郭線で別のViewとしてカリングする
        static constexpr std::size_t k_surfaceViewIndex = 0ULL;
        static constexpr std::size_t k_outlineViewIndex = 1ULL;

        static constexpr bool k_useNearPlaneForCulling = true;

        FWK_DEFINE_TYPE_INFO(StaticModelRitchToonPass, RenderGraphPassBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::StaticModelRitchToonPass)