#pragma once

namespace FWK::Utility
{
    inline void IMGUIPushItemHighlightColors(const bool a_isSelected,
                                             const bool a_isActiveTarget,
                                             const bool a_isCutTarget    = false,
                                             const bool a_isRenameTarget = false)
    {
        // デフォルトは未選択アイテムのニュートラルなグレー系
        ImVec4 l_headerColor  = Constant::k_imguiItemColor;
        ImVec4 l_hoveredColor = Constant::k_imguiItemHoveredColor;
        ImVec4 l_activeColor  = Constant::k_imguiItemActiveColor;

        if (a_isRenameTarget)
        {
            // リネーム中のアイテムは、他の状態より優先して、未選択のアイテムにマウスを乗せたときと同じ色にする
            // 選択色(青)のままだと、InputTextの選択範囲の青と重なって見分けにくいため
            // 3色とも同じ色にして、マウスの位置で色が変わらないようにする
            l_headerColor  = Constant::k_imguiItemHoveredColor;
            l_hoveredColor = Constant::k_imguiItemHoveredColor;
            l_activeColor  = Constant::k_imguiItemHoveredColor;
        }
        else if (a_isCutTarget)
        {
            // Cut対象は選択状態より優先してCut色を使う
            // Cut色は選択色へ半分倍率を掛けて暗くしたもの
            const auto& l_cutColor = Constant::k_imguiItemSelectedColor * Constant::k_halfMagnification;

            l_headerColor  = l_cutColor;
            l_hoveredColor = l_cutColor;
            l_activeColor  = l_cutColor;
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
                // 非アクティブなウィンドウ・ペイン内では半透明にして
                // フォーカスが外れていることが見た目で分かるようにする
                l_headerColor  = Constant::k_imguiItemSelectedInactiveColor;
                l_hoveredColor = Constant::k_imguiItemSelectedInactiveColor;
                l_activeColor  = Constant::k_imguiItemSelectedInactiveColor;
            }
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