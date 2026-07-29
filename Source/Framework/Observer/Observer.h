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

		void BeginFrame()
		{
			for (auto& [l_key, l_value] : m_eventMap)
			{
				// 瞬間的に記録するイベントは毎フレーム"false"にする
				// そうすることでそのフレームしか通知しないイベントを実現できる
				l_value.m_isNotificastionActiveThisFrame = false;
			}
		}

		void EditInspector()
		{
			ImGui::PushID(&m_eventMap);

			
		}

	private:

		std::unordered_map<Type, Struct::EventData> m_eventMap = {};

		// 絶対にEnum側で無効値としてInvlaidを用意しておく
		Type m_imguiSelectingTag = Type::Invalid;
	};
}