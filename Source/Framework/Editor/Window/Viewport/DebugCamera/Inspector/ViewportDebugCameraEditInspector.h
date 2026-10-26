#pragma once

namespace FWK::Editor
{
    class ViewportDebugCamera;
}

namespace FWK::Editor
{
    // デバッグカメラの設定を編集するImGuiの部品(ビューポート右上のカメラボタンから開くポップアップの中身)
    class ViewportDebugCameraEditInspector final
    {
    public:

         ViewportDebugCameraEditInspector() = default;
        ~ViewportDebugCameraEditInspector() = default;

        void EditInspector(ViewportDebugCamera& a_debugCamera) const;

    private:

        void DrawCullingLegend() const;

        void MoveToMainCamera(ViewportDebugCamera& a_debugCamera) const;

        static constexpr std::string_view k_titleLabel                   = "デバッグカメラ";
        static constexpr std::string_view k_enableLabel                  = "デバッグカメラで描画する";
        static constexpr std::string_view k_enableTooltip                = "ビューポートを、メインカメラとは別の自由に動かせるカメラから見た絵にします。\nカリングはメインカメラのまま行うため、メインカメラが何を描いているかを外から確認できます。\n操作 : 右ボタンを押しながらマウスで向き、WASDで移動、E / Qで上下、Shiftで速く移動、ホイールで前後。";
        static constexpr std::string_view k_moveSpeedLabel               = "移動速度(m/秒)";
        static constexpr std::string_view k_rotationSpeedLabel           = "回転速度(度/ピクセル)";
        static constexpr std::string_view k_cullingVisualizeLabel        = "カリング結果を描画";
        static constexpr std::string_view k_cullingVisualizeTooltip      = "メインカメラのカリングで消えるMesh / Meshletも、判定結果ごとの色を付けて描画します。\nオフにすると、メインカメラで実際に描かれる物だけが表示されます。";
        static constexpr std::string_view k_drawFrustumLabel             = "メインカメラの視錐台を描画";
        static constexpr std::string_view k_moveToMainCameraLabel        = "メインカメラの位置へ移動";
        static constexpr std::string_view k_legendTitleLabel             = "色の意味";
        static constexpr std::string_view k_legendMeshFrustumLabel       = "Mesh単位 : 視錐台の外";
        static constexpr std::string_view k_legendMeshOccludedLabel      = "Mesh単位 : 手前の物に隠れている";
        static constexpr std::string_view k_legendMeshletFrustumLabel    = "Meshlet単位 : 視錐台の外";
        static constexpr std::string_view k_legendMeshletBackfaceLabel   = "Meshlet単位 : 全ての三角形が裏向き";
        static constexpr std::string_view k_legendOcclusionNoteLabel     = "※ デバッグカメラで描画している間、オクルージョンカリング(Hi-Z)は行いません。";
        static constexpr std::string_view k_unavailableLabel             = "デバッグカメラはDebugビルドでのみ使えます。";

        static constexpr ImVec4 k_meshFrustumCulledColor     = { 1.00F, 0.15F, 0.15F, 1.00F };
        static constexpr ImVec4 k_meshOccludedColor          = { 1.00F, 0.20F, 1.00F, 1.00F };
        static constexpr ImVec4 k_meshletFrustumCulledColor  = { 1.00F, 0.60F, 0.10F, 1.00F };
        static constexpr ImVec4 k_meshletBackfaceCulledColor = { 0.20F, 0.50F, 1.00F, 1.00F };

        static constexpr float k_moveSpeedDragStep = 0.1F;
        static constexpr float k_minMoveSpeed      = 0.1F;
        static constexpr float k_maxMoveSpeed      = 1000.0F;

        static constexpr float k_rotationSpeedDragStep = 0.005F;
        static constexpr float k_minRotationSpeed      = 0.01F;
        static constexpr float k_maxRotationSpeed      = 2.0F;
    };
}
