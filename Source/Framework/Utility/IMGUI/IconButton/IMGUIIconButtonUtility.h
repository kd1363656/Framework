#pragma once

namespace FWK::Utility
{
    // アイコン(記号)を書いたボタンを描き、押されたフレームだけtrueを返す
    // a_isHighlighted : trueの時はエディターの強調色の青で描く(選んでいる物、動いている物の目印)
    // a_isEnabled     : falseの時は灰色にして押せないようにする(説明の表示は残す)
    inline bool IMGUIIconButton(const std::string_view& a_label,
                                const std::string_view& a_tooltip,
                                const ImVec2&           a_size,
                                const bool              a_isHighlighted,
                                const bool              a_isEnabled)
    {
        // ビューポートの絵の上に重ねても下が見えるよう、背景を少し透けさせる
        auto l_buttonColor = a_isHighlighted ? Constant::k_imguiAccentColor : Constant::k_imguiHeaderColor;

        l_buttonColor.w = Constant::k_imguiIconButtonBackgroundAlpha;

        ImGui::PushStyleColor(ImGuiCol_Button,        l_buttonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, a_isHighlighted ? Constant::k_imguiAccentHoverColor : Constant::k_imguiHoverColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  a_isHighlighted ? Constant::k_imguiAccentPressColor : Constant::k_imguiPressColor);

        // BeginDisabled(true)からEndDisabledまでの間に描いた物は、灰色になり押せなくなる
        ImGui::BeginDisabled(!a_isEnabled);

        const bool l_isClicked = ImGui::Button(a_label.data(), a_size);

        ImGui::EndDisabled();

        ImGui::PopStyleColor(Constant::k_imguiIconButtonStyleColorCount);

        // 押せないボタンにも説明を出したいため、Disabledの範囲の外で判定する
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip("%s", a_tooltip.data());
        }

        return l_isClicked;
    }
}
