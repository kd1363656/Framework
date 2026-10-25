#include "ModelLODLayerDetector.h"

std::vector<std::uint32_t> FWK::Graphics::ModelLODLayerDetector::DetectCoveredLayerCountList(const std::vector<LayerVertex>& a_layerVertexList) const
{
    // 判定できない場合は、どの頂点も覆われていない(動かさない)として扱う
    std::vector<std::uint32_t> l_coveredLayerCountList(a_layerVertexList.size(), k_initialCoveredLayerCount);

    if (a_layerVertexList.size() == k_initialLayerVertexCount) { return l_coveredLayerCountList; }

    // 1. モデル全体を囲む箱の大きさから、近くを探す範囲(半径)を決める
    auto l_boundsMIN = TypeAlias::Math::Vector3( std::numeric_limits<float>::max());
    auto l_boundsMAX = TypeAlias::Math::Vector3(-std::numeric_limits<float>::max());

    for (const auto& l_layerVertex : a_layerVertexList)
    {
        l_boundsMIN = TypeAlias::Math::Vector3::Min(l_boundsMIN, l_layerVertex.m_position);
        l_boundsMAX = TypeAlias::Math::Vector3::Max(l_boundsMAX, l_layerVertex.m_position);
    }

    // 密着した服と肌の隙間は、モデルの大きさに比べてごく小さいので、対角線の1%の範囲だけを探す
    const float l_searchRadius = TypeAlias::Math::Vector3::Distance(l_boundsMIN, l_boundsMAX) * k_layerSearchRadiusScale;

    if (l_searchRadius <= std::numeric_limits<float>::epsilon()) { return l_coveredLayerCountList; }

    const float l_searchRadiusSquared = l_searchRadius * l_searchRadius;

    // 2. 空間を「探す範囲」と同じ大きさの立方体(セル)に区切り、頂点をセルの番号順に並べる
    //    近くの頂点は、自分のセルと周りの26個のセルの中にしか無いので、全頂点と比べずに済む
    //    (セルの番号順に並べておけば、二分探索(std::ranges::equal_range)で同じセルの頂点をまとめて取り出せる)
    std::vector<std::pair<std::uint64_t, std::uint32_t>> l_cellVertexList = {};

    l_cellVertexList.reserve(a_layerVertexList.size());

    for (std::uint32_t l_vertexIndex = 0U; l_vertexIndex < static_cast<std::uint32_t>(a_layerVertexList.size()); ++l_vertexIndex)
    {
        const auto& l_position = a_layerVertexList[l_vertexIndex].m_position;

        const auto l_cellKey = CalculateCellKey(CalculateCellCoordinate(l_position.x, l_searchRadius),
                                                CalculateCellCoordinate(l_position.y, l_searchRadius),
                                                CalculateCellCoordinate(l_position.z, l_searchRadius));

        l_cellVertexList.emplace_back(l_cellKey, l_vertexIndex);
    }

    std::ranges::sort(l_cellVertexList);

    // 3. 頂点ごとに、外側(法線の向き)の近くにある別のMeshを、Meshの種類ごとに1回だけ数える
    //    数えすぎても沈める量が増えるだけなので、描画で使う上限(Constant::k_maxLODCoveredLayerCount)で止める
    std::array<std::uint32_t, Constant::k_maxLODCoveredLayerCount> l_coveringMeshIndexList = {};

    for (std::size_t l_vertexIndex = 0ULL; l_vertexIndex < a_layerVertexList.size(); ++l_vertexIndex)
    {
        const auto& l_layerVertex = a_layerVertexList[l_vertexIndex];

        // 法線が無い頂点は、どちらが外側か決められない
        if (l_layerVertex.m_normal.LengthSquared() <= k_minNormalLengthSquared) { continue; }

        auto l_normal = l_layerVertex.m_normal;

        l_normal.Normalize();

        const auto l_cellX = CalculateCellCoordinate(l_layerVertex.m_position.x, l_searchRadius);
        const auto l_cellY = CalculateCellCoordinate(l_layerVertex.m_position.y, l_searchRadius);
        const auto l_cellZ = CalculateCellCoordinate(l_layerVertex.m_position.z, l_searchRadius);

        std::uint32_t l_coveredLayerCount = k_initialCoveredLayerCount;

        for (std::int64_t l_offsetZ = k_neighborCellOffsetMIN; l_offsetZ <= k_neighborCellOffsetMAX; ++l_offsetZ)
        {
            for (std::int64_t l_offsetY = k_neighborCellOffsetMIN; l_offsetY <= k_neighborCellOffsetMAX; ++l_offsetY)
            {
                for (std::int64_t l_offsetX = k_neighborCellOffsetMIN; l_offsetX <= k_neighborCellOffsetMAX; ++l_offsetX)
                {
                    const auto l_cellKey = CalculateCellKey(l_cellX + l_offsetX, l_cellY + l_offsetY, l_cellZ + l_offsetZ);

                    // std::ranges::equal_range(並べた配列、探す値、比べ方、比べる値の取り出し方) : 同じセルの頂点の範囲を求める
                    const auto& l_cellRange = std::ranges::equal_range(l_cellVertexList, l_cellKey, std::less<>{}, &std::pair<std::uint64_t, std::uint32_t>::first);

                    for (const auto& l_cellVertex : l_cellRange)
                    {
                        if (l_coveredLayerCount >= Constant::k_maxLODCoveredLayerCount) { break; }

                        const auto& l_otherLayerVertex = a_layerVertexList[l_cellVertex.second];

                        // 同じMeshの頂点は、重なりの判定に使わない
                        if (l_otherLayerVertex.m_meshIndex == l_layerVertex.m_meshIndex) { continue; }

                        const auto& l_toOtherVertex        = l_otherLayerVertex.m_position - l_layerVertex.m_position;
                        const float l_otherDistanceSquared = l_toOtherVertex.LengthSquared();

                        if (l_otherDistanceSquared > l_searchRadiusSquared) { continue; }

                        // 法線の向きへの距離(正なら外側にある)
                        // 横に並んでいるだけの頂点(服の端と肌の境目など)は数えないように、
                        // 法線の向きへの距離が、頂点同士の距離の半分以上ある物だけを外側とみなす
                        const float l_normalDistance = l_toOtherVertex.Dot(l_normal);

                        if (l_normalDistance < std::sqrt(l_otherDistanceSquared) * k_minNormalAlignment) { continue; }

                        // 既に数えたMeshなら数えない
                        const auto l_coveringMeshEnd = l_coveringMeshIndexList.begin() + l_coveredLayerCount;

                        if (std::find(l_coveringMeshIndexList.begin(), l_coveringMeshEnd, l_otherLayerVertex.m_meshIndex) != l_coveringMeshEnd) { continue; }

                        l_coveringMeshIndexList[l_coveredLayerCount] = l_otherLayerVertex.m_meshIndex;

                        ++l_coveredLayerCount;
                    }
                }
            }
        }

        l_coveredLayerCountList[l_vertexIndex] = l_coveredLayerCount;
    }

    return l_coveredLayerCountList;
}

std::uint64_t FWK::Graphics::ModelLODLayerDetector::CalculateCellKey(const std::int64_t a_cellX, const std::int64_t a_cellY, const std::int64_t a_cellZ) const
{
    // x、y、zのセル番号(負の値もあるので底上げしてから)を21bitずつ並べて、1つの64bitの番号にする
    const auto l_keyX = static_cast<std::uint64_t>(a_cellX + k_cellCoordinateBias) & k_cellCoordinateMask;
    const auto l_keyY = static_cast<std::uint64_t>(a_cellY + k_cellCoordinateBias) & k_cellCoordinateMask;
    const auto l_keyZ = static_cast<std::uint64_t>(a_cellZ + k_cellCoordinateBias) & k_cellCoordinateMask;

    return l_keyX | (l_keyY << k_cellKeyShiftBitY) | (l_keyZ << k_cellKeyShiftBitZ);
}
std::int64_t FWK::Graphics::ModelLODLayerDetector::CalculateCellCoordinate(const float a_position, const float a_cellSize) const
{
    return static_cast<std::int64_t>(std::floor(a_position / a_cellSize));
}