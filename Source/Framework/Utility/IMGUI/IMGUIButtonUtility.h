#pragma once

namespace FWK::Utility
{
    inline float IMGUIFetchButtonWidth(const std::string_view& a_label)
    {
        // Buttonの横幅は テキスト幅 + 左側FramePadding.x + 右側FramePadding.x になる
        // 例 : テキスト幅が60、FramePadding.xが4なら60 + 4 * 2 = 68
        const float l_labelWidth = ImGui::CalcTextSize(a_label.data(), a_label.data() + a_label.size()).x;

        return l_labelWidth + ImGui::GetStyle().FramePadding.x * Constant::k_imguiFramePaddingBothSidesNUM;
    }

    inline float IMGUIFetchFitButtonWidth(const std::span<const std::string_view>& a_labelList)
    {
        if (a_labelList.empty()) { return Constant::k_imguiEmptyButtonListWidth; }

        // ボタンとボタンの間はItemSpacing.xだけ空くので、
        // N個並べると間はN - 1か所になる
        const float l_buttonCount  = static_cast<float>(a_labelList.size());
        const float l_spacingWidth = ImGui::GetStyle().ItemSpacing.x * (l_buttonCount - Constant::k_imguiButtonSpacingCountOffset);

        // 描画可能領域の横幅から間の分を引いて、残りをボタンの数で等分する
        // 例 : 描画可能領域が400、ItemSpacing.xが8、ボタンが3個なら(400 - 8 * 2) / 3 = 128
        const float l_fitButtonWidth = (ImGui::GetContentRegionAvail().x - l_spacingWidth) / l_buttonCount;

        // AlwaysAutoResizeのウィンドウは開いた最初のフレームだと描画可能領域がまだ決まっておらず、
        // 等分した幅が0やマイナスになることがあるため、一番長いラベルが切れない幅は必ず確保する
        float l_longestButtonWidth = Constant::k_imguiInitialLongestButtonWidth;

        for (const auto& l_label : a_labelList)
        {
            l_longestButtonWidth = std::max(l_longestButtonWidth, IMGUIFetchButtonWidth(l_label));
        }

        return std::max(l_fitButtonWidth, l_longestButtonWidth);
    }
}