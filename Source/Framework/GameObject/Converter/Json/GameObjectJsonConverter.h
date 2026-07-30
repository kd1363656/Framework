#pragma once

namespace FWK
{
	class GameObject;
}

namespace FWK
{
	class GameObjectJsonConverter final
	{
	public:

		 GameObjectJsonConverter() = default;
		~GameObjectJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, GameObject& a_gameObject) const;
		
		nlohmann::json Serialize      (const GameObject& a_gameObject) const;
		nlohmann::json SerializePrefab(const GameObject& a_gameObject) const;
	};
}