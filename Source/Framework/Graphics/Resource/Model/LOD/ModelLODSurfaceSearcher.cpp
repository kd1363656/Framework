#include "ModelLODSurfaceSearcher.h"

void FWK::Graphics::ModelLODSurfaceSearcher::BuildSearchData(const std::vector<std::uint32_t>&            a_indexList,
                                                             const std::vector<TypeAlias::Math::Vector3>& a_vertexPositionList,
                                                             const std::vector<TypeAlias::Math::Vector3>& a_vertexNormalList,
                                                                   Struct::ModelLODSurfaceSearchData&     a_searchData) const
{
    const auto l_vertexCount = a_vertexPositionList.size();

    a_searchData = {};

    // 三角形の3辺から、頂点ごとの隣の頂点の一覧を作る(CSR : 頂点ごとに「何番目から何個か」で引ける形)
    std::vector<std::vector<std::uint32_t>> l_neighborListList = {};

    l_neighborListList.resize(l_vertexCount);

    for (std::size_t l_triangleBegin = 0ULL; l_triangleBegin + k_triangleCornerCount <= a_indexList.size(); l_triangleBegin += k_triangleCornerCount)
    {
        for (std::size_t l_cornerIndex = 0ULL; l_cornerIndex < k_triangleCornerCount; ++l_cornerIndex)
        {
            const auto l_firstVertexIndex  = a_indexList[l_triangleBegin + l_cornerIndex];
            const auto l_secondVertexIndex = a_indexList[l_triangleBegin + (l_cornerIndex + k_nextCornerOffset) % k_triangleCornerCount];

            if (l_firstVertexIndex  >= l_vertexCount ||
                l_secondVertexIndex >= l_vertexCount)
            {
                continue;
            }

            l_neighborListList[l_firstVertexIndex].emplace_back (l_secondVertexIndex);
            l_neighborListList[l_secondVertexIndex].emplace_back(l_firstVertexIndex);
        }
    }

    auto& l_vertexNeighborOffsetList = a_searchData.m_vertexNeighborOffsetList;
    auto& l_vertexNeighborList       = a_searchData.m_vertexNeighborList;

    l_vertexNeighborOffsetList.reserve(l_vertexCount + k_neighborOffsetListExtra);

    for (auto& l_neighborList : l_neighborListList)
    {
        // 隣り合う三角形が同じ辺を持つため、重複を取り除く
        std::ranges::sort(l_neighborList);

        const auto& l_uniqueEnd = std::ranges::unique(l_neighborList);

        l_neighborList.erase(l_uniqueEnd.begin(), l_uniqueEnd.end());

        l_vertexNeighborOffsetList.emplace_back(static_cast<std::uint32_t>(l_vertexNeighborList.size()));
        l_vertexNeighborList.insert            (l_vertexNeighborList.end(), l_neighborList.begin(), l_neighborList.end());
    }

    l_vertexNeighborOffsetList.emplace_back(static_cast<std::uint32_t>(l_vertexNeighborList.size()));

    a_searchData.m_vertexPositionList = a_vertexPositionList;
    a_searchData.m_vertexNormalList   = a_vertexNormalList;
}

