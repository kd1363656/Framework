#pragma once

namespace FWK::Enum
{
    // ビューポートのギズモ(ImGuizmo)で行う操作の種類
    enum class ViewportGizmoOperation
    {
        Translate,
        Rotate,
        Scale,
    };

    FWK_JSON_SERIALIZE_ENUM
    (
        ViewportGizmoOperation,
        FWK_JSON_ENUM_VALUE(ViewportGizmoOperation::Translate),
        FWK_JSON_ENUM_VALUE(ViewportGizmoOperation::Rotate),
        FWK_JSON_ENUM_VALUE(ViewportGizmoOperation::Scale)
    )

    // そのフレームでRendererがどのカメラから描画したか(ビューポートのプレビュー用のコピー先の番号にも使う)
    enum class ViewportRenderViewKind
    {
        // メインカメラ(CameraComponentがRendererへ登録したカメラ)
        Main,

        // エディターのデバッグカメラ
        DebugCamera,

        // 選択中のGameObjectが持つCameraComponentのカメラ(プレビュー)
        CameraPreview,

        Count,
    };
}
