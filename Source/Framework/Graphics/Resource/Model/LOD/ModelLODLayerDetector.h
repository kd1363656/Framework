#pragma once

namespace FWK::Graphics
{
    // 頂点ごとに「外側に重なっている別のMeshの数」(覆われている層の数)を自動で求めるクラス
    //
    // LODは三角形を減らすときに曲面を弦のように横切るため、表面が元の位置から少しずれる。
    // 服が肌に密着していると、服が肌の下へ沈んで肌が透けて見えてしまう。
    // そこで描画時に、覆われている頂点だけを「覆われている層の数 × LODの許容誤差」だけカメラから見て奥へ押し出す。
    // (肌はInnerの奥、Innerはスカートの奥…と、下の層ほど奥になるので、深度の比べ合いの結果(前後関係)が崩れない)
    // 視線の上で動かすので画面に映る位置は変わらず、形や輪郭は元のまま。外側に何も無い頂点は動かさない
    //
    // 求め方
    // 頂点ごとに、近くにある別のMeshの頂点のうち、法線の向き(外側)にある物を探し、何種類のMeshがあるかを数える
    class ModelLODLayerDetector final
    {
    private:

        struct LayerVertex final
        {
            TypeAlias::Math::Vector3 m_position = {};
            TypeAlias::Math::Vector3 m_normal   = {};

            std::uint32_t m_meshIndex = {};
        };

    public:

         ModelLODLayerDetector() = default;
        ~ModelLODLayerDetector() = default;

        // 全Meshの頂点ごとの覆われている層の数を求め、ModelMeshletData::m_lodCoveredLayerCountListへ書き込む
        // StaticModelとSkeletalAnimationModelのModelMeshは別の型だが、使う変数名が同じなのでtemplateで共通化する
        template <typename ModelMeshListType>
        void DetectModelMeshLayer(ModelMeshListType& a_modelMeshList) const
        {
            std::vector<LayerVertex> l_layerVertexList = {};

            for (std::size_t l_meshIndex = 0ULL; l_meshIndex < a_modelMeshList.size(); ++l_meshIndex)
            {
                for (const auto& l_modelVertex : a_modelMeshList[l_meshIndex].m_modelVertexList)
                {
                    LayerVertex l_layerVertex = {};

                    l_layerVertex.m_position  = l_modelVertex.m_position;
                    l_layerVertex.m_normal    = l_modelVertex.m_normal;
                    l_layerVertex.m_meshIndex = static_cast<std::uint32_t>(l_meshIndex);

                    l_layerVertexList.emplace_back(l_layerVertex);
                }
            }

            // 全Meshの頂点を1本に並べて求めたので、Meshごとに切り分けて書き込む
            const auto& l_coveredLayerCountList = DetectCoveredLayerCountList(l_layerVertexList);

            std::size_t l_layerVertexIndex = k_initialLayerVertexCount;

            for (auto& l_modelMesh : a_modelMeshList)
            {
                const auto l_vertexCount = l_modelMesh.m_modelVertexList.size();

                const auto l_beginItr = l_coveredLayerCountList.begin() + static_cast<std::ptrdiff_t>(l_layerVertexIndex);

                l_modelMesh.m_modelMeshletData.m_lodCoveredLayerCountList.assign(l_beginItr, l_beginItr + static_cast<std::ptrdiff_t>(l_vertexCount));

                l_layerVertexIndex += l_vertexCount;
            }
        }

    private:

        std::vector<std::uint32_t> DetectCoveredLayerCountList(const std::vector<LayerVertex>& a_layerVertexList) const;

        std::uint64_t CalculateCellKey(const std::int64_t a_cellX, const std::int64_t a_cellY, const std::int64_t a_cellZ) const;

        std::int64_t CalculateCellCoordinate(const float a_position, const float a_cellSize) const;

        static constexpr float k_layerSearchRadiusScale = 0.01F;
        static constexpr float k_minNormalAlignment     = 0.5F;
        static constexpr float k_minNormalLengthSquared = 0.000001F;

        static constexpr std::int64_t k_neighborCellOffsetMIN = -1LL;
        static constexpr std::int64_t k_neighborCellOffsetMAX =  1LL;

        static constexpr std::int64_t k_cellCoordinateBias = 1LL << 20;

        static constexpr std::uint64_t k_cellCoordinateMask      = (1ULL << 21) - 1ULL;
        static constexpr std::uint64_t k_cellKeyShiftBitY        = 21ULL;
        static constexpr std::uint64_t k_cellKeyShiftBitZ        = 42ULL;
        static constexpr std::size_t   k_initialLayerVertexCount = 0ULL;

        static constexpr std::uint32_t k_initialCoveredLayerCount = 0U;
    };
}