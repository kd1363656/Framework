#pragma once

namespace FWK
{
	class MoveComponent;
}

namespace FWK::Converter
{
	class MoveComponentJsonConverter final
	{
	public:

		 MoveComponentJsonConverter() = default;
		~MoveComponentJsonConverter() = default;

		void DeserializePrefab(const nlohmann::json& a_rootJson, MoveComponent& a_moveComponent) const;
		
		nlohmann::json SerializePrefab(const MoveComponent& a_moveComponent) const;
	};
}