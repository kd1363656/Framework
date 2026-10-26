#pragma once

namespace FWK::Struct
{
    // ※ HLSL側(Shader/Camera/CameraPass.hlsli)と必ず同じ並びにする
    struct CBCameraPass final
    {
        TypeAlias::Math::Matrix m_viewMatrix           = TypeAlias::Math::Matrix::Identity;
        TypeAlias::Math::Matrix m_projectionMatrix     = TypeAlias::Math::Matrix::Identity;
        TypeAlias::Math::Matrix m_viewProjectionMatrix = TypeAlias::Math::Matrix::Identity;

        float m_nearClip    = Constant::k_cameraDefaultNearClip;
        float m_farClip     = Constant::k_cameraDefaultFarClip;
        float m_tanHalfFOVX = Constant::k_cameraDefaultTangentHalfFOVX;
        float m_tanHalfFOVY = Constant::k_cameraDefaultTangentHalfFOVY;

        TypeAlias::Math::Vector3 m_cameraWorldPosition = TypeAlias::Math::Vector3::Zero;

        float m_padding = 0.0F;

        // カリング(Meshlet単位の視錐台、裏面の判定と、影のカスケードの選択)に使うカメラの値
        // 通常は上の描画に使うカメラと同じ値が入り、エディターのデバッグカメラで描画している間だけメインカメラの値が入る
        // (CameraPassDrawRequestがGPUへ送る直前に書き込むため、Cameraクラスはこの値を触らない)
        TypeAlias::Math::Matrix m_cullingViewMatrix = TypeAlias::Math::Matrix::Identity;

        float m_cullingNearClip    = Constant::k_cameraDefaultNearClip;
        float m_cullingFarClip     = Constant::k_cameraDefaultFarClip;
        float m_cullingTanHalfFOVX = Constant::k_cameraDefaultTangentHalfFOVX;
        float m_cullingTanHalfFOVY = Constant::k_cameraDefaultTangentHalfFOVY;

        TypeAlias::Math::Vector3 m_cullingCameraWorldPosition = TypeAlias::Math::Vector3::Zero;

        // 1ならカリングで消された物も色を付けて描画する(デバッグカメラのカリング結果の可視化)
        std::uint32_t m_isCullingDebugVisualize = Constant::k_cameraPassCullingDebugVisualizeOff;
    };

    static_assert(sizeof(CBCameraPass) == Constant::k_cbCameraPassByteSize, "CBCameraPassのサイズがHLSL側と一致しません。");
}
