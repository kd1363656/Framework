#pragma once

namespace FWK::Utility
{
	inline void DelayedTooltip(const std::string_view& a_tooltipText, const float a_delaySeconds = Constant::k_imguiDefaultDelayDrawSecond)
	{
		if (a_tooltipText.empty()) { return; }

		auto& l_imGuiStyle = ImGui::GetStyle();

		// このTooltip判定にだけ指定されたHover時間を使用する
		const float l_originalHoverDelayNormal = l_imGuiStyle.HoverDelayNormal;

		l_imGuiStyle.HoverDelayNormal = a_delaySeconds;

		const bool l_isTooltipDisplayable = ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal);

		// 他のImGuiItemへ影響させないため
		// 元の値へ必ず戻す
		l_imGuiStyle.HoverDelayNormal = l_originalHoverDelayNormal;

		if (!l_isTooltipDisplayable) { return; }

		ImGui::BeginTooltip();
		ImGui::Text        (a_tooltipText.data());
		ImGui::EndTooltip  ();
	}
}