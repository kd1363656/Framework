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
				l_value = Utility::DisableFlag(Enum::EventLane::TriggeredThisFrame, l_value);
			}
		}

		void NotifyEvent(const Type a_type, const Enum::EventLane a_enableFlagEventLane)
		{
			auto l_itr = m_eventMap.find(a_type);

			if (l_itr == m_eventMap.end()) { return; }

			l_itr->second = Utility::EnableFlag(a_enableFlagEventLane, l_itr->second);
		}

		bool IsEventMatching(const Type a_type, const Enum::EventLane a_isMatchEventLane)
		{
			auto l_itr = m_eventMap.find(a_type);

			if (l_itr == m_eventMap.end()) { return false; }

			return Utility::IsFlagEnabled(a_isMatchEventLane, l_itr->second);
		}

		void EditInspector(const std::string_view& a_label)
		{
			m_inspector.EditorInspector(a_label, m_eventMap);
		}

		nlohmann::json Serialize() const
		{
			return m_jsonConverter.Serialize(*this);
		}

		void AddEvent(Type a_event)
		{
			if (a_event == Type::Invalid) { return; }

			// フラグはすべて無効状態で初期化される
			m_eventMap.try_emplace(a_event, Constant::k_noFlagValue);
		}

		const auto& GetREFEventMap() const { return m_eventMap; }

	private:

		std::unordered_map<Type, std::uint32_t> m_eventMap = {};

		ObserverInspector<Type> m_inspector = {};

		Converter::ObserverJsonConverter<Type> m_jsonConverter = {};

		// 絶対にEnum側で無効値としてInvalidを用意しておく
		Type m_imguiSelectingEvent = Type::Invalid;
	};
}