#pragma once

namespace FWK::Utility
{
	template <typename Type>
	inline bool StringKeyRegistryRadioButtonSelector(const std::string_view& a_label, Type& a_value)
	{
		bool l_isCreate = false;

		ImGui::BeginGroup();

		const auto& l_stringKeyRegistry = StringKeyRegistry<Type>::GetInstance();

		// 値から名前を取得する
		const std::string_view& l_valueName = l_stringKeyRegistry.FindVALByValue(a_value);
		
		// 文字列が空なら登録されていない値なのでUnknownの文字列を渡す
		if (l_valueName.empty())
		{
			l_valueName = Constant::k_selecteUnknown;
		}

		if (!ImGui::BeginCombo(a_label.data(), l_valueName.data()))
		{
			ImGui::EndGroup();

			return false;
		}

		for (const auto& [l_key, l_value] : l_stringKeyRegistry.GetREFStringToValueMap())
		{
			bool l_isSelected = l_valueName == l_key;
			
			ImGui::PushID(&l_value);

			// ラジオボタンがクリックされなければ処理をスキップ
			if (!ImGui::RadioButton(l_key.c_str() , l_isSelected))
			{
				ImGui::PopID();

				continue;
			}

			// 選択された項目にカーソルを当てる
			if (l_isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}

			a_value    = l_value;
			l_isCreate = true;

			ImGui::PopID();
		}

		ImGui::EndCombo();
		ImGui::EndGroup();

		return l_isCreate;
	}
}