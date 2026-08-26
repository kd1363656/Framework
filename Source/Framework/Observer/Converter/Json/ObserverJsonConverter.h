#pragma once

namespace FWK
{
	template <typename Type>
		requires std::is_enum_v<Type>
	class Observer;
}

namespace FWK::Converter
{
	template <typename Type>
		requires std::is_enum_v<Type>
	class ObserverJsonConverter final
	{
	public:

		 ObserverJsonConverter() = default;
		~ObserverJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Observer<Type>& a_observer)
		{
			if (a_rootJson.is_null() ||
				!Utility::IsJsonArray(a_rootJson, k_eventMapJsonKey))
			{
				return; 
			}
			
			for (const auto& l_json : a_rootJson[k_eventMapJsonKey])
			{
				if (l_json.is_null()) { continue; }

				const auto l_eventEnum = l_json.value(k_eventJsonKey, Type::Invalid);

				a_observer.AddEvent(l_eventEnum);
			}
		}

		nlohmann::json Serialize(const Observer<Type>& a_observer) const
		{
			nlohmann::json l_rootJson  = {};
			auto           l_jsonArray = nlohmann::json::array();

			for (const auto& [l_key, l_value] : a_observer.GetREFEventMap())
			{
				nlohmann::json l_json = {};

				l_json[k_eventJsonKey] = l_key;

				l_jsonArray.emplace_back(l_json);
			}

			l_rootJson[k_eventMapJsonKey] = l_jsonArray;

			return l_rootJson;
		}

	private:

		static constexpr std::string_view k_eventMapJsonKey = "EventMap";
		static constexpr std::string_view k_eventJsonKey    = "Event";
	};
}