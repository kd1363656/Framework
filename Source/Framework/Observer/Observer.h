#pragma once

namespace FWK
{
	template <typename Type>
		requires std::is_enum_v<Type>
	class Observer final
	{
	public:

		 Observer() = default;
		~Observer() = default;

		void INIT()
		{
			m_eventMap.clear();
		}
		void Deserialize(const nlohmann::json& a_rootJson)
		{
			if (a_rootJson.is_null()) { return; }

			m_jsonConverter.Deserialize(a_rootJson, *this);
		}

		void BeginFrame()
		{
			for (auto& [l_key, l_value] : m_eventMap)
			{
				// 瞬間的に記録するイベントは毎フレーム"false"にする
				// そうすることでそのフレームしか通知しないイベントを実現できる
				l_value.m_isNotificationActiveThisFrame = false;
			}
		}

		void EditInspector()
		{
			ImGui::PushID(&m_eventMap);

			// StringKeyRegistryから追加したいEventを選択する
			Utility::StringValueBidirectionalRegistryRadioButtonSelector<Type>(k_imguiLabel, m_imguiSelectingEvent);

			ImGui::SameLine();

			if (ImGui::Button("AddEvent"))
			{
				AddEvent(m_imguiSelectingEvent);
			}

			ImGui::Separator();

			const auto& l_stringValueBidirectionalRegistry = Utility::StringValueBidirectionalRegistry<Type>::GetInstance();
			      auto  l_itr                              = m_eventMap.begin                                            ();

			while (l_itr != m_eventMap.end())
			{
				// イテレータごとにIDを発行
				ImGui::PushID(&l_itr->second);

				std::string_view l_keyName = l_stringValueBidirectionalRegistry.FindVALKeyByValue(l_itr->first);

				if (l_keyName.empty())
				{
					l_keyName = Constant::k_selecteUnknownString;
				}

				if (!ImGui::TreeNodeEx(l_keyName.data(), ImGuiTreeNodeFlags_Framed))
				{
					++l_itr;
					ImGui::PopID();
					
					continue;
				}

				// 削除されたらイテレーターを更新して"continue"
				ImGui::SameLine();

				if (ImGui::Button("Delete"))
				{
					l_itr = m_eventMap.erase(l_itr);
					ImGui::TreePop          ();
					ImGui::PopID            ();

					continue;
				}

				ImGui::Text("HasPendingNotification         : %s" , Utility::BoolToString(l_itr->second.m_hasPendingNotification).data());
				ImGui::Text("IsNotificastionActiveThisFrame : %s" , Utility::BoolToString(l_itr->second.m_isNotificationActiveThisFrame).data());
				ImGui::Separator();


				l_itr++;

				ImGui::TreePop();
				ImGui::PopID  ();
			}

			ImGui::PopID();
		}

		nlohmann::json Serialize() const
		{
			return m_jsonConverter.Serialize(*this);
		}

		void AddEvent(Type a_event)
		{
			if (a_event == Type::Invalid) { return; }

			// イベントに適したキーを格納
			Struct::EventData l_eventData = {};

			m_eventMap.try_emplace(a_event, l_eventData);
		}

		const auto& GetREFEventMap() const { return m_eventMap; }

	private:

		static constexpr std::string_view k_imguiLabel = "EventSelector";

		std::unordered_map<Type, Struct::EventData> m_eventMap = {};

		Converter::ObserverJsonConverter<Type> m_jsonConverter = {};

		// 絶対にEnum側で無効値としてInvlaidを用意しておく
		Type m_imguiSelectingEvent = Type::Invalid;
	};
}