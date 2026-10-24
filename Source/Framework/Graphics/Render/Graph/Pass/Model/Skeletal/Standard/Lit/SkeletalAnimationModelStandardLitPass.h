#pragma once

namespace FWK::Graphics
{
    class SkeletalAnimationModelStandardLitPass final : public RenderGraphPassBase
    {
    public:

         SkeletalAnimationModelStandardLitPass();
        ~SkeletalAnimationModelStandardLitPass() override;

        void Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph) override;

    private:

        // FrameProfilerへ記録する、描画した三角形数の名前
        static constexpr std::string_view k_triangleCounterName           = "Triangles : SkeletalAnimationModel Lit";
        static constexpr std::string_view k_gpuSkinnedTriangleCounterName = "Triangles : SkeletalAnimationModel GPUSkinned Lit";

        // GPUドリブン描画のView番号(カメラは1つだけなので0番)
        static constexpr std::size_t k_cameraViewIndex = 0ULL;

        // カメラのカリングでは、カメラより手前の物も除外する
        static constexpr bool k_useNearPlaneForCulling = true;

        FWK_DEFINE_TYPE_INFO(SkeletalAnimationModelStandardLitPass, RenderGraphPassBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::SkeletalAnimationModelStandardLitPass)