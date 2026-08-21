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

		bool EditorInspector(std::unordered_map<Type, std::uint32_t>& a_eventMap)
		{
			      bool  l_isChanged                        = false;
			const auto& l_stringValueBidirectionalRegistry = Utility::StringValueBidirectionalRegistry<Type>::GetInstance();

			ImGui::PushID    (std::addressof(a_eventMap));
			ImGui::BeginGroup();

			ImGui::EndGroup();
			ImGui::PopID   ();

			return l_isChanged;
		}
	};
}