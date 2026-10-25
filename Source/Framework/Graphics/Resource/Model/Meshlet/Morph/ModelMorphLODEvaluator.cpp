#include "ModelMorphLODEvaluator.h"

bool FWK::Graphics::ModelMorphLODEvaluator::Build(const Struct::ModelMorphData& a_modelMorphData, Struct::ModelMorphLODData& a_modelMorphLODData) const
{
    a_modelMorphLODData = {};

    FWK_ASSERT_RETURN_VALUE_IF(a_modelMorphData.m_targetList.empty(), "モーフターゲットが無いため、LOD用のモーフの情報を作れません。", false);

    BuildVertexLockList(a_modelMorphData, a_modelMorphLODData);

    return true;
}

float FWK::Graphics::ModelMorphLODEvaluator::CalculateLODMorphError(const Struct::ModelMorphData&            a_modelMorphData,
                                                                    const Struct::ModelLODSurfaceSearchData& a_surfaceSearchData,
                                                                    const std::vector<std::uint32_t>&        a_lodIndexList,
                                                                          std::vector<float>&                a_targetMorphErrorList) const
{
    const auto l_vertexCount = static_cast<std::uint32_t>(a_modelMorphData.m_vertexRangeList.size());
    const auto l_targetCount = a_modelMorphData.m_targetList.size();

    a_targetMorphErrorList.assign(l_targetCount, k_emptyMorphError);

    FWK_ASSERT_RETURN_VALUE_IF(a_surfaceSearchData.m_vertexPositionList.size() != l_vertexCount, "頂点の位置の数がモーフの頂点数と一致しません。", k_emptyMorphError);

    LODWorkData l_lodWorkData = {};

    m_lodSurfaceSearcher.BuildWorkData(a_lodIndexList, l_vertexCount, l_lodWorkData.m_surfaceSearchWorkData);

    l_lodWorkData.m_targetDeltaSumList.assign (l_targetCount, TypeAlias::Math::Vector3::Zero);
    l_lodWorkData.m_isTouchedTargetList.assign(l_targetCount, false);

    for (std::uint32_t l_vertexIndex = 0U; l_vertexIndex < l_vertexCount; ++l_vertexIndex)
    {
        // 消えた頂点自身が動かなくても、描かれる三角形の角が動けば崩れるため、消えた頂点は全て調べる
        if (l_lodWorkData.m_surfaceSearchWorkData.m_isKeptVertexList[l_vertexIndex]) { continue; }

        const auto& l_lodSurfacePoint = m_lodSurfaceSearcher.FindLODSurfacePoint(a_surfaceSearchData,
                                                                                 a_lodIndexList,
                                                                                 l_vertexIndex,
                                                                                 l_lodWorkData.m_surfaceSearchWorkData);

        // つながった部品ごと消えている頂点は、崩れではなく部品が見えなくなるだけなので数えない
        if (l_lodSurfacePoint.m_triangleBegin == Constant::k_invalidModelLODSurfaceTriangleBegin) { continue; }

        AccumulateInterpolatedDeltaDifference(a_modelMorphData,
                                              a_lodIndexList,
                                              l_vertexIndex,
                                              l_lodSurfacePoint,
                                              l_lodWorkData,
                                              a_targetMorphErrorList);
    }

    if (a_targetMorphErrorList.empty()) { return k_emptyMorphError; }

    return std::ranges::max(a_targetMorphErrorList);
}

