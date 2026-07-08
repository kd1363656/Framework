#pragma once

namespace FWK::Utility
{
	// ファクトリーから生成するクラスを選べるセレクター
	template <typename FactoryType , typename Type>
	inline bool FactoryRadioButtonSelector(const std::string_view& a_label, Type& a_wantChange)
	{
		bool l_isCreate = false;

		if constexpr (!Concept::IsSmartPTRConcept<Type>) { return false; }
		
		ImGui::BeginGroup();

		std::string l_createInstanceName = {};

		// もしストラテジーが既にインスタンス化されているなら文字列を取得
		if (a_wantChange)
		{
			l_createInstanceName = a_wantChange->GetREFRuntimeTypeINFO().k_name;
		}

		if (!ImGui::BeginCombo(a_label.data(), l_createInstanceName.c_str()))
		{
			ImGui::EndGroup();

			return false;
		}

		auto& l_factory = FactoryType::GetInstance();

		for (const auto& [l_key, l_value] : l_factory.GetREFFactoryMap())
		{
			bool l_isSelected = l_createInstanceName == l_key;
			
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

			a_wantChange = l_value();

			l_isCreate   = true;

			ImGui::PopID();
		}

		ImGui::EndCombo();
		ImGui::EndGroup();

		return l_isCreate;
	}
}