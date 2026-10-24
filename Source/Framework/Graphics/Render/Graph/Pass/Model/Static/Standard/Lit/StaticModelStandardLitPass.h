#pragma once

namespace FWK::Graphics
{
    class StaticModelStandardLitPass final : public RenderGraphPassBase
    {
    public:

         StaticModelStandardLitPass();
        ~StaticModelStandardLitPass() override;

        void Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph) override;

    private:

        // FrameProfilerへ記録する、描画した三角形数の名前
        static constexpr std::string_view k_triangleCounterName = "Triangles : StaticModel Lit";

        // GPUドリブン描画のView番号
        // オクルージョンカリングは2回に分けて行うため、1回目と2回目で別のViewを使う
        static constexpr std::size_t k_earlyCullingViewIndex = 0ULL;
        static constexpr std::size_t k_lateCullingViewIndex  = 1ULL;

        // カメラのカリングでは、カメラより手前の物も除外する
        static constexpr bool k_useNearPlaneForCulling = true;

        // オクルージョンカリングに使うHi-Z(前のフレームの物を次のフレームでも使うため、Passが持ち続ける)
        HierarchicalDepthTexture m_hierarchicalDepthTexture;

        // 前のフレームのビュープロジェクション行列(前のフレームのHi-Zで判定するときに使う)
        TypeAlias::Math::Matrix m_previousViewProjectionMatrix = TypeAlias::Math::Matrix::Identity;

        bool m_hasPreviousViewProjectionMatrix = false;

        FWK_DEFINE_TYPE_INFO(StaticModelStandardLitPass, RenderGraphPassBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::StaticModelStandardLitPass)