void FWK::Graphics::ModelMorphLODEvaluator::BuildVertexLockList(const Struct::ModelMorphData& a_modelMorphData, Struct::ModelMorphLODData& a_modelMorphLODData) const
{
    const auto& l_targetList      = a_modelMorphData.m_targetList;
    const auto& l_vertexRangeList = a_modelMorphData.m_vertexRangeList;
    const auto& l_vertexDeltaList = a_modelMorphData.m_vertexDeltaList;

    auto& l_vertexLockList = a_modelMorphLODData.m_vertexLockList;

    l_vertexLockList.assign(l_vertexRangeList.size(), k_emptyVertexLock);

    for (std::size_t l_vertexIndex = 0ULL; l_vertexIndex < l_vertexRangeList.size(); ++l_vertexIndex)
    {
        const auto& l_vertexRange = l_vertexRangeList[l_vertexIndex];

        for (std::uint32_t l_deltaIndex = l_vertexRange.m_deltaOffset; l_deltaIndex < l_vertexRange.m_deltaOffset + l_vertexRange.m_deltaCount; ++l_deltaIndex)
        {
            const auto& l_vertexDelta = l_vertexDeltaList[l_deltaIndex];

            const bool l_isLockTarget = l_targetList[l_vertexDelta.m_targetIndex].m_lodProtection == Enum::ModelMorphLODProtection::Lock;

            // LOD保護が「固定」のターゲットで目に見えて動く頂点は、簡略化で動かさない
            if (l_isLockTarget &&
                l_vertexDelta.m_deltaPosition.Length() > k_lockTargetMINDelta)
            {
                l_vertexLockList[l_vertexIndex] = static_cast<unsigned char>(meshopt_SimplifyVertex_Lock);

                a_modelMorphLODData.m_hasLockVertex = true;

                break;
            }
        }
    }
}
void FWK::Graphics::ModelMorphLODEvaluator::AccumulateInterpolatedDeltaDifference(const Struct::ModelMorphData&       a_modelMorphData,
                                                                                   const std::vector<std::uint32_t>&   a_lodIndexList,
                                                                                   const std::uint32_t                 a_removedVertexIndex,
                                                                                   const Struct::ModelLODSurfacePoint& a_lodSurfacePoint,
                                                                                         LODWorkData&                  a_lodWorkData,
                                                                                         std::vector<float>&           a_targetMorphErrorList) const
{
    const auto& l_targetList  = a_modelMorphData.m_targetList;
    const auto& l_barycentric = a_lodSurfacePoint.m_barycentric;

    const std::size_t l_triangleBegin = a_lodSurfacePoint.m_triangleBegin;

    // ターゲットごとに「消えた頂点の移動量 - Σ(角の移動量 × 重心座標)」を合計する
    AddScaledVertexDelta(a_modelMorphData, a_removedVertexIndex,                                        k_removedVertexDeltaScale, a_lodWorkData);
    AddScaledVertexDelta(a_modelMorphData, a_lodIndexList[l_triangleBegin],                             -l_barycentric.x,          a_lodWorkData);
    AddScaledVertexDelta(a_modelMorphData, a_lodIndexList[l_triangleBegin + k_secondCornerOffset],      -l_barycentric.y,          a_lodWorkData);
    AddScaledVertexDelta(a_modelMorphData, a_lodIndexList[l_triangleBegin + k_thirdCornerOffset],       -l_barycentric.z,          a_lodWorkData);

    // 足したターゲットだけ長さを崩れにし、次の頂点のために0へ戻す
    for (const auto l_targetIndex : a_lodWorkData.m_touchedTargetList)
    {
        auto& l_targetDeltaSum = a_lodWorkData.m_targetDeltaSumList[l_targetIndex];

        const float l_targetScale = FetchVALTargetScale(l_targetList[l_targetIndex].m_lodProtection);

        a_targetMorphErrorList[l_targetIndex] = std::max(a_targetMorphErrorList[l_targetIndex], l_targetDeltaSum.Length() * l_targetScale);

        l_targetDeltaSum                                   = TypeAlias::Math::Vector3::Zero;
        a_lodWorkData.m_isTouchedTargetList[l_targetIndex] = false;
    }

    a_lodWorkData.m_touchedTargetList.clear();
}

void FWK::Graphics::ModelMorphLODEvaluator::AddScaledVertexDelta(const Struct::ModelMorphData& a_modelMorphData, const std::uint32_t a_vertexIndex, const float a_scale, LODWorkData& a_lodWorkData) const
{
    const auto& l_vertexDeltaList = a_modelMorphData.m_vertexDeltaList;
    const auto& l_vertexRange     = a_modelMorphData.m_vertexRangeList[a_vertexIndex];

    for (std::uint32_t l_deltaIndex = l_vertexRange.m_deltaOffset; l_deltaIndex < l_vertexRange.m_deltaOffset + l_vertexRange.m_deltaCount; ++l_deltaIndex)
    {
        const auto& l_vertexDelta = l_vertexDeltaList[l_deltaIndex];

        const auto l_targetIndex = l_vertexDelta.m_targetIndex;

        if (!a_lodWorkData.m_isTouchedTargetList[l_targetIndex])
        {
            a_lodWorkData.m_isTouchedTargetList[l_targetIndex] = true;

            a_lodWorkData.m_touchedTargetList.emplace_back(l_targetIndex);
        }

        a_lodWorkData.m_targetDeltaSumList[l_targetIndex] += l_vertexDelta.m_deltaPosition * a_scale;
    }
}

float FWK::Graphics::ModelMorphLODEvaluator::FetchVALTargetScale(const Enum::ModelMorphLODProtection a_lodProtection) const
{
    if (a_lodProtection == Enum::ModelMorphLODProtection::Important) { return k_importantTargetScale; }

    return k_normalTargetScale;
}