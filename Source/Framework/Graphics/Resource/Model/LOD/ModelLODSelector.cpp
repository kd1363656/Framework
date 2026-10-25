#include "ModelLODSelector.h"

void FWK::Graphics::ModelLODSelector::Setup(const Struct::CBCameraPass& a_cbCameraPass, const float a_screenHeight, const float a_maxScreenSpaceErrorPixel)
{
    m_cameraWorldPosition      = a_cbCameraPass.m_cameraWorldPosition;
    m_maxScreenSpaceErrorPixel = a_maxScreenSpaceErrorPixel;
    m_nearClip                 = a_cbCameraPass.m_nearClip;

    // 距離dの位置で画面の縦全体に映る長さは「2 × d × tan(縦の視野角 ÷ 2)」
    // つまり、長さ1は画面上で「画面の高さ ÷ (2 × d × tan(縦の視野角 ÷ 2))」ピクセルになる
    // dで割る部分だけを毎回計算すればよいように、残りをここで計算しておく
    const float l_fullHeightTangent = a_cbCameraPass.m_tanHalfFOVY * k_tanHalfFOVToFullHeightScale;

    m_projectionScale = l_fullHeightTangent > k_initialProjectionScale ? a_screenHeight / l_fullHeightTangent : k_initialProjectionScale;
}

const FWK::Struct::ModelMeshLOD* FWK::Graphics::ModelLODSelector::SelectPTRModelMeshLOD(const Struct::ModelMeshletData& a_modelMeshletData, const TypeAlias::Math::Matrix& a_worldMatrix, const float a_worldMaxScale) const
{
    const auto& l_lodList = a_modelMeshletData.m_lodList;

    // LODが作られていないMeshは、呼び出し側でMeshlet全体を描画する
    if (l_lodList.empty()) { return nullptr; }

    const float l_distance = CalculateModelMeshDistance(a_modelMeshletData, a_worldMatrix, a_worldMaxScale);

    // 三角形が一番少ないLODから順に調べ、画面上の誤差が許容値以下の最初のLODを使う
    for (auto l_itr = l_lodList.rbegin(); l_itr != l_lodList.rend(); ++l_itr)
    {
        const float l_worldSimplifyError    = l_itr->m_simplifyError * a_worldMaxScale;
        const float l_screenSpaceErrorPixel = l_worldSimplifyError * m_projectionScale / l_distance;

        if (l_screenSpaceErrorPixel <= m_maxScreenSpaceErrorPixel) { return &(*l_itr); }
    }

    // どのLODも許容値を超える場合(カメラのすぐ近く)は、元の形を使う
    return &l_lodList.front();
}

float FWK::Graphics::ModelLODSelector::CalculateCoveredLayerDepthOffset(const Struct::ModelMeshletData& a_modelMeshletData, const TypeAlias::Math::Matrix& a_worldMatrix, const float a_worldMaxScale) const
{
    if (m_projectionScale <= k_initialProjectionScale) { return Constant::k_initialCBModelPerObjectLODCoveredLayerDepthOffset; }

    // 画面上の誤差(ピクセル) = ワールド座標の長さ × m_projectionScale ÷ 距離 を、長さについて解く
    const float l_distance = CalculateModelMeshDistance(a_modelMeshletData, a_worldMatrix, a_worldMaxScale);

    return a_modelMeshletData.m_lodSurfaceDeviationScale * m_maxScreenSpaceErrorPixel * l_distance / m_projectionScale;
}

float FWK::Graphics::ModelLODSelector::FetchVALMinDistance() const
{
    return std::max(m_nearClip, k_minDistance);
}

float FWK::Graphics::ModelLODSelector::CalculateModelMeshDistance(const Struct::ModelMeshletData& a_modelMeshletData, const TypeAlias::Math::Matrix& a_worldMatrix, const float a_worldMaxScale) const
{
    // Meshを囲む球をワールド座標へ変換する
    const auto  l_worldBoundsCenter = TypeAlias::Math::Vector3::Transform(a_modelMeshletData.m_boundsCenter, a_worldMatrix);
    const float l_worldBoundsRadius = a_modelMeshletData.m_boundsRadius * a_worldMaxScale;

    // カメラから「球の表面」までの距離(一番近い部分の距離)を使う
    // 球の中心までの距離を使うと、大きなMeshの手前側が粗いLODになってしまうため
    const float l_centerDistance = TypeAlias::Math::Vector3::Distance(m_cameraWorldPosition, l_worldBoundsCenter);

    return std::max(l_centerDistance - l_worldBoundsRadius, FetchVALMinDistance());
}