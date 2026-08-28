#pragma once

namespace FWK::Utility
{
	template <typename Type>
	inline bool StringValueBidirectionalRegistryRadioButtonSelector(const std::string_view& a_label, Type& a_wantChange)
	{
		const auto& l_stringValueBidirectionalRegistry = StringValueBidirectionalRegistry<Type>::GetInstance      ();
		const auto& l_stringToValueMap                 = l_stringValueBidirectionalRegistry.GetREFStringToValueMap();

		std::string_view l_preview = Constant::k_selectUnknownString;

		// 現在選択されているEnumに対応する文字列を取得する
		for (const auto& [l_key, l_value] : l_stringToValueMap)
		{
			if (l_value != a_wantChange) { continue; }

			l_preview = l_key;

			break;
		}

		// 同じLabelを持つSelectorが複数存在しても
		// ImGui内部IDが衝突しないように
		// 選択対象の変数のアドレスをIDとして使用する
		ImGui::PushID    (std::addressof(a_wantChange));
		ImGui::BeginGroup();

		if (!ImGui::BeginCombo(a_label.data(), l_preview.data()))
		{
			ImGui::EndGroup();
			ImGui::PopID   ();

			return false;
		}

		bool l_isChanged = false;

		for (const auto& [l_key, l_value] : l_stringToValueMap)
		{
			const bool l_isSelected = a_wantChange == l_value;

			if (ImGui::RadioButton(l_key.c_str(), l_isSelected))
			{
				a_wantChange = l_value;
				l_isChanged  = true;
			}

			// 現在選択されている項目へ
			// Comboを開いた時の先頭Focusを設定する
			if (l_isSelected) 
			{
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndCombo();
		ImGui::EndGroup();
		ImGui::PopID   ();

		return l_isChanged;
	}
}