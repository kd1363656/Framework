#pragma once

namespace FWK::Graphics
{
    class StaticModelStandardLitPass final : public RenderGraphPassBase
    {
    public:

         StaticModelStandardLitPass();
        ~StaticModelStandardLitPass() override;

        void Deserialize(const nlohmann::json& a_rootJson) override;

        void Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph) override;

        nlohmann::json Serialize() const override;

        void EditInspector() override;

        // オクルージョンカリング(Hi-Zで、手前の物に完全に隠れている物を描画しない)を行うか
        // 半透明の物を描く場合など、手前の物で奥を隠したくない場合はfalseにする
        void SetIsOcclusionCullingEnabled(const bool a_set) { m_isOcclusionCullingEnabled = a_set; }

        bool IsOcclusionCullingEnabled() const { return m_isOcclusionCullingEnabled; }

    private:

        // GPUドリブン描画のView番号
        // オクルージョンカリングは2回に分けて行うため、1回目と2回目で別のViewを使う
        static constexpr std::size_t k_earlyCullingViewIndex = 0ULL;
        static constexpr std::size_t k_lateCullingViewIndex  = 1ULL;

        static constexpr bool k_useNearPlaneForCulling = true;

        // オクルージョンカリングに使うHi-Z(前のフレームの物を次のフレームでも使うため、Passが持ち続ける)
        HierarchicalDepthTexture m_hierarchicalDepthTexture;

        StaticModelStandardLitPassEditInspector m_editInspector = {};

        Converter::StaticModelStandardLitPassJsonConverter m_jsonConverter = {};

        TypeAlias::Math::Matrix m_previousViewProjectionMatrix = TypeAlias::Math::Matrix::Identity;

        bool m_hasPreviousViewProjectionMatrix = false;
        bool m_isOcclusionCullingEnabled       = Constant::k_defaultStaticModelStandardLitPassIsOcclusionCullingEnabled;

        FWK_DEFINE_TYPE_INFO(StaticModelStandardLitPass, RenderGraphPassBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::StaticModelStandardLitPass)