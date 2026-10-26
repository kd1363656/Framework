#pragma once

namespace FWK::Graphics
{
    // RitchToonMaterialを使うSkeletalAnimationModelを描画するPass
    // 従来の方式(ComputeShaderでスキニング)とGPUスキニングの両方について、
    // 1. 表面をトゥーン調で描く
    // 2. 同じMeshを、頂点を外側へ押し出して裏面だけ描き、輪郭線にする
    class SkeletalAnimationModelRitchToonPass final : public RenderGraphPassBase
    {
    public:

         SkeletalAnimationModelRitchToonPass();
        ~SkeletalAnimationModelRitchToonPass() override;

        void Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph) override;

    private:

        // GPUスキニングの描画のView番号
        // 描画コマンドは1回実行すると使い終わった扱いになるため、表面と輪郭線で別のViewとしてカリングする
        static constexpr std::size_t k_surfaceViewIndex = 0ULL;
        static constexpr std::size_t k_outlineViewIndex = 1ULL;

        static constexpr bool k_useNearPlaneForCulling = true;

        FWK_DEFINE_TYPE_INFO(SkeletalAnimationModelRitchToonPass, RenderGraphPassBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::SkeletalAnimationModelRitchToonPass)