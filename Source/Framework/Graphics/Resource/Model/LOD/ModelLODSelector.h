#pragma once

namespace FWK::Graphics
{
    // カメラからの距離と画面の大きさから、Meshごとに使うLODを選ぶクラス
    //
    // 仕組み :
    // LODを作るときに「元の形から最大で何メートル形が崩れたか(簡略化誤差)」を記録している。
    // 遠くにある物ほど画面上で小さく映るため、同じ誤差でも画面上では短い長さになる。
    //
    //   画面上の誤差(ピクセル) = 簡略化誤差 × 画面の高さ(ピクセル) ÷ (2 × 距離 × tan(縦の視野角 ÷ 2))
    //
    // この値が1ピクセル程度以下なら、人の目にはほぼ違いが分からない。
    // そこで「画面上の誤差が許容値以下になる、最も三角形の少ないLOD」を選ぶ
    class ModelLODSelector final
    {
    public:

         ModelLODSelector() = default;
        ~ModelLODSelector() = default;

        void Setup(const Struct::CBCameraPass& a_cbCameraPass, const float a_screenHeight, const float a_maxScreenSpaceErrorPixel);

        const Struct::ModelMeshLOD* SelectPTRModelMeshLOD(const Struct::ModelMeshletData& a_modelMeshletData, const TypeAlias::Math::Matrix& a_worldMatrix, const float a_worldMaxScale) const;

        // 他のMeshに覆われている頂点を、覆われている層1つ分だけカメラから見て奥へ押し出す距離(ワールド座標。0以上)を求める
        // LODの許容誤差(画面上でm_maxScreenSpaceErrorPixelピクセル)を、このMeshの位置での長さに直し、実際のずれの倍率を掛けた値
        // (どのLODが選ばれても、表面のずれはこの長さ以下になるので、覆う側が沈んでも覆われる側はその奥になる)
        float CalculateCoveredLayerDepthOffset(const Struct::ModelMeshletData& a_modelMeshletData, const TypeAlias::Math::Matrix& a_worldMatrix, const float a_worldMaxScale) const;

        // 距離の最小値(カメラのすぐ近くで0除算にならないようにする)
        float FetchVALMinDistance() const;

        // GPUでLODを選ぶ場合に、同じ計算をするための値
        const auto& GetREFCameraWorldPosition() const { return m_cameraWorldPosition; }

        float GetVALProjectionScale         () const { return m_projectionScale; }
        float GetVALMaxScreenSpaceErrorPixel() const { return m_maxScreenSpaceErrorPixel; }

    private:

        // カメラからMeshを囲む球の表面までの距離(LODの選び方と、沈める量の計算で同じ距離を使う)
        float CalculateModelMeshDistance(const Struct::ModelMeshletData& a_modelMeshletData, const TypeAlias::Math::Matrix& a_worldMatrix, const float a_worldMaxScale) const;

        static constexpr float k_initialProjectionScale       = 0.0F;
        static constexpr float k_initialNearClip              = 0.0F;
        static constexpr float k_tanHalfFOVToFullHeightScale  = 2.0F;
        static constexpr float k_minDistance                  = 0.0001F;

        TypeAlias::Math::Vector3 m_cameraWorldPosition = TypeAlias::Math::Vector3::Zero;

        // 「距離1の位置で、ローカル座標の長さ1が画面上で何ピクセルになるか」
        // 画面上の誤差 = 簡略化誤差 × m_projectionScale ÷ 距離 で求められるように、事前に計算しておく
        float m_projectionScale          = k_initialProjectionScale;
        float m_maxScreenSpaceErrorPixel = Constant::k_defaultMaxScreenSpaceErrorPixel;
        float m_nearClip                 = k_initialNearClip;
    };
}