void FWK::Graphics::ModelLODSurfaceSearcher::BuildWorkData(const std::vector<std::uint32_t>& a_lodIndexList, const std::size_t a_vertexCount, Struct::ModelLODSurfaceSearchWorkData& a_workData) const
{
    // LODの三角形から参照されている頂点 = 残った頂点
    a_workData.m_isKeptVertexList.assign(a_vertexCount, false);
    a_workData.m_visitedStampList.assign(a_vertexCount, Constant::k_initialModelLODSurfaceVisitedStamp);

    a_workData.m_currentVisitedStamp = Constant::k_initialModelLODSurfaceVisitedStamp;

    // 頂点ごとの「その頂点を角に持つLODの三角形」の一覧(CSR)
    // 1回目に頂点ごとの三角形の数を数え、2回目にその位置へ三角形の先頭のインデックス番号を書く
    auto& l_vertexTriangleOffsetList = a_workData.m_vertexTriangleOffsetList;
    auto& l_vertexTriangleList       = a_workData.m_vertexTriangleList;

    l_vertexTriangleOffsetList.assign(a_vertexCount + k_neighborOffsetListExtra, k_initialVertexTriangleCount);

    const std::size_t l_lodIndexCount = a_lodIndexList.size() - a_lodIndexList.size() % k_triangleCornerCount;

    for (std::size_t l_indexIndex = 0ULL; l_indexIndex < l_lodIndexCount; ++l_indexIndex)
    {
        const auto l_vertexIndex = a_lodIndexList[l_indexIndex];

        if (l_vertexIndex >= a_vertexCount) { continue; }

        a_workData.m_isKeptVertexList[l_vertexIndex] = true;

        ++l_vertexTriangleOffsetList[l_vertexIndex + k_neighborOffsetListExtra];
    }

    for (std::size_t l_vertexIndex = 0ULL; l_vertexIndex < a_vertexCount; ++l_vertexIndex)
    {
        l_vertexTriangleOffsetList[l_vertexIndex + k_neighborOffsetListExtra] += l_vertexTriangleOffsetList[l_vertexIndex];
    }

    l_vertexTriangleList.assign(l_vertexTriangleOffsetList.back(), k_initialVertexTriangleCount);

    // 書き込み位置(頂点ごとの先頭から、書いた数だけ進める)
    std::vector<std::uint32_t> l_writeOffsetList(l_vertexTriangleOffsetList.begin(), l_vertexTriangleOffsetList.end() - k_neighborOffsetListExtra);

    for (std::size_t l_indexIndex = 0ULL; l_indexIndex < l_lodIndexCount; ++l_indexIndex)
    {
        const auto l_vertexIndex = a_lodIndexList[l_indexIndex];

        if (l_vertexIndex >= a_vertexCount) { continue; }

        l_vertexTriangleList[l_writeOffsetList[l_vertexIndex]++] = static_cast<std::uint32_t>(l_indexIndex - l_indexIndex % k_triangleCornerCount);
    }
}

FWK::Struct::ModelLODSurfacePoint FWK::Graphics::ModelLODSurfaceSearcher::FindLODSurfacePoint(const Struct::ModelLODSurfaceSearchData&     a_searchData,
                                                                                              const std::vector<std::uint32_t>&            a_lodIndexList,
                                                                                              const std::uint32_t                          a_removedVertexIndex,
                                                                                                    Struct::ModelLODSurfaceSearchWorkData& a_workData) const
{
    CollectNearestKeptVertexList(a_searchData, a_removedVertexIndex, a_workData);

    // 残った頂点にたどり着けない頂点は、つながった部品ごと消えている(崩れではなく、部品が見えなくなるだけ)
    if (a_workData.m_keptVertexList.empty()) { return {}; }

    return FindNearestLODSurfacePoint(a_searchData,
                                      a_lodIndexList,
                                      a_removedVertexIndex,
                                      a_workData);
}

