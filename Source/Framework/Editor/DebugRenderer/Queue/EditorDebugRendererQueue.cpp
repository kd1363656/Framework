#include "EditorDebugRendererQueue.h"

void FWK::Editor::EditorDebugRendererQueue::ClearFrame()
{
    // 前のフレームで集めた線を、すべて消す
    // 毎フレーム集め直すため、描画が終わったものを残さない
    m_lineVertexList.clear();
}

void FWK::Editor::EditorDebugRendererQueue::AddLine(const TypeAlias::Math::Vector3& a_startPosition, const TypeAlias::Math::Vector3& a_endPosition, const TypeAlias::Math::Color& a_color)
{
    // 線は、始点と終点の2つの頂点で表す
    // LINELISTで描画するため、2頂点ずつが1本の線になる
    m_lineVertexList.emplace_back(a_startPosition, a_color);
    m_lineVertexList.emplace_back(a_endPosition,   a_color);
}

void FWK::Editor::EditorDebugRendererQueue::AddFrustum(const TypeAlias::Math::Matrix& a_worldMatrix,
                                                       const TypeAlias::Math::Color&  a_color,
                                                       const float                    a_tanHalfFOVX,
                                                       const float                    a_tanHalfFOVY,
                                                       const float                    a_nearDistance,
                                                       const float                    a_farDistance)
{
    // カメラの視錐台(カメラに映る範囲の四角錐台)を、12本の線で描く
    // 手前の面(near)と奥の面(far)の4つずつの角を、カメラのローカル空間で作り、
    // ワールド行列でワールド空間へ変換してから、辺を線として追加する
    // カメラのローカル空間 : 前がZ+、右がX+、上がY+
    // カメラから距離dだけ離れた面の、中心から右端・上端までの長さは、
    // d * tan(視野角の半分)で求まる(tanHalfFOVX/Yは、あらかじめ計算済みの値)
    const auto l_nearHalfWidth  = a_nearDistance * a_tanHalfFOVX;
    const auto l_nearHalfHeight = a_nearDistance * a_tanHalfFOVY;
    const auto l_farHalfWidth   = a_farDistance  * a_tanHalfFOVX;
    const auto l_farHalfHeight  = a_farDistance  * a_tanHalfFOVY;

    // 面の4つの角(左下 → 右下 → 右上 → 左上の順)
    // この順に並べると、隣り合う角同士を結ぶだけで面の枠になる
    const std::array<TypeAlias::Math::Vector3, k_planeCornerCount> l_nearLocalCornerList =
    {
        TypeAlias::Math::Vector3{ -l_nearHalfWidth, -l_nearHalfHeight, a_nearDistance },
        TypeAlias::Math::Vector3{  l_nearHalfWidth, -l_nearHalfHeight, a_nearDistance },
        TypeAlias::Math::Vector3{  l_nearHalfWidth,  l_nearHalfHeight, a_nearDistance },
        TypeAlias::Math::Vector3{ -l_nearHalfWidth,  l_nearHalfHeight, a_nearDistance }
    };

    const std::array<TypeAlias::Math::Vector3, k_planeCornerCount> l_farLocalCornerList =
    {
        TypeAlias::Math::Vector3{ -l_farHalfWidth, -l_farHalfHeight, a_farDistance },
        TypeAlias::Math::Vector3{  l_farHalfWidth, -l_farHalfHeight, a_farDistance },
        TypeAlias::Math::Vector3{  l_farHalfWidth,  l_farHalfHeight, a_farDistance },
        TypeAlias::Math::Vector3{ -l_farHalfWidth,  l_farHalfHeight, a_farDistance }
    };

    // ローカル空間の角を、ワールド空間へ変換する
    std::array<TypeAlias::Math::Vector3, k_planeCornerCount> l_nearWorldCornerList = {};
    std::array<TypeAlias::Math::Vector3, k_planeCornerCount> l_farWorldCornerList  = {};

    for (std::size_t l_cornerIndex = 0ULL; l_cornerIndex < k_planeCornerCount; ++l_cornerIndex)
    {
        l_nearWorldCornerList[l_cornerIndex] = TypeAlias::Math::Vector3::Transform(l_nearLocalCornerList[l_cornerIndex], a_worldMatrix);
        l_farWorldCornerList [l_cornerIndex] = TypeAlias::Math::Vector3::Transform(l_farLocalCornerList [l_cornerIndex], a_worldMatrix);
    }

    // 12本の辺を追加する(手前の面の4本、奥の面の4本、手前と奥を結ぶ4本)
    for (std::size_t l_cornerIndex = 0ULL; l_cornerIndex < k_planeCornerCount; ++l_cornerIndex)
    {
        // 最後の角(左上)の次は、最初の角(左下)に戻して、面の枠を閉じる
        const auto l_nextCornerIndex = (l_cornerIndex + k_nextCornerOffset) % k_planeCornerCount;

        AddLine(l_nearWorldCornerList[l_cornerIndex], l_nearWorldCornerList[l_nextCornerIndex], a_color);
        AddLine(l_farWorldCornerList [l_cornerIndex], l_farWorldCornerList [l_nextCornerIndex], a_color);
        AddLine(l_nearWorldCornerList[l_cornerIndex], l_farWorldCornerList [l_cornerIndex],     a_color);
    }
}