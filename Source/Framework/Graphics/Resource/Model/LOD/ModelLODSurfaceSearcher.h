#pragma once

namespace FWK::Graphics
{
    // LOD(三角形を減らした形)で消えた頂点が、LODのどの三角形の上に描かれるかを探すクラス
    //
    // 消えた頂点の位置は、LODでは「一番近いLODの三角形の上の点」として描かれる。
    // 消えた頂点から辺をたどって周りに残った頂点を見つけ、それらを角に持つLODの三角形の中から元の位置に一番近い点を求める。
    // (全ての三角形と比べずに済むので速い)
    //
    // 使い道
    // ・モーフの崩れ(ModelMorphLODEvaluator) : その点の移動量と、消えた頂点自身の移動量の差を測る
    // ・形のずれ(CalculateLODSurfaceDeviation) : その点が元の表面より内側か外側か、どれだけ離れているかを測る
    class ModelLODSurfaceSearcher final
    {
    public:

         ModelLODSurfaceSearcher() = default;
        ~ModelLODSurfaceSearcher() = default;

        // LODを作る前に1回呼び、頂点ごとの隣の頂点の一覧と、頂点の元の位置・法線を作る
        void BuildSearchData(const std::vector<std::uint32_t>&            a_indexList,
                             const std::vector<TypeAlias::Math::Vector3>& a_vertexPositionList,
                             const std::vector<TypeAlias::Math::Vector3>& a_vertexNormalList,
                                   Struct::ModelLODSurfaceSearchData&     a_searchData) const;

        // LOD1段階を調べる前に呼び、残った頂点の印と、頂点ごとの「その頂点を角に持つLODの三角形」の一覧を作る
        void BuildWorkData(const std::vector<std::uint32_t>& a_lodIndexList, const std::size_t a_vertexCount, Struct::ModelLODSurfaceSearchWorkData& a_workData) const;

        // 消えた頂点が描かれる、LODの三角形の上の点を求める
        // つながった部品ごと消えている(周りに残った頂点が無い)場合は、m_triangleBeginが無効値の点を返す
        Struct::ModelLODSurfacePoint FindLODSurfacePoint(const Struct::ModelLODSurfaceSearchData&     a_searchData,
                                                         const std::vector<std::uint32_t>&            a_lodIndexList,
                                                         const std::uint32_t                          a_removedVertexIndex,
                                                               Struct::ModelLODSurfaceSearchWorkData& a_workData) const;

        // LOD1段階で、LODの表面が元の表面より内側へ沈んだ距離と、外側へ出た距離を測る
        // (消えた頂点ごとに、描かれる点が元の位置から法線の向きへどれだけ離れているかを調べる)
        // 細い部品などで一部の頂点だけ大きく外れることがあるため、最大値ではなく95%の頂点が収まる距離を返す
        Struct::ModelLODSurfaceDeviation CalculateLODSurfaceDeviation(const Struct::ModelLODSurfaceSearchData& a_searchData, const std::vector<std::uint32_t>& a_lodIndexList) const;

    private:

        // 小さい順に並べたときに、a_percentileの割合の位置にある値を求める(空なら0)
        float CalculatePercentileDistance(std::vector<float>& a_distanceList, const float a_percentile) const;

        // 消えた頂点から辺をたどり、最初に見つかった周りの残った頂点をa_workData.m_keptVertexListへ集める
        // 見つからない(つながった部品ごと消えている)場合は空のまま
        void CollectNearestKeptVertexList(const Struct::ModelLODSurfaceSearchData& a_searchData, const std::uint32_t a_removedVertexIndex, Struct::ModelLODSurfaceSearchWorkData& a_workData) const;

        // 周りに残った頂点を角に持つLODの三角形の中から、消えた頂点の元の位置に一番近い点を求める
        Struct::ModelLODSurfacePoint FindNearestLODSurfacePoint(const Struct::ModelLODSurfaceSearchData&     a_searchData,
                                                                const std::vector<std::uint32_t>&            a_lodIndexList,
                                                                const std::uint32_t                          a_removedVertexIndex,
                                                                const Struct::ModelLODSurfaceSearchWorkData& a_workData) const;

        // 点に一番近い、三角形(a, b, c)の上の点の重心座標を求める(x, y, z = a, b, cの混ぜ具合で、合計1)
        TypeAlias::Math::Vector3 CalculateClosestPointBarycentric(const TypeAlias::Math::Vector3& a_point,
                                                                  const TypeAlias::Math::Vector3& a_cornerA,
                                                                  const TypeAlias::Math::Vector3& a_cornerB,
                                                                  const TypeAlias::Math::Vector3& a_cornerC) const;

        static constexpr std::size_t k_triangleCornerCount     = 3ULL;
        static constexpr std::size_t k_nextCornerOffset        = 1ULL;
        static constexpr std::size_t k_secondCornerOffset      = 1ULL;
        static constexpr std::size_t k_thirdCornerOffset       = 2ULL;
        static constexpr std::size_t k_neighborOffsetListExtra = 1ULL;
        static constexpr std::size_t k_maxSearchVertexCount    = 4096ULL;
        static constexpr std::size_t k_singleLayerVertexCount  = 1ULL;

        static constexpr float k_lodSurfaceDeviationPercentile = 0.95F;

        static constexpr float k_barycentricZero           = 0.0F;
        static constexpr float k_barycentricOne            = 1.0F;
        static constexpr float k_minBarycentricDenominator = 1.0e-20F;

        static constexpr std::uint32_t k_initialVertexTriangleCount = 0U;
        static constexpr std::uint32_t k_visitedStampIncrement      = 1U;
    };
}