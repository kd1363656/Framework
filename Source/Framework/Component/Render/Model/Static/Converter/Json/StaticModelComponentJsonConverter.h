#pragma once

namespace FWK
{
	class StaticModelComponent;
}

namespace FWK::Converter
{
	class StaticModelComponentJsonConverter final
	{
	public:

		 StaticModelComponentJsonConverter() = default;
		~StaticModelComponentJsonConverter() = default;

		void DeserializePrefab(const nlohmann::json& a_rootJson, StaticModelComponent& a_staticModelComponent) const;
		
		nlohmann::json SerializePrefab(StaticModelComponent& a_staticModelComponent) const;
		
	private:

		void DeserializeRegisterDrawRequestStrategyMap(const nlohmann::json& a_rootJson, StaticModelComponent& a_staticModelComponent) const;

		nlohmann::json SerializeRegisterDrawRequestStrategyMap(const StaticModelComponent& a_staticModelComponent) const;

		static constexpr std::string_view k_registerDrawRequestStrategyMapJsonKey = "RegisterDrawRequestStrategyMapJsonKey";
		static constexpr std::string_view k_drawRequestStrategyJsonKey            = "DrawRequestStrategyJsonKey";
	};
}