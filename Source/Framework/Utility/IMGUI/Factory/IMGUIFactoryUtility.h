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

		std::string l_createInstanceName = Constant::k_selectUnknownString.data();

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

	template <typename FactoryType, typename Value>
	inline bool FactoryCheckBoxMapSelector(const std::string_view& a_label, std::unordered_map<TypeAlias::StaticTypeID, Value>& a_selectedMap, const float a_visibleItemCount = Constant::k_defaultChildVisibleItemCount)
	{
		// Factoryから生成されるGameObjectは
		// shared_ptr/weak_ptrなどのSmartPointer前提にする
		if constexpr (!Concept::IsSharedPTRConcept<Value> &&
			          !Concept::IsUniquePTRConcept<Value>)
		{
			return false; 
		}

		const auto& l_typeINFORegistry = TypeINFORegistry::GetInstance();
		      auto& l_factory          = FactoryType::GetInstance     ();
		      bool  l_isChanged        = false;

		ImGui::PushID    (std::addressof(a_selectedMap));
		ImGui::BeginGroup();

		// リスト前の区切り線
		if (!a_label.empty())
		{
			ImGui::SeparatorText(a_label.data());
		}
		else
		{
			ImGui::Separator();
		}

		// -1.0Fを使用すると
		// 現在利用可能な横幅いっぱいまでリストを広げる
		if (const float l_listHeight = ImGui::GetTextLineHeightWithSpacing() * a_visibleItemCount;
			!ImGui::BeginListBox(Constant::k_factoryCheckBoxListLabel.data(), ImVec2(Constant::k_childWindowMAXSize, l_listHeight)))
		{
			ImGui::EndGroup();
			ImGui::PopID   ();

			return false;
		}

		for (const auto& [l_key, l_value] : l_factory.GetREFFactoryMap())
		{
			const auto* l_typeINFO = l_typeINFORegistry.FindPTRByName(l_key);

			if (!l_typeINFO)
			{
				FWK_ADD_LOG(Constant::k_debugWarningColor, "Factoryに登録されている型のTypeINFOを取得できませんでした。\nTypeName : {}", l_key);

				continue;
			}

			const auto l_staticTypeID = l_typeINFO->k_staticTypeID;
			      bool l_isSelected   = false;

			// Mapのキーだけを信用するのではなく
			// 実際に保持しているInstanceのRuntimeTypeInfoとFactory型のStaticTypeIDを比較する
			if (const auto& l_itr = a_selectedMap.find(l_staticTypeID);
				l_itr != a_selectedMap.end())
			{
				const auto& l_instance = l_itr->second;

				if (l_instance)
				{
					l_isSelected = l_instance->GetREFRuntimeTypeINFO().k_staticTypeID == l_staticTypeID;
				}
			}

			// CheckBoxのOn/Offが切り替えられていないならcontinue
			if (!ImGui::Checkbox(l_key.c_str(), &l_isSelected)) { continue; }

			// チェックボックスのチェックが外されるかつけられるかで実行する
			if (l_isSelected) 
			{
				auto l_instance = l_value();

				if (!l_instance)
				{
					FWK_ADD_LOG(Constant::k_debugWarningColor, "FactoryからInstanceを生成できませんでした。\nTypeName : {}", l_key);

					continue;
				}

				// Factoryから生成されたInstanceが、本当に選択したFactory型と一致しているか確認する
				const auto l_createdStaticTypeID = l_instance->GetREFRuntimeTypeINFO().k_staticTypeID;

				if (l_createdStaticTypeID != l_staticTypeID)
				{
					FWK_ADD_LOG(Constant::k_debugWarningColor, "Factoryから生成したInstanceのStaticTypeIDが一致しませんでした。\nTypeName : {}", l_key);

					continue;
				}

				a_selectedMap.try_emplace(l_staticTypeID, std::move(l_instance));

				l_isChanged = true;

				continue;
			}
			else
			{
				if (a_selectedMap.erase(l_staticTypeID) != static_cast<std::uint32_t>(NULL))
				{
					l_isChanged = true;
				}
			}
		}

		ImGui::EndListBox();
		ImGui::EndGroup  ();
		ImGui::PopID     ();

		return l_isChanged;
	}
}