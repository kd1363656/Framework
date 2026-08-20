#pragma once

namespace FWK::Utility
{
	// ファクトリーから生成するクラスを選べるセレクター
	template <typename FactoryType, typename Type>
	inline bool FactoryRadioButtonSelector(const std::string_view& a_label, Type& a_wantChange)
	{
		bool l_isCreate = false;

		if constexpr (!Concept::IsSmartPTRConcept<Type>) { return false; }
		
		ImGui::PushID    (std::to_address(a_wantChange));
		ImGui::BeginGroup();

		std::string l_createInstanceName = Constant::k_selecteUnknownString.data();

		// もしストラテジーが既にインスタンス化されているなら文字列を取得
		if (a_wantChange)
		{
			l_createInstanceName = a_wantChange->GetREFRuntimeTypeINFO().k_name;
		}

		if (!ImGui::BeginCombo(a_label.data(), l_createInstanceName.c_str()))
		{
			ImGui::PopID   ();
			ImGui::EndGroup();

			return false;
		}

		auto& l_factory = FactoryType::GetInstance();

		for (const auto& [l_key, l_value] : l_factory.GetREFFactoryMap())
		{
			bool l_isSelected = l_createInstanceName == l_key;
			
			// ラジオボタンがクリックされなければ処理をスキップ
			if (!ImGui::RadioButton(l_key.c_str() , l_isSelected))
			{
				continue;
			}

			// 選択された項目にカーソルを当てる
			if (l_isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}

			a_wantChange = l_value();
			l_isCreate   = true;
		}

		ImGui::EndCombo();
		ImGui::EndGroup();
		ImGui::PopID   ();

		return l_isCreate;
	}

	template <typename FactoryType, typename Key, typename Value>
	inline bool FactoryCheckBoxMapSelector(std::unordered_map<Key, Value>& a_selectedMap)
	{
		// Factoryから生成されるGameObjectは
		// shared_ptr/weak_ptrなどのSmartPointer前提にする
		if constexpr (!Concept::IsSmartPTRConcept<Type>) { return false; }

		bool  l_isChanged = false;
		auto& l_factory   = FactoryType::GetInstance();

		ImGui::PushID    (std::to_address(a_selectedMap));
		ImGui::BeginGroup();

		// Factoryへ登録されている全Typeを
		// Checkboxとして一覧表示する
		for (const auto& [l_key, l_value] : l_factory.GetREFFactoryMap())
		{
			// 現在a_selectedMapに存在しているなら
		}
	}
}