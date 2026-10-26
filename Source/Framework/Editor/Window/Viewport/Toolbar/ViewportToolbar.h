#pragma once

namespace FWK::Editor
{
    class EditorManager;
}

namespace FWK::Editor
{
    // ビューポートの絵の上に並ぶツールバー(絵には重ねず、絵より先に1行分の場所を取る)
    //
    // 左端                   : 表示(当たり判定の可視化、描画の設定を開く)
    // ウィンドウの左右の真ん中 : ▶(再生)と■(停止)
    //                          ▶でシーンのGameObjectが動き出し、■で▶を押す前の状態へ戻る(■は再生中だけ押せる)
    class ViewportToolbar final
    {
    public:

         ViewportToolbar() = default;
        ~ViewportToolbar() = default;

        void Draw(EditorManager& a_editorManager);

    private:

        void DrawDisplayOptionsButton() const;
        void DrawDisplayOptionsPopup () const;
        void DrawCollisionCheckbox   () const;
        void DrawRenderPassSettings  () const;

        void DrawPlayButtonGroup(EditorManager& a_editorManager, const float a_windowCenterX) const;

        void BeginPlay()                               const;
        void EndPlay  (EditorManager& a_editorManager) const;

        static constexpr std::string_view k_toolbarChildID            = "##ViewportToolbar";
        static constexpr std::string_view k_displayOptionsButtonLabel = "表示";
        static constexpr std::string_view k_displayOptionsPopupID     = "##SceneViewDisplayOptionsPopup";
        static constexpr std::string_view k_collisionCheckboxLabel    = "当たり判定の可視化";

        static constexpr std::string_view k_playIconLabel = "\xEF\x81\x8B##ViewportPlay";
        static constexpr std::string_view k_stopIconLabel = "\xEF\x81\x8D##ViewportStop";

        static constexpr std::string_view k_playTooltip = "再生 : シーンのゲームオブジェクトを動かし始めます。";
        static constexpr std::string_view k_stopTooltip = "停止 : 再生する前の状態へ戻します。";

        static constexpr float k_toolbarHeight     = 36.0F;
        static constexpr float k_toolbarButtonSize = 28.0F;

        static constexpr float k_fitToLabelWidth = 0.0F;

        static constexpr ImVec2 k_iconButtonSize = { k_toolbarButtonSize, k_toolbarButtonSize };

        static constexpr float k_buttonSpacing            = 2.0F;
        static constexpr float k_playButtonCount          = 2.0F;
        static constexpr float k_playButtonGapCount       = 1.0F;
        static constexpr float k_sameLineOffsetFromStartX = 0.0F;
        static constexpr float k_half                     = 0.5F;

        static constexpr bool k_isNotHighlighted = false;
    };
}
