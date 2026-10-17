#pragma once

namespace FWK::Utility
{
    inline void IMGUIPushItemHighlightColors(const bool a_isSelected,
                                             const bool a_isActiveTarget,
                                             const bool a_isCutTarget      = false,
                                             const bool a_isSelectedParent = false)
    {
        // デフォルトは未選択アイテムのニュートラルなグレー系
        auto l_headerColor  = Constant::k_imguiItemColor;
        auto l_hoveredColor = Constant::k_imguiItemHoveredColor;
        auto l_activeColor  = Constant::k_imguiItemActiveColor;

        if (a_isCutTarget)
        {
            // Cut対象は選択状態より優先してCut色を使う
            // Cut色は選択色を半分の明るさにしたもので、アセットブラウザーのカードと同じ定数を使う
            l_headerColor  = Constant::k_imguiItemCutColor;
            l_hoveredColor = Constant::k_imguiItemCutColor;
            l_activeColor  = Constant::k_imguiItemCutColor;
        }
        else if (a_isSelected)
        {
            if (a_isActiveTarget)
            {
                // アクティブなウィンドウ・ペイン内の選択色
                // ホバー・押下で色が消えないよう選択色系を維持する
                l_headerColor  = Constant::k_imguiItemSelectedColor;
                l_hoveredColor = Constant::k_imguiItemSelectedHoveredColor;
                l_activeColor  = Constant::k_imguiItemSelectedActiveColor;
            }
            else
            {
                // 非アクティブなウィンドウ・ペイン内ではUEと同じくくすんだ青にして
                // フォーカスが外れていることが見た目で分かるようにする
                // 半透明の青だと下の縞模様で色が変わるため不透明の色にしている
                l_headerColor  = Constant::k_imguiItemSelectedInactiveColor;
                l_hoveredColor = Constant::k_imguiItemSelectedInactiveColor;
                l_activeColor  = Constant::k_imguiItemSelectedInactiveColor;
            }
        }
        else if (a_isSelectedParent)
        {
            // 選択中のアイテムの親(フォルダの親・GameObjectの親)は
            // UEと同じく暗い青灰色で塗り、選択がどこの下にあるかを畳んだままでも分かるようにする
            // ホバー・押下は通常のアイテムと同じ色にする
            l_headerColor = Constant::k_imguiItemSelectedParentColor;
        }

        ImGui::PushStyleColor(ImGuiCol_Header,        l_headerColor);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, l_hoveredColor);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,  l_activeColor);
    }

    // IMGUIPushItemHighlightColorsでPushした色をまとめて戻す
    inline void IMGUIPopItemHighlightColors()
    {
        ImGui::PopStyleColor(Constant::k_imguiItemHighlightColorPushCount);
    }
}