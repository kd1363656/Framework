#pragma once

namespace FWK
{
	template <typename Type>
		requires std::is_enum_v<Type>
	class ObserverInspector final
	{
	public:

		 ObserverInspector() = default;
		~ObserverInspector() = default;

		bool EditorInspector(const std::string_view& a_label, std::unordered_map<Type, std::uint32_t>& a_eventMap)
		{
			      bool  l_isChanged                        = false;
			const auto& l_stringValueBidirectionalRegistry = Utility::StringValueBidirectionalRegistry<Type>::GetInstance();

			ImGui::PushID    (std::addressof(a_eventMap));
			ImGui::BeginGroup();

			// リスト前の区切り
			if (!a_label.empty())
			{
				ImGui::SeparatorText(a_label.data());
			}
			else
			{
				ImGui::Separator();
			}

			const float l_listHeight = ImGui::GetTextLineHeightWithSpacing() * Constant::k_defaultChildVisibleItemCount;

			// -1.0Fを使用すると
			// 現在利用可能な横幅いっぱいまでリストを広げる
			if (!ImGui::BeginListBox(k_observerSelectorCheckBoxListLabel.data(), ImVec2(Constant::k_childWindowMAXSize, l_listHeight)))
			{
				ImGui::EndGroup();
				ImGui::PopID   ();

				return false;
			}

			for (const auto& [l_label, l_enumType] : l_stringValueBidirectionalRegistry.GetREFStringToValueMap())
			{
				// 無効な値は候補として出さない
				if (l_enumType == Type::Invalid) { continue; }

				// EventMapに存在していれば、
				// 現在そのEventを監視しているためCheckboxON
				bool l_isSelected = a_eventMap.contains(l_enumType);

				// CheckBoxに変更がなければ
				// EventMapを変更する必要はない
				if (!ImGui::Checkbox(l_label.c_str(), &l_isSelected)) { continue; }

				// 無効から有効化するならEventを監視対象へ追加
				if (l_isSelected)
				{
					const bool l_isAdded = a_eventMap.try_emplace(l_enumType, Constant::k_noFlagValue).second;

					if (l_isAdded) 
					{
						l_isChanged = true;
					}

					continue;
				}
				else
				{
					// 有効から無効ならEventを監視対象から外す
					if (a_eventMap.erase(l_enumType) != static_cast<std::uint32_t>(NULL))
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

	private:

		static constexpr std::string_view k_observerSelectorCheckBoxListLabel = "##ObserverSelectorCheckBoxList";
	};
}