FWK::Struct::ModelLODSurfaceDeviation FWK::Graphics::ModelLODSurfaceSearcher::CalculateLODSurfaceDeviation(const Struct::ModelLODSurfaceSearchData& a_searchData, const std::vector<std::uint32_t>& a_lodIndexList) const
{
    const auto& l_vertexPositionList = a_searchData.m_vertexPositionList;
    const auto& l_vertexNormalList   = a_searchData.m_vertexNormalList;

    const auto l_vertexCount = static_cast<std::uint32_t>(l_vertexPositionList.size());

    Struct::ModelLODSurfaceDeviation      l_lodSurfaceDeviation = {};
    Struct::ModelLODSurfaceSearchWorkData l_workData            = {};

    BuildWorkData(a_lodIndexList, l_vertexCount, l_workData);

    std::vector<float> l_sinkDistanceList  = {};
    std::vector<float> l_bulgeDistanceList = {};

    for (std::uint32_t l_vertexIndex = 0U; l_vertexIndex < l_vertexCount; ++l_vertexIndex)
    {
        // 残った頂点は元の位置のまま描かれるので、ずれは無い
        if (l_workData.m_isKeptVertexList[l_vertexIndex]) { continue; }

        const auto& l_lodSurfacePoint = FindLODSurfacePoint(a_searchData,
                                                            a_lodIndexList,
                                                            l_vertexIndex,
                                                            l_workData);

        if (l_lodSurfacePoint.m_triangleBegin == Constant::k_invalidModelLODSurfaceTriangleBegin) { continue; }

        const auto& l_barycentric = l_lodSurfacePoint.m_barycentric;

        const auto& l_cornerA = l_vertexPositionList[a_lodIndexList[l_lodSurfacePoint.m_triangleBegin]];
        const auto& l_cornerB = l_vertexPositionList[a_lodIndexList[l_lodSurfacePoint.m_triangleBegin + k_secondCornerOffset]];
        const auto& l_cornerC = l_vertexPositionList[a_lodIndexList[l_lodSurfacePoint.m_triangleBegin + k_thirdCornerOffset]];

        // 消えた頂点が描かれる点
        const auto& l_lodSurfacePosition = l_cornerA * l_barycentric.x + l_cornerB * l_barycentric.y + l_cornerC * l_barycentric.z;

        // 描かれる点が、元の位置から法線の向きへどれだけ離れているか(正なら外側へ出ている、負なら内側へ沈んでいる)
        const float l_normalDistance = (l_lodSurfacePosition - l_vertexPositionList[l_vertexIndex]).Dot(l_vertexNormalList[l_vertexIndex]);

        if (l_normalDistance < Constant::k_initialModelLODSurfaceDeviationDistance) { l_sinkDistanceList.emplace_back (-l_normalDistance); }
        else                                                                        { l_bulgeDistanceList.emplace_back( l_normalDistance); }
    }

    l_lodSurfaceDeviation.m_sinkDistance  = CalculatePercentileDistance(l_sinkDistanceList,  k_lodSurfaceDeviationPercentile);
    l_lodSurfaceDeviation.m_bulgeDistance = CalculatePercentileDistance(l_bulgeDistanceList, k_lodSurfaceDeviationPercentile);

    return l_lodSurfaceDeviation;
}

float FWK::Graphics::ModelLODSurfaceSearcher::CalculatePercentileDistance(std::vector<float>& a_distanceList, const float a_percentile) const
{
    if (a_distanceList.empty()) { return Constant::k_initialModelLODSurfaceDeviationDistance; }

    // std::ranges::nth_element(配列、位置) : 全体を並べ替えずに、その位置に「小さい順に並べたときの値」だけを置く
    const auto l_percentileIndex = static_cast<std::size_t>(static_cast<float>(a_distanceList.size() - k_singleLayerVertexCount) * a_percentile);

    std::ranges::nth_element(a_distanceList, a_distanceList.begin() + static_cast<std::ptrdiff_t>(l_percentileIndex));

    return a_distanceList[l_percentileIndex];
}

void FWK::Graphics::ModelLODSurfaceSearcher::CollectNearestKeptVertexList(const Struct::ModelLODSurfaceSearchData& a_searchData, const std::uint32_t a_removedVertexIndex, Struct::ModelLODSurfaceSearchWorkData& a_workData) const
{
    const auto& l_vertexNeighborOffsetList = a_searchData.m_vertexNeighborOffsetList;
    const auto& l_vertexNeighborList       = a_searchData.m_vertexNeighborList;

    auto& l_currentLayerVertexList = a_workData.m_currentLayerVertexList;
    auto& l_nextLayerVertexList    = a_workData.m_nextLayerVertexList;
    auto& l_keptVertexList         = a_workData.m_keptVertexList;
    auto& l_visitedStampList       = a_workData.m_visitedStampList;

    // 頂点ごとに探索するため、「この探索で訪れたか」を番号で覚える(毎回0に戻さずに済む)
    a_workData.m_currentVisitedStamp += k_visitedStampIncrement;

    const auto l_visitedStamp = a_workData.m_currentVisitedStamp;

    // 幅優先探索(近い順に広げていく探索)で、消えた頂点から辺をたどる
    // 1層 = 辺1本分の距離。今の層の頂点と、次の層の頂点を分けて持つ
    l_currentLayerVertexList.assign(k_singleLayerVertexCount, a_removedVertexIndex);
    l_keptVertexList.clear();

    l_visitedStampList[a_removedVertexIndex] = l_visitedStamp;

    std::size_t l_visitedVertexCount = l_currentLayerVertexList.size();

    while (!l_currentLayerVertexList.empty())
    {
        // この層に残った頂点があれば、それが周りの残った頂点(消えた頂点はこれらを角に持つ三角形の上に描かれる)
        for (const auto l_vertexIndex : l_currentLayerVertexList)
        {
            if (a_workData.m_isKeptVertexList[l_vertexIndex]) { l_keptVertexList.emplace_back(l_vertexIndex); }
        }

        if (!l_keptVertexList.empty()) { return; }

        // 次の層(1つ外側の頂点)を集める
        l_nextLayerVertexList.clear();

        for (const auto l_vertexIndex : l_currentLayerVertexList)
        {
            for (auto l_neighborOffset = l_vertexNeighborOffsetList[l_vertexIndex]; l_neighborOffset < l_vertexNeighborOffsetList[l_vertexIndex + k_neighborOffsetListExtra]; ++l_neighborOffset)
            {
                const auto l_neighborVertexIndex = l_vertexNeighborList[l_neighborOffset];

                if (l_visitedStampList[l_neighborVertexIndex] == l_visitedStamp) { continue; }

                l_visitedStampList[l_neighborVertexIndex] = l_visitedStamp;

                l_nextLayerVertexList.emplace_back(l_neighborVertexIndex);
            }
        }

        l_visitedVertexCount += l_nextLayerVertexList.size();

        // とても遠くまで残った頂点が無い場合は、大きな部分が丸ごと消えているため探索を打ち切る
        if (l_visitedVertexCount > k_maxSearchVertexCount) { return; }

        std::swap(l_currentLayerVertexList, l_nextLayerVertexList);
    }
}

FWK::Struct::ModelLODSurfacePoint FWK::Graphics::ModelLODSurfaceSearcher::FindNearestLODSurfacePoint(const Struct::ModelLODSurfaceSearchData&     a_searchData,
                                                                                                     const std::vector<std::uint32_t>&            a_lodIndexList,
                                                                                                     const std::uint32_t                          a_removedVertexIndex,
                                                                                                     const Struct::ModelLODSurfaceSearchWorkData& a_workData) const
{
    const auto& l_vertexPositionList       = a_searchData.m_vertexPositionList;
    const auto& l_vertexTriangleOffsetList = a_workData.m_vertexTriangleOffsetList;
    const auto& l_vertexTriangleList       = a_workData.m_vertexTriangleList;

    const auto& l_removedVertexPosition = l_vertexPositionList[a_removedVertexIndex];

    Struct::ModelLODSurfacePoint l_nearestLODSurfacePoint = {};

    float l_nearestDistanceSquared = std::numeric_limits<float>::max();

    // 周りに残った頂点を角に持つ三角形だけを調べる(消えた頂点はその近くの三角形の上に描かれるため)
    // 同じ三角形を何回か調べることがあるが、結果は同じなので問題ない
    for (const auto l_keptVertexIndex : a_workData.m_keptVertexList)
    {
        for (auto l_triangleOffset = l_vertexTriangleOffsetList[l_keptVertexIndex]; l_triangleOffset < l_vertexTriangleOffsetList[l_keptVertexIndex + k_neighborOffsetListExtra]; ++l_triangleOffset)
        {
            const std::size_t l_triangleBegin = l_vertexTriangleList[l_triangleOffset];

            const auto& l_cornerA = l_vertexPositionList[a_lodIndexList[l_triangleBegin]];
            const auto& l_cornerB = l_vertexPositionList[a_lodIndexList[l_triangleBegin + k_secondCornerOffset]];
            const auto& l_cornerC = l_vertexPositionList[a_lodIndexList[l_triangleBegin + k_thirdCornerOffset]];

            const auto& l_barycentric = CalculateClosestPointBarycentric(l_removedVertexPosition,
                                                                         l_cornerA,
                                                                         l_cornerB,
                                                                         l_cornerC);

            const auto& l_closestPoint = l_cornerA * l_barycentric.x + l_cornerB * l_barycentric.y + l_cornerC * l_barycentric.z;

            const float l_distanceSquared = TypeAlias::Math::Vector3::DistanceSquared(l_removedVertexPosition, l_closestPoint);

            if (l_distanceSquared >= l_nearestDistanceSquared) { continue; }

            l_nearestDistanceSquared                 = l_distanceSquared;
            l_nearestLODSurfacePoint.m_triangleBegin = l_triangleBegin;
            l_nearestLODSurfacePoint.m_barycentric   = l_barycentric;
        }
    }

    return l_nearestLODSurfacePoint;
}

FWK::TypeAlias::Math::Vector3 FWK::Graphics::ModelLODSurfaceSearcher::CalculateClosestPointBarycentric(const TypeAlias::Math::Vector3& a_point,
                                                                                                       const TypeAlias::Math::Vector3& a_cornerA,
                                                                                                       const TypeAlias::Math::Vector3& a_cornerB,
                                                                                                       const TypeAlias::Math::Vector3& a_cornerC) const
{
    // 点に一番近い三角形の上の点は、「角」「辺」「面の内側」のどこかにある
    // 点が三角形のどの部分の外側にあるかを内積で調べ、当てはまる部分の上で一番近い点の重心座標を返す
    // (Christer Ericson「Real-Time Collision Detection」5.1.5 の方法)
    const auto& l_edgeAB = a_cornerB - a_cornerA;
    const auto& l_edgeAC = a_cornerC - a_cornerA;

    const auto& l_toPointFromA = a_point - a_cornerA;

    const float l_dotABA = l_edgeAB.Dot(l_toPointFromA);
    const float l_dotACA = l_edgeAC.Dot(l_toPointFromA);

    // 角Aの外側
    if (l_dotABA <= k_barycentricZero &&
        l_dotACA <= k_barycentricZero)
    {
        return { k_barycentricOne, k_barycentricZero, k_barycentricZero };
    }

    const auto& l_toPointFromB = a_point - a_cornerB;

    const float l_dotABB = l_edgeAB.Dot(l_toPointFromB);
    const float l_dotACB = l_edgeAC.Dot(l_toPointFromB);

    // 角Bの外側
    if (l_dotABB >= k_barycentricZero &&
        l_dotACB <= l_dotABB)
    {
        return { k_barycentricZero, k_barycentricOne, k_barycentricZero };
    }

    // 辺ABの外側
    const float l_areaC = l_dotABA * l_dotACB - l_dotABB * l_dotACA;

    if (l_areaC  <= k_barycentricZero &&
        l_dotABA >= k_barycentricZero &&
        l_dotABB <= k_barycentricZero)
    {
        const float l_ratio = l_dotABA / std::max(l_dotABA - l_dotABB, k_minBarycentricDenominator);

        return { k_barycentricOne - l_ratio, l_ratio, k_barycentricZero };
    }

    const auto& l_toPointFromC = a_point - a_cornerC;

    const float l_dotABC = l_edgeAB.Dot(l_toPointFromC);
    const float l_dotACC = l_edgeAC.Dot(l_toPointFromC);

    // 角Cの外側
    if (l_dotACC >= k_barycentricZero &&
        l_dotABC <= l_dotACC)
    {
        return { k_barycentricZero, k_barycentricZero, k_barycentricOne };
    }

    // 辺ACの外側
    const float l_areaB = l_dotABC * l_dotACA - l_dotABA * l_dotACC;

    if (l_areaB  <= k_barycentricZero &&
        l_dotACA >= k_barycentricZero &&
        l_dotACC <= k_barycentricZero)
    {
        const float l_ratio = l_dotACA / std::max(l_dotACA - l_dotACC, k_minBarycentricDenominator);

        return { k_barycentricOne - l_ratio, k_barycentricZero, l_ratio };
    }

    // 辺BCの外側
    const float l_areaA = l_dotABB * l_dotACC - l_dotABC * l_dotACB;

    if (l_areaA             <= k_barycentricZero &&
        l_dotACB - l_dotABB >= k_barycentricZero &&
        l_dotABC - l_dotACC >= k_barycentricZero)
    {
        const float l_ratio = (l_dotACB - l_dotABB) / std::max((l_dotACB - l_dotABB) + (l_dotABC - l_dotACC), k_minBarycentricDenominator);

        return { k_barycentricZero, k_barycentricOne - l_ratio, l_ratio };
    }

    // 面の内側
    const float l_areaSum = l_areaA + l_areaB + l_areaC;

    // 潰れた(面積が0の)三角形は、角Aを一番近い点とする
    if (l_areaSum <= k_minBarycentricDenominator) { return { k_barycentricOne, k_barycentricZero, k_barycentricZero }; }

    const float l_ratioB = l_areaB / l_areaSum;
    const float l_ratioC = l_areaC / l_areaSum;

    return { k_barycentricOne - l_ratioB - l_ratioC, l_ratioB, l_ratioC